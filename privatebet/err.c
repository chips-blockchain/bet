#include <stdio.h>
#include "err.h"

void bet_err_str(int32_t err_no)
{
	switch (err_no) {
	case ERR_DECRYPTING_OWN_SHARE:
		dlg_error("Failed failed to decrypt its own shamir secret shard to retrieve the card");
		break;
	case ERR_DECRYPTING_OTHER_SHARE:
		dlg_error("Player failed to decrypt the other players shamir secret shard to retrieve the card");
		break;
	case ERR_CARD_RETRIEVING_USING_SS:
		dlg_error("Error in retrieving the card by combining all shamir secret shards");
		break;
	case ERR_PT_PLAYER_UNAUTHORIZED:
		dlg_error("Player is unauthorized to join the private table hosting by the dealer");
		break;
	case ERR_CHIPS_TX_SIGNING:
		dlg_error("Error in signing the CHIPS tx");
		break;
	case ERR_CHIPS_INVALID_TX:
		dlg_error("The tx made by the player marked invalid by the dealer");
		break;
	case ERR_LN:
		dlg_error("An LN Error in performing the LN operation");
		break;
	case ERR_LN_CHANNEL_ESTABLISHMENT:
		dlg_error("Error in establishing LN channel");
		break;
	case ERR_INI_PARSING:
		dlg_error("Error in parsing the ini configuration file");
		break;
	case ERR_JSON_PARSING:
		dlg_error("Error in parsing the json configuration file");
		break;
	case ERR_NNG_SEND:
		dlg_error("Error in sending the data using Nano sockets");
		break;
	case ERR_NNG_BINDING:
		dlg_error("Error in binding to the Nano sockets");
		break;
	case ERR_PTHREAD_LAUNCHING:
		dlg_error("Error in launching the thread");
		break;
	case ERR_PTHREAD_JOINING:
		dlg_error("Error in joining the parent thread");
		break;
	case ERR_SQL:
		dlg_error("Error occured in performing the sqlite operation");
		break;
	default:
		dlg_error("The err_no :: %d is not handled yet...", err_no);
	}
}
