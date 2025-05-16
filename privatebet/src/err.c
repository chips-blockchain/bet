#include <stdio.h>
#include <string.h>

#include "err.h"

const char *bet_err_str(int32_t err_no)
{
	switch (err_no) {
	case OK:
		return "No Error";
	case ERR_DECRYPTING_OWN_SHARE:
		return "Failed failed to decrypt its own shamir secret shard to retrieve the card";
	case ERR_DECRYPTING_OTHER_SHARE:
		return "Player failed to decrypt the other players shamir secret shard to retrieve the card";
	case ERR_CARD_RETRIEVING_USING_SS:
		return "Error in retrieving the card by combining all shamir secret shards";
	case ERR_PT_PLAYER_UNAUTHORIZED:
		return "Player is not unauthorized to join the private table hosting by the dealer";
	case ERR_NOT_ENOUGH_PLAYERS:
		return "There should be atleast two players needed to play the game";
	case ERR_ALL_CARDS_DRAWN:
		return "All cards are drawn from the deck";
	case ERR_DCV_COMMISSION_MISMATCH:
		return "Dealer commission is not within the limits set by the player";
	case ERR_DEALER_TABLE_FULL:
		return "Dealer table is FULL";
	case ERR_INVALID_POS:
		return "Invalid position of the player on table";
	case ERR_NO_TURN:
		return "Couldn't able to find next turn";
	case ERR_BET_AMOUNTS_MISMATCH:
		return "Error in bet amount mismatch";
	case ERR_GAME_RECORD_TX:
		return "Error in recording the game moves on to the CHIPS blockchain";
	case ERR_PAYOUT_TX:
		return "Error in making Payout TX";
	case ERR_PLAYER_STAKE_MISMATCH:
		return "There is mismatch b/w the stake value sent by the player with the stake value that is present at the dealer";
	case ERR_CHIPS_TX_SIGNING:
		return "Error in signing the CHIPS tx";
	case ERR_CHIPS_INVALID_TX:
		return "Player tx is invalid";
	case ERR_LN:
		return "An LN Error in performing the LN operation";
	case ERR_LN_CHANNEL_ESTABLISHMENT:
		return "Error in establishing LN channel";
	case ERR_LN_IS_NOT_OVER_TOR:
		return "The LN channel is not established over TOR";
	case ERR_CHIPS_CREATE_RAW_TX:
		return "Error while creating chips raw transaction";
	case ERR_CHIPS_INSUFFICIENT_FUNDS:
		return "CHIPS Wallet doesn't have sufficient funds";
	case ERR_LN_INSUFFICIENT_FUNDS:
		return "LN Wallet doesn't have sufficient funds";
	case ERR_CHIPS_TX_FAILED:
		return "Error in making CHIPS TX";
	case ERR_LN_FUNDCHANNEL:
		return "Funding LN channel got failed";
	case ERR_LN_PAY:
		return "Paying LN invoice got failed";
	case ERR_LN_ADDRESS_TYPE_MISMATCH:
		return "LN address type is not matched with the one configured";
	case ERR_LN_COMMAND:
		return "Error occured while running LN command";
	case ERR_CHIPS_COMMAND:
		return "Error occured while running Chips command";
	case ERR_LN_INVOICE_CREATE:
		return "Error in creating LN invoice";
	case ERR_LN_NEWADDR:
		return "Error in creating new LN wallet address";
	case ERR_LN_LISTFUNDS:
		return "LN error occured while running listfunds";
	case ERR_LN_CONNECT:
		return "Error in connecting to LN Channel";
	case ERR_LN_LISTPEERS:
		return "Error in listing peers";
	case ERR_LN_PEERCHANNEL_STATE:
		return "Error in finding the peer channel state";
	case ERR_CHIPS_GET_RAW_TX:
		return "Error in getting CHIPS raw TX";
	case ERR_CHIPS_DECODE_TX:
		return "Error in decoding CHIPS raw TX";
	case ERR_INI_PARSING:
		return "Error in parsing the ini configuration file";
	case ERR_JSON_PARSING:
		return "Error in parsing the json configuration file";
	case ERR_NNG_SEND:
		return "Error in sending the data using Nano sockets";
	case ERR_NNG_BINDING:
		return "Error in binding to the Nano sockets";
	case ERR_PTHREAD_LAUNCHING:
		return "Error in launching the thread";
	case ERR_PTHREAD_JOINING:
		return "Error in joining the parent thread";
	case ERR_SQL:
		return "Error occured in performing the sqlite operation";
	case ERR_MEMORY_ALLOC:
		return "Memory allocation failed";
	case ERR_ARGS_NULL:
		return "Command arguments are NULL";
	case ERR_ARG_SIZE_TOO_LONG:
		return "Argument size is too long";
	case ERR_PORT_BINDING:
		return "Error in binding to the port";
	case ERR_NO_DEALERS_FOUND:
		return "No delears found";
	case ERR_NO_TABLES_FOUND:
		return "No tables are available";
	case ERR_UPDATEIDENTITY:
		return "Error in updating the ID";
	case ERR_SENDCURRENCY:
		return "Error in sendcurrency";
	case ERR_PLAYER_NOT_EXISTS:
		return "Error player doesn't exists";
	case ERR_T_PLAYER_INFO_NULL:
		return "t_player_info key value is null";
	case ERR_T_PLAYER_INFO_CORRUPTED:
		return "t_player_info either not updated correctly or corrupted";
	case ERR_T_TABLE_INFO_NULL:
		return "No table info exists";
	case ERR_TABLE_DECODING_FAILED:
		return "Decoding of table info failed";
	case ERR_PAYIN_TX_INVALID_FUNDS:
		return "The funds in payin_tx doesn't match the table stake";
	case ERR_TABLE_IS_FULL:
		return "Table is full";
	case ERR_PA_EXISTS:
		return "Player with primaryaddress alreay joined the table";
	case ERR_WRONG_PA_TX_ID_FORMAT:
		return "Error is parsing the pa_tx_id stored in t_player_info";
	case ERR_DUP_PAYIN_UPDATE_REQ:
		return "Duplicate update request";
	case ERR_PA_NOT_ADDED_TO_TABLE:
		return "Player primaryaddress is not added to the table";
	case ERR_ID_NOT_FOUND:
		return "ID not found";
	case ERR_INVALID_PLAYER_ID:
		return "Invalid player id";
	case ERR_PLAYER_DECK_SHUFFLING:
		return "Error occured in player deck shuffling";
	case ERR_GAME_ID_NOT_FOUND:
		return "Game ID is not found";
	case ERR_T_PLAYER_INFO_UPDATE:
		return "Error in updating t_player_info key";
	case ERR_NO_PAYIN_DATA:
		return "There exists no payin tx data";
	case ERR_TABLE_LAUNCH:
		return "Error in launching the table";
	case ERR_GAME_STATE_UPDATE:
		return "Error in updating the game state";
	case ERR_DECK_BLINDING_DEALER:
		return "Error in deck blinding at dealer";
	case ERR_DECK_BLINDING_CASHIER:
		return "Error in deck blinding at cashier";
	case ERR_INVALID_TABLE_STATE:
		return "Error Invalid table state";
	case ERR_BV_UPDATE:
		return "Error in updating the Blinding Value at Table ID";
	case ERR_NO_TX_INFO_AVAILABLE:
		return "No information about TX is available";
	case ERR_CARD_DECODING_FAILED:
		return "Error in decoding the card";
	case ERR_TABLE_IS_NOT_STARTED:
		return "Table is not started";
	case ERR_CONFIG_PLAYER_ARGS:
		return "Error in player config args, check if all the necessary args are confiured in verus_player.ini";
	case ERR_CONFIG_DEALER_ARGS:
		return "Error in dealer config args, check if all the necessary args are confiured in verus_dealer.ini";
	case ERR_CONFIG_CASHIER_ARGS:
		return "Error in cashier config args, check if all the necessary args are confiured in verus_cashier.ini";
	case ERR_CONFIG_BLOCKCHAIN_ARGS:
		return "Error in blockchain config args, check if all the necessary args are confiured in blockchain_config.ini";
	case ERR_IDS_NOT_CONFIGURED:
		return "The mandatory IDs needed to play poker are not created on CHAIN";
	case ERR_ID_AUTH:
		return "Not enough authority to update ID, probably the underlying wallet doesn't have privkeys of the primaryaddresses of ID";
	case ERR_ADDR_AUTH:
		return "Address/PrimaryAddress is not authorized, the wallet doesn't have private keys to spend or sign";
	case ERR_DEALER_UNREGISTERED:
		return "Dealer hasn't been registered with dealers.poker.chips10sec";
	case ERR_NULL_KEY:
		return "Data Key of contentmultimap is NULL, key must be a valid name";
	case ERR_NULL_ID:
		return "ID name is NULL";
	case ERR_TABLE_UNREGISTERED:
		return "Table is unregistered with the Dealer";
	case ERR_DUPLICATE_PLAYERID:
		return "Player ID has already been added to the table";
	case ERR_PLAYER_NOT_ADDED:
		return "Player ID hasn't added to the table";
	case ERR_CASHIERS_ID_NOT_FOUND:
		return "Cashiers ID not found";
	case ERR_DEALERS_ID_NOT_FOUND:
		return "Dealers ID not found";
	case ERR_NO_DEALERS_REGISTERED:
		return "No dealers registered";
	default:
		dlg_error("err_no::%d", err_no);
		return "This error is not handled yet...";
	}
}
