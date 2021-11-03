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
	case ERR_CHIPS_TX_SIGNING:
		return "Error in signing the CHIPS tx";
		break;
	case ERR_CHIPS_INVALID_TX:
		return "The tx made by the player marked invalid by the dealer";
		break;
	case ERR_LN:
		return "An LN Error in performing the LN operation";
		break;
	case ERR_LN_IS_NOT_OVER_TOR:
		return "The LN channel is not established over TOR";
		break;
	case ERR_LN_CHANNEL_ESTABLISHMENT:
		return "Error in establishing LN channel";
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
