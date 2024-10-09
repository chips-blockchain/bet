#include "bet.h"
#include "help.h"
#include "switchs.h"

void bet_command_info()
{
	const char *help_text = "\nAvailable Commands:\n"
				"\n==Dealer==\n"
				"dcv <ipv4_address>     Start the dealer node\n"
				"\n==Player==\n"
				"player                 Start the player node\n"
				"\n==Cashier==\n"
				"cashierd cashier <ipv4_address>  Start the cashier node\n"
				"\n==DRP (Dispute Resolution Protocol)==\n"
				"game info <fail|success>         Display game information\n"
				"game solve                       Resolve all disputes\n"
				"game dispute <tx_id>             Resolve specific dispute\n"
				"\n==Wallet==\n"
				"withdraw <amount> <chips_address>  Withdraw specific amount\n"
				"withdraw all <chips_address>       Withdraw all funds\n"
				"spendable                          List spendable transactions\n"
				"consolidate                        Consolidate funds\n"
				"tx_split <m> <n>                   Split transaction (m into n parts)\n"
				"extract_tx_data <tx_id>            Extract transaction data\n"
				"\n==Blockchain Scanner==\n"
				"scan                               Scan blockchain for game info\n"
				"\n==VDXF ID Commands==\n"
				"print_id <id_name> <type>          Print ID information\n"
				"print <id_name> <key_name>         Print specific key information\n"
				"add_dealer <dealer_id>             Add a dealer\n"
				"list_dealers                       List all dealers\n"
				"list_tables                        List all tables\n"
				"reset_id                           Reset ID information\n"
				"\n==Help==\n"
				"help <command>         Get detailed help for a command\n"
				"                       Supported commands: cashier, dealer, player, game,\n"
				"                       spendable, scan, withdraw, verus, vdxf\n"
				"\nFor more information about a specific command, use: ./bet help <command>\n";

	dlg_info("%s", help_text);
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