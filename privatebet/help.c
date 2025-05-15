#include <stdlib.h>
#include <string.h>

#include "bet.h"
#include "help.h"
#include "switchs.h"

// Implementation of strdup if not available
#ifndef strdup
char *strdup(const char *s)
{
	if (!s)
		return NULL;
	size_t len = strlen(s) + 1;
	char *new = malloc(len);
	if (!new)
		return NULL;
	return memcpy(new, s, len);
}
#endif

void bet_command_info()
{
	// Define command sections with their descriptions
	typedef struct {
		const char *section;
		const char *commands[10]; // Array of command strings
		int cmd_count; // Number of commands in this section
	} CommandSection;

	// Define all command sections
	CommandSection sections[] = {
		{ "==Dealer==",
		  {
			  "dcv <ipv4_address>     Start the dealer node",
		  },
		  1 },
		{ "==Player==",
		  {
			  "player                 Start the player node",
		  },
		  1 },
		{ "==Cashier==",
		  {
			  "cashierd cashier <ipv4_address>  Start the cashier node",
		  },
		  1 },
		{ "==Dealer Registration==",
		  {
			  "register_dealer <dealer_id>      Register a new dealer",
			  "deregister_dealer <dealer_id>    Deregister an existing dealer",
			  "raise_registration_dispute <dealer_id> <action>  Raise a dealer registration dispute",
		  },
		  3 },
		{ "==DRP (Dispute Resolution Protocol)==",
		  {
			  "game info <fail|success>         Display game information",
			  "game solve                       Resolve all disputes",
			  "game dispute <tx_id>             Resolve specific dispute",
		  },
		  3 },
		{ "==Wallet==",
		  {
			  "withdraw <amount> <chips_address>  Withdraw specific amount",
			  "withdraw all <chips_address>       Withdraw all funds",
			  "spendable                          List spendable transactions",
			  "consolidate                        Consolidate funds",
			  "tx_split <m> <n>                   Split transaction (m into n parts)",
			  "extract_tx_data <tx_id>            Extract transaction data",
		  },
		  6 },
		{ "==Blockchain Scanner==",
		  {
			  "scan                               Scan blockchain for game info",
		  },
		  1 },
		{ "==VDXF ID Commands==",
		  {
			  "print_id <id_name> <type>          Print ID information",
			  "print <id_name> <key_name>         Print specific key information",
			  "add_dealer <dealer_id>             Add a dealer",
			  "list_dealers                       List all dealers",
			  "list_tables                        List all tables",
			  "reset_id                           Reset ID information",
		  },
		  6 },
		{ "==Help==",
		  {
			  "help <command>         Get detailed help for a command",
			  "                       Supported commands: cashier, dealer, player, game,",
			  "                       spendable, scan, withdraw, verus, vdxf",
		  },
		  3 }
	};

	// Build the help text
	char *help_text = strdup("\nAvailable Commands:\n");
	if (!help_text) {
		dlg_error("Memory allocation failed");
		return;
	}

	// Add each section and its commands
	for (size_t i = 0; i < sizeof(sections) / sizeof(sections[0]); i++) {
		// Add section header
		char *temp = malloc(strlen(help_text) + strlen(sections[i].section) + 3);
		if (!temp) {
			free(help_text);
			dlg_error("Memory allocation failed");
			return;
		}
		sprintf(temp, "%s\n%s\n", help_text, sections[i].section);
		free(help_text);
		help_text = temp;

		// Add commands
		for (int j = 0; j < sections[i].cmd_count; j++) {
			temp = malloc(strlen(help_text) + strlen(sections[i].commands[j]) + 3);
			if (!temp) {
				free(help_text);
				dlg_error("Memory allocation failed");
				return;
			}
			sprintf(temp, "%s%s\n", help_text, sections[i].commands[j]);
			free(help_text);
			help_text = temp;
		}
	}

	// Add final help message
	char *temp = malloc(strlen(help_text) + 100);
	if (!temp) {
		free(help_text);
		dlg_error("Memory allocation failed");
		return;
	}
	sprintf(temp, "%s\nFor more information about a specific command, use: ./bet help <command>\n", help_text);
	free(help_text);
	help_text = temp;

	// Display the help text
	dlg_info("%s", help_text);
	free(help_text);
}

void bet_help_dcv_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "ip address", "ip address of the dealer");

	dlg_info("\nCommand: \n"
		 "dcv\n"
		 "\nDescription: \n"
		 "Starts the backend dealer node \n"
		 "\nArguments: \n"
		 "Inputs: \n"
		 "%s"
		 "\nResult: \n"
		 "A dealer node get started and informed about its availability to the cashier nodes \n"
		 "\nExample: \n"
		 "./bet dcv \"ip address of the dealer\" ",
		 cJSON_Print(command_info));
}

void bet_help_player_command_usage()
{
	dlg_info(
		"\nCommand: \n"
		"player \n"
		"\nDescription: \n"
		"Starts the backend player node \n"
		"\nResult: \n"
		"A player node get started and look for the available dealers and joins the table if there are sufficient funds \n"
		"\nExample: \n"
		"./bet player");
}

void bet_help_cashier_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "ip address", "ip address of the cashier");

	dlg_info(
		"\nCommand: \n"
		"cashier \n"
		"\nDescription: \n"
		"Starts the cashier node \n"
		"\nArguments: \n"
		"Inputs: \n"
		"%s"
		"\nResult: \n"
		"A cashier node get started, like this a group of cashier nodes are required to hold and release the funds during the game. Since the BVV functionalties are integrated into the cashier node, so while deck shuflling the cashier node can also acts like a blinder\n"
		"\nExample: \n"
		"./cashierd cashier \"ip address of the cashier\" \n"
		"or (at the moment bet and cashier daemons contains all the functionalities while building) \n"
		"./bet cashier \"ip address of the cashier\" \n",
		cJSON_Print(command_info));
}

void bet_help_game_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "info", "This basically retrieves the info about the games played");
	cJSON_AddStringToObject(
		command_info, "game_status",
		"[fail/success] or [0/1] Retrives the games which are fully played on passing [success/1] and disconnected games on passing [fail/0] ");

	dlg_info(
		"\nCommand: \n"
		"game \n"
		"\nDescription: \n"
		"This provides the statistics about the games played and to resolve any disputes. The dispute resolution protocol(DRP) which is implemented beneath this command is used to resolve the disputes and players can use this command in order to reverse the funding tx of the games which are not fully played(due to network disruptions or by any other reason) \n"
		"\nArguments: \n"
		"Inputs: \n"
		"%s"
		"\nExample: \n"
		"./bet game info fail\" \n"
		"Result: \n"
		"Displays list of games which are not successfully played.\n"
		"\nExample: \n"
		"./bet game info success\" \n"
		"Result: \n"
		"Displays list of games which are played successfully.\n"
		"\nExample: \n"
		"./bet game solve\" \n"
		"Result: \n"
		"It parses through all unsuccessful games and resolve them using DRP, provided if the game is not played and payout_tx is not happened and the notaries involved(atleast 2) are active then the payin_tx will be reversed and the CHIPS amount will be credited back to the address from which the CHIPS are spent.\n"
		"\nExample: \n"
		"./bet game dispute \"disputed tx id \"  \n"
		"Result: \n"
		"Only the game with the disputed tx id will be resolved using DRP, provided if the game is not played and payout_tx is not happened and the notaries involved(atleast 2) are active then the payin_tx will be reversed and the CHIPS amount will be credited back to the address from which the CHIPS are spent.\n",
		cJSON_Print(command_info));
}

void bet_help_withdraw_command_usage()
{
	dlg_info("\nCommand: \n"
		 "withdraw\n"
		 "\nDescription: \n"
		 "Withdraws CHIPS to the address specified \n"
		 "\nExample: \n"
		 "./bet withdraw amount \"chips address\" \n"
		 "./bet withdraw all \"chips address\" \n");
}

void bet_help_spendable_command_usage()
{
	dlg_info("\nCommand: \n"
		 "spendable \n"
		 "\nDescription: \n"
		 "List the unspent transactions that are spendable by this node \n"
		 "\nResult: \n"
		 "Returns the utxo of which spendable is set to true\n"
		 "\nExample: \n"
		 "./bet spendable \n");
}

void bet_help_extract_tx_data_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "tx_id",
				"It extracts the data part of the payin_tx and display the JSON data of it");

	dlg_info(
		"\nCommand: \n"
		"extract_tx_data \n"
		"\nDescription: \n"
		"The data part of the payin transactions made during the game contains the game info and this is extracted and displayed in a user readable format.\n"
		"\nArguments: \n"
		"Inputs: \n"
		"%s"
		"\nExample: \n"
		"./bet extract_tx_data tx_id \n",
		cJSON_Print(command_info));
}

void bet_help_scan_command_usage()
{
	dlg_info(
		"\nCommand: \n"
		"scan \n"
		"\nDescription: \n"
		"It scans the entire chips blockchain and look for tx's that contain game_info and store them in the Local DB which further can be used by the explorer nodes to showcase the graphical representation of the games played \n"
		"\nResult: \n"
		"Updates the local DB located at the path ~/.bet/db/pangea.db, to be specific it updates sc_games_info table which has stores all the games played info \n"
		"\nExample: \n"
		"./bet scan \n");
}

void bet_help_vdxf_command_usage()
{
	dlg_info(
		"\nCommand: \n"
		"print \n"
		"\nDescription: \n"
		"Parses the specific key info from the contentmultimap of the given ID.\n"
		"\nResult: \n"
		"Display of parsed key information of an ID \n"
		"\nExample: \n"
		"./bet print <id_name> <key_name>\n"
		"Note: Here id_name can be any ID under the namespace poker.chips10sec@, supported key names are dealers/t_game_ids/t_game_ids/t_player_info\n");

	dlg_info(
		"\nCommand: \n"
		"print_id \n"
		"\nDescription: \n"
		"Parses the contentmultimap of the given ID. Since each ID's contentmultimap holds data of different type, so we need pass the type of the ID to this command\n"
		"\nResult: \n"
		"Display the contentmultimap of the given ID \n"
		"\nExample: \n"
		"./bet print_id <id_name> <id_type>\n"
		"Note: Here id_name can be any ID under the namespace poker.chips10sec@, supported ID types are table/dealer/dealers\n");

	dlg_info(
		"\nCommand: \n"
		"add_dealer \n"
		"\nDescription: \n"
		"Registers the dealer ID, for this command to run one need to have authrotity to update the ID dealers.poker.chips10sec@ ID\n"
		"\nResult: \n"
		"A dealer ID is added to the dealers \n"
		"\nExample: \n"
		"./bet add_dealer <id_name>\n"
		"Note: Before adding dealer ID or name to dealers make sure ID exists\n");

	dlg_info("\nCommand: \n"
		 "list_dealers \n"
		 "\nDescription: \n"
		 "Lists all the dealers that are attached to the dealers key in dealers.poker.chips10sec@ ID\n"
		 "\nResult: \n"
		 "List of available dealer names \n"
		 "\nExample: \n"
		 "./bet list_dealers\n");

	dlg_info("\nCommand: \n"
		 "list_tables \n"
		 "\nDescription: \n"
		 "Lists all the tables that are hosted by all the dealers\n"
		 "\nResult: \n"
		 "List of tables info \n"
		 "\nExample: \n"
		 "./bet list_tables\n");

	dlg_info(
		"\nCommand: \n"
		"reset_id \n"
		"\nDescription: \n"
		"Reset the contentmultimap of an ID, meaning set it to NULL, to execute this command the authority to the ID is needed\n"
		"\nResult: \n"
		"Latest CMM of the ID is set to NULL \n"
		"\nExample: \n"
		"./bet reset_id <id_name>\n");
}

void bet_help_print_command_usage(void)
{
	dlg_info(
		"\nCommand: \n"
		"print \n"
		"\nDescription: \n"
		"Print information about a specific ID and key\n"
		"\nUsage: \n"
		"./bet print <id> <key>\n"
		"\nArguments: \n"
		"<id>: The ID to query\n"
		"<key>: The key to retrieve information for\n"
		"\nExample: \n"
		"./bet print myid table_info\n"
		"\nNote: \n"
		"This command supports printing information for various keys such as table_info, player_info, dealers, game_id, and game_info.\n"
		"The output format depends on the key type.\n");
}

void bet_help_print_id_command_usage()
{
	dlg_info("\nCommand: \n"
		 "print_id \n"
		 "\nDescription: \n"
		 "Print information about a specific ID and its type\n"
		 "\nUsage: \n"
		 "./bet print_id <id_name> <type>\n"
		 "\nArguments: \n"
		 "<id_name>: The name of the ID to query\n"
		 "<type>: The type of ID information to retrieve (e.g., 'dealers', 'table', 'dealer', etc.)\n"
		 "\nExample: \n"
		 "./bet print_id myid table\n");
}
void bet_help_reset_id_command_usage(void)
{
	dlg_info(
		"\nCommand: \n"
		"reset_id \n"
		"\nDescription: \n"
		"Reset the contentmultimap (CMM) of an ID to NULL. This command requires authority over the ID to execute.\n"
		"\nUsage: \n"
		"./bet reset_id <id_name>\n"
		"\nArguments: \n"
		"<id_name>: The name of the ID whose CMM you want to reset\n"
		"\nResult: \n"
		"The latest CMM of the specified ID is set to NULL\n"
		"\nExample: \n"
		"./bet reset_id myid\n"
		"\nNote: \n"
		"This command should be used with caution as it resets all the information associated with the ID.\n"
		"Ensure you have the necessary permissions before executing this command.\n");
}

void bet_help_raise_registration_dispute_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "dealer_id", "ID of the dealer raising the dispute");
	cJSON_AddStringToObject(command_info, "action", "Action to take: 'refund' or 'add_dealer'");

	dlg_info("\nCommand: \n"
		 "raise_dispute\n"
		 "\nDescription: \n"
		 "Raises a dispute for a dealer registration transaction\n"
		 "\nArguments: \n"
		 "Inputs: \n"
		 "%s"
		 "\nResult: \n"
		 "Creates a dispute transaction that will be processed by the block processor\n"
		 "\nExample: \n"
		 "./bet raise_registration_dispute \"dealer_id\" \"refund\"",
		 cJSON_Print(command_info));
}

void bet_help_register_dealer_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "dealer_id", "ID of the dealer to register");

	dlg_info("\nCommand: \n"
		 "register_dealer\n"
		 "\nDescription: \n"
		 "Registers a dealer by creating a registration transaction and storing it in the dealer's ID\n"
		 "\nArguments: \n"
		 "Inputs: \n"
		 "%s"
		 "\nResult: \n"
		 "Creates a registration transaction and stores it in the dealer's ID for future reference\n"
		 "\nExample: \n"
		 "./bet register_dealer \"dealer_id\"",
		 cJSON_Print(command_info));
}

void bet_help_deregister_dealer_command_usage()
{
	cJSON *command_info = cJSON_CreateObject();
	cJSON_AddStringToObject(command_info, "dealer_id", "ID of the dealer to deregister");

	dlg_info("\nCommand: \n"
		 "deregister_dealer\n"
		 "\nDescription: \n"
		 "Deregisters a dealer by creating a deregistration transaction and storing it in the dealer's ID\n"
		 "\nArguments: \n"
		 "Inputs: \n"
		 "%s"
		 "\nResult: \n"
		 "Creates a deregistration transaction and stores it in the dealer's ID for future reference\n"
		 "\nExample: \n"
		 "./bet deregister_dealer \"dealer_id\"",
		 cJSON_Print(command_info));
}

// clang-format off
void bet_help_command(char *command)
{
    if (!command) {
        dlg_error("Invalid command: NULL");
        return;
    }

    typedef struct {
        const char *cmd;
        void (*help_func)(void);
    } CommandHelp;

    const CommandHelp command_helps[] = {
        {"cashier", bet_help_cashier_command_usage},
        {"cashierd", bet_help_cashier_command_usage},
        {"dcv", bet_help_dcv_command_usage},
        {"dealer", bet_help_dcv_command_usage},
        {"extract_tx_data", bet_help_extract_tx_data_command_usage},
        {"game", bet_help_game_command_usage},
        {"player", bet_help_player_command_usage},
        {"spendable", bet_help_spendable_command_usage},
        {"scan", bet_help_scan_command_usage},
        {"withdraw", bet_help_withdraw_command_usage},
        {"vdxf", bet_help_vdxf_command_usage},
        {"verus", bet_help_vdxf_command_usage},
        {"print", bet_help_print_command_usage},
		{"print_id", bet_help_print_id_command_usage},
        {"reset_id", bet_help_reset_id_command_usage},
        {"raise_registration_dispute", bet_help_raise_registration_dispute_command_usage},
        {"register_dealer", bet_help_register_dealer_command_usage},
        {"deregister_dealer", bet_help_deregister_dealer_command_usage},
        {NULL, NULL}  // Sentinel to mark the end of the array
    };

    for (const CommandHelp *ch = command_helps; ch->cmd != NULL; ch++) {
        if (strcmp(command, ch->cmd) == 0) {
            ch->help_func();
            return;
        }
    }

    dlg_info("The command %s is not yet supported by bet", command);
}