#ifndef ERR_H
#define ERR_H

#include <stdio.h>
#include <stdint.h>

#include "../external/dlg/include/dlg/dlg.h"

/******************************************************************************
All the errors that come across in bet are defined here. The error numbers are assigned with the following criteria
* Error No's 1-32 reserved for bet gameplay
* Error No's 33-64 reserved for CHIPS and LN related issues
* All other erros numbered from 65 and so on with the spacing for new errors 
******************************************************************************/
// clang-format off
#define MAX_ERR_NO 						128

//Bet Gameplay Errors
#define OK 								0
#define ERR_RESERVED 					1
#define ERR_DECRYPTING_OWN_SHARE 		2
#define ERR_DECRYPTING_OTHER_SHARE 		3
#define ERR_CARD_RETRIEVING_USING_SS 	4
#define ERR_PT_PLAYER_UNAUTHORIZED 		5
#define ERR_NOT_ENOUGH_PLAYERS 			6
#define ERR_ALL_CARDS_DRAWN 			7
#define ERR_DCV_COMMISSION_MISMATCH 	8
#define ERR_DEALER_TABLE_FULL 			9
#define ERR_INVALID_POS 				10
#define ERR_NO_TURN 					11
#define ERR_BET_AMOUNTS_MISMATCH 		12
#define ERR_PAYOUT_TX 					13
#define ERR_PLAYER_STAKE_MISMATCH 		14
#define ERR_GAME_RECORD_TX				15

//Chips Errors (33-48)
#define ERR_CHIPS_CREATE_RAW_TX			33
#define ERR_CHIPS_GET_RAW_TX 			34
#define ERR_CHIPS_DECODE_TX 			35
#define ERR_CHIPS_TX_SIGNING 			36
#define ERR_CHIPS_INVALID_TX 			37
#define ERR_CHIPS_INSUFFICIENT_FUNDS 	38
#define ERR_CHIPS_TX_FAILED 			39
#define ERR_CHIPS_COMMAND 				40


//LN Errors (49-64)
#define ERR_LN 							49
#define ERR_LN_CHANNEL_ESTABLISHMENT 	50
#define ERR_LN_IS_NOT_OVER_TOR 			51
#define ERR_LN_INSUFFICIENT_FUNDS 		52
#define ERR_LN_FUNDCHANNEL 				53
#define ERR_LN_PAY 						54
#define ERR_LN_ADDRESS_TYPE_MISMATCH 	55
#define ERR_LN_COMMAND 					56
#define ERR_LN_INVOICE_CREATE 			57
#define ERR_LN_NEWADDR 					58
#define ERR_LN_LISTFUNDS 				59
#define ERR_LN_CONNECT 					60
#define ERR_LN_LISTPEERS 				61
#define ERR_LN_PEERCHANNEL_STATE 		62


//Parsing Errors
#define ERR_INI_PARSING 				65
#define ERR_JSON_PARSING 				66

//NNG Related Errors
#define ERR_NNG_SEND 					67
#define ERR_NNG_BINDING 				68

//PTHREAD Errors
#define ERR_PTHREAD_LAUNCHING 			69
#define ERR_PTHREAD_JOINING 			70

//SQLITE Errors
#define ERR_SQL 						71

//Standard Errors
#define ERR_MEMORY_ALLOC 				72
#define ERR_ARGS_NULL 					73
#define ERR_ARG_SIZE_TOO_LONG 			74

//Network erros
#define ERR_PORT_BINDING				75

//VDXF ID Related error
#define ERR_NO_DEALERS_FOUND			101
#define ERR_NO_TABLES_FOUND				102
#define ERR_UPDATEIDENTITY 				103
#define ERR_SENDCURRENCY				104
#define ERR_PLAYER_NOT_EXISTS			105
#define ERR_T_PLAYER_INFO_NULL			106
#define ERR_T_PLAYER_INFO_CORRUPTED     107
#define ERR_T_TABLE_INFO_NULL			108
#define ERR_TABLE_DECODING_FAILED		109
#define ERR_PAYIN_TX_INVALID_FUNDS		110
#define ERR_TABLE_IS_FULL				111
#define ERR_PA_EXISTS					112
#define ERR_WRONG_PA_TX_ID_FORMAT		113
#define ERR_DUP_PAYIN_UPDATE_REQ		114
#define ERR_PA_NOT_ADDED_TO_TABLE		115
#define ERR_ID_NOT_FOUND				116
#define ERR_NOT_ENOUGH_FUNDS			117
#define ERR_INVALID_PLAYER_ID			118
#define ERR_PLAYER_DECK_SHUFFLING		119
#define ERR_GAME_ID_NOT_FOUND			120
#define ERR_T_PLAYER_INFO_UPDATE		121
#define ERR_NO_PAYIN_DATA				122
#define ERR_TABLE_LAUNCH				123
#define ERR_GAME_STATE_UPDATE			124
#define ERR_DECK_BLINDING_DEALER		125
#define ERR_DECK_BLINDING_CASHIER		126
#define ERR_INVALID_TABLE_STATE			127
#define ERR_BV_UPDATE					128
#define ERR_NO_TX_INFO_AVAILABLE		129
#define ERR_CARD_DECODING_FAILED		130
#define ERR_TABLE_IS_NOT_STARTED        131
#define ERR_CONFIG_PLAYER_ARGS          132
#define ERR_CONFIG_DEALER_ARGS          133
#define ERR_CONFIG_CASHIER_ARGS         134
#define ERR_CONFIG_BLOCKCHAIN_ARGS      135
#define ERR_IDS_NOT_CONFIGURED			136
#define ERR_ID_AUTH						137
#define ERR_ADDR_AUTH					138
#define ERR_DEALER_UNREGISTERED         139
#define ERR_NULL_KEY                    140
#define ERR_NULL_ID						141
#define ERR_TABLE_UNREGISTERED          142
#define ERR_DUPLICATE_PLAYERID          143
#define ERR_PLAYER_NOT_ADDED            144
#define ERR_CASHIERS_ID_NOT_FOUND       145
#define ERR_DEALERS_ID_NOT_FOUND        146
#define ERR_NO_DEALERS_REGISTERED       147

// clang-format on
const char *bet_err_str(int32_t err_no);
#endif
