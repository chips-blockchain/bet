#include <stdio.h>
#include <string.h>

#include "err.h"

const char *bet_err_str(int32_t err_no)
{
	switch (err_no) {
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
		return "Player is unauthorized to join the private table hosting by the dealer";
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
	case ERR_CHIPS_TX_SIGNING:
		return "Error in signing the CHIPS tx";
		break;
	case ERR_CHIPS_INVALID_TX:
		return "The tx made by the player marked invalid by the dealer";
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
	case ERR_LN_CHANNEL_FUNDING_FAILED:
		return "Funding LN channel got failed";
		break;
	case ERR_LN_PAY:
		return "Paying LN invoice got failed";
		break;
	case ERR_LN_ADDRESS_TYPE_MISMATCH:
		return "LN address type is not matched with the one configured";
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
	default:
		return "This error is not handled yet...";
	}
}
