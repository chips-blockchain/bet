#include "bet.h"
#include "help.h"
#include "switchs.h"

void bet_command_info()
{
	dlg_info("\n==Dealer==\n"
		 "dcv \"ipv4 address of the dealer node\" \n"
		 "\n==Player==\n"
		 "player \n"
		 "\n==Cashier==\n"
		 "cashierd cashier \"ipv4 address of the cashier node\" \n"
		 "\n==DRP==\n"
		 "game info [fail]/[success] \n"
		 "game solve \n"
		 "game dispute \" Disputed tx to resolve\" \n"
		 "\n==Wallet==\n"
		 "withdraw amount \"chips address\" \n"
		 "withdraw all \"chips address\" \n"
		 "spendable \n"
		 "consolidate \n"
		 "tx_split m n #Where m is splitted into n parts\n"
		 "extract_tx_data tx_id \n"
		 "\n==Blockchain scanner for Explorer==\n"
		 "scan \n"
		 "\n==VDXF ID Commands==\n"
		 "print_id <id_name> <type>\n"
		 "print_id <id_name> <key_name>\n"
		 "\nTo get more info about a specific command try ./bet help command \n");
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

// clang-format off
void bet_help_command(char *command)
{
	switchs(command) {
		cases("cashier")
		cases("cashierd")
			bet_help_cashier_command_usage();
			break;
		cases("dcv")
		cases("dealer")
			bet_help_dcv_command_usage();
			break;
		cases("extract_tx_data")
			bet_help_extract_tx_data_command_usage();
			break;
		cases("game")
			bet_help_game_command_usage();
			break;
		cases("player")
			bet_help_player_command_usage();
			break;
		cases("spendable")
			bet_help_spendable_command_usage();
			break;
		cases("scan")
			bet_help_scan_command_usage();
			break;
		cases("withdraw")
			bet_help_withdraw_command_usage();
			break;
		defaults
			dlg_info("The command %s is not yet supported by bet", command);
	}switchs_end;
}
// clang-format on
