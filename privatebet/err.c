#include <stdio.h>
#include <string.h>

#include "err.h"

char *bet_err_str(int32_t err_no)
{
	char *err_str = (char *)malloc(128 * sizeof(char));

	switch (err_no) {
	case ERR_DECRYPTING_OWN_SHARE:
		strcpy(err_str, "Failed failed to decrypt its own shamir secret shard to retrieve the card");
		break;
	case ERR_DECRYPTING_OTHER_SHARE:
		strcpy(err_str, "Player failed to decrypt the other players shamir secret shard to retrieve the card");
		break;
	case ERR_CARD_RETRIEVING_USING_SS:
		strcpy(err_str, "Error in retrieving the card by combining all shamir secret shards");
		break;
	case ERR_PT_PLAYER_UNAUTHORIZED:
		strcpy(err_str, "Player is unauthorized to join the private table hosting by the dealer");
		break;
	case ERR_CHIPS_TX_SIGNING:
		strcpy(err_str, "Error in signing the CHIPS tx");
		break;
	case ERR_CHIPS_INVALID_TX:
		strcpy(err_str, "The tx made by the player marked invalid by the dealer");
		break;
	case ERR_LN:
		strcpy(err_str, "An LN Error in performing the LN operation");
		break;
	case ERR_LN_CHANNEL_ESTABLISHMENT:
		strcpy(err_str, "Error in establishing LN channel");
		break;
	case ERR_INI_PARSING:
		strcpy(err_str, "Error in parsing the ini configuration file");
		break;
	case ERR_JSON_PARSING:
		strcpy(err_str, "Error in parsing the json configuration file");
		break;
	case ERR_NNG_SEND:
		strcpy(err_str, "Error in sending the data using Nano sockets");
		break;
	case ERR_NNG_BINDING:
		strcpy(err_str, "Error in binding to the Nano sockets");
		break;
	case ERR_PTHREAD_LAUNCHING:
		strcpy(err_str, "Error in launching the thread");
		break;
	case ERR_PTHREAD_JOINING:
		strcpy(err_str, "Error in joining the parent thread");
		break;
	case ERR_SQL:
		strcpy(err_str, "Error occured in performing the sqlite operation");
		break;
	default:
		sprintf(err_str, "The err_no :: %d is not handled yet...", err_no);
	}
	return err_str;
}
