#include <stdio.h>
#include <string.h>

#include "err.h"

const char *bet_err_str(int32_t err_no)
{
	switch (err_no) {
	case OK:
		return "No Error";
		break;
	case ERR_DECRYPTING_OWN_SHARE:
		return "Failed failed to decrypt its own shamir secret shard to retrieve the card";
		break;
	case ERR_DECRYPTING_OTHER_SHARE:
		return "Player failed to decrypt the other players shamir secret shard to retrieve the card";
		break;
	case ERR_CARD_RETRIEVING_USING_SS:
		return "Error in retrieving the card by combining all shamir secret shards";
		break;
	case ERR_PT_PLAYER_UNAUTHORIZED:
		return "Player is not unauthorized to join the private table hosting by the dealer";
		break;
	case ERR_NOT_ENOUGH_PLAYERS:
		return "There should be atleast two players needed to play the game";
		break;
	case ERR_ALL_CARDS_DRAWN:
		return "All cards are drawn from the deck";
		break;
	case ERR_DCV_COMMISSION_MISMATCH:
		return "Dealer commission is not within the limits set by the player";
		break;
	case ERR_DEALER_TABLE_FULL:
		return "Dealer table is FULL";
		break;
	case ERR_INVALID_POS:
		return "Invalid position of the player on table";
		break;
	case ERR_NO_TURN:
		return "Couldn't able to find next turn";
		break;
	case ERR_BET_AMOUNTS_MISMATCH:
		return "Error in bet amount mismatch";
		break;
	case ERR_GAME_RECORD_TX:
		return "Error in recording the game moves on to the CHIPS blockchain";
		break;
	case ERR_PAYOUT_TX:
		return "Error in making Payout TX";
		break;
	case ERR_PLAYER_STAKE_MISMATCH:
		return "There is mismatch b/w the stake value sent by the player with the stake value that is present at the dealer";
		break;
	case ERR_CHIPS_TX_SIGNING:
		return "Error in signing the CHIPS tx";
		break;
	case ERR_CHIPS_INVALID_TX:
		return "Player tx is invalid";
		break;
	case ERR_LN:
		return "An LN Error in performing the LN operation";
		break;
	case ERR_LN_CHANNEL_ESTABLISHMENT:
		return "Error in establishing LN channel";
		break;
	case ERR_LN_IS_NOT_OVER_TOR:
		return "The LN channel is not established over TOR";
		break;
	case ERR_CHIPS_INSUFFICIENT_FUNDS:
		return "CHIPS Wallet doesn't have sufficient funds";
		break;
	case ERR_LN_INSUFFICIENT_FUNDS:
		return "LN Wallet doesn't have sufficient funds";
		break;
	case ERR_CHIPS_TX_FAILED:
		return "Error in making CHIPS TX";
		break;
	case ERR_LN_FUNDCHANNEL:
		return "Funding LN channel got failed";
		break;
	case ERR_LN_PAY:
		return "Paying LN invoice got failed";
		break;
	case ERR_LN_ADDRESS_TYPE_MISMATCH:
		return "LN address type is not matched with the one configured";
		break;
	case ERR_LN_COMMAND:
		return "Error occured while running LN command";
		break;
	case ERR_CHIPS_COMMAND:
		return "Error occured while running Chips command";
		break;
	case ERR_LN_INVOICE_CREATE:
		return "Error in creating LN invoice";
		break;
	case ERR_LN_NEWADDR:
		return "Error in creating new LN wallet address";
		break;
	case ERR_LN_LISTFUNDS:
		return "LN error occured while running listfunds";
		break;
	case ERR_LN_CONNECT:
		return "Error in connecting to LN Channel";
		break;
	case ERR_LN_LISTPEERS:
		return "Error in listing peers";
		break;
	case ERR_LN_PEERCHANNEL_STATE:
		return "Error in finding the peer channel state";
		break;
	case ERR_CHIPS_GET_RAW_TX:
		return "Error in getting CHIPS raw TX";
		break;
	case ERR_CHIPS_DECODE_TX:
		return "Error in decoding CHIPS raw TX";
		break;
	case ERR_INI_PARSING:
		return "Error in parsing the ini configuration file";
		break;
	case ERR_JSON_PARSING:
		return "Error in parsing the json configuration file";
		break;
	case ERR_NNG_SEND:
		return "Error in sending the data using Nano sockets";
		break;
	case ERR_NNG_BINDING:
		return "Error in binding to the Nano sockets";
		break;
	case ERR_PTHREAD_LAUNCHING:
		return "Error in launching the thread";
		break;
	case ERR_PTHREAD_JOINING:
		return "Error in joining the parent thread";
		break;
	case ERR_SQL:
		return "Error occured in performing the sqlite operation";
		break;
	case ERR_MEMORY_ALLOC:
		return "Memory allocation failed";
		break;
	case ERR_ARGS_NULL:
		return "Command arguments are NULL";
		break;
	case ERR_ARG_SIZE_TOO_LONG:
		return "Argument size is too long";
		break;
	default:
		return "This error is not handled yet...";
	}
}
