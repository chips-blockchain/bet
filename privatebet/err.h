#ifndef ERR_H
#define ERR_H

#include <stdio.h>
#include <stdint.h>

#include "../external/dlg/include/dlg/dlg.h"

/******************************************************************************
All the errors that come across in bet are defined here. The error numbers are assigned with the following criteria
* Error No's 1-32 reserved for bet gameplay
* Error No's 33-64 reserved for CHIPS and LN related issues
* All other errors numbered from 65 and so on with the spacing for new errors 
******************************************************************************/
// clang-format off
#define MAX_ERR_NO 						128

typedef enum {
    // Bet Gameplay Errors
    OK = 0,
    ERR_RESERVED = 1,
    ERR_DECRYPTING_OWN_SHARE = 2,
    ERR_DECRYPTING_OTHER_SHARE = 3,
    ERR_CARD_RETRIEVING_USING_SS = 4,
    ERR_PT_PLAYER_UNAUTHORIZED = 5,
    ERR_NOT_ENOUGH_PLAYERS = 6,
    ERR_ALL_CARDS_DRAWN = 7,
    ERR_DCV_COMMISSION_MISMATCH = 8,
    ERR_DEALER_TABLE_FULL = 9,
    ERR_INVALID_POS = 10,
    ERR_NO_TURN = 11,
    ERR_BET_AMOUNTS_MISMATCH = 12,
    ERR_PAYOUT_TX = 13,
    ERR_PLAYER_STAKE_MISMATCH = 14,
    ERR_GAME_RECORD_TX = 15,

    // Chips Errors (33-48)
    ERR_CHIPS_CREATE_RAW_TX = 33,
    ERR_CHIPS_GET_RAW_TX = 34,
    ERR_CHIPS_DECODE_TX = 35,
    ERR_CHIPS_TX_SIGNING = 36,
    ERR_CHIPS_INVALID_TX = 37,
    ERR_CHIPS_INSUFFICIENT_FUNDS = 38,
    ERR_CHIPS_TX_FAILED = 39,
    ERR_CHIPS_COMMAND = 40,

    // LN Errors (49-64)
    ERR_LN = 49,
    ERR_LN_CHANNEL_ESTABLISHMENT = 50,
    ERR_LN_IS_NOT_OVER_TOR = 51,
    ERR_LN_INSUFFICIENT_FUNDS = 52,
    ERR_LN_FUNDCHANNEL = 53,
    ERR_LN_PAY = 54,
    ERR_LN_ADDRESS_TYPE_MISMATCH = 55,
    ERR_LN_COMMAND = 56,
    ERR_LN_INVOICE_CREATE = 57,
    ERR_LN_NEWADDR = 58,
    ERR_LN_LISTFUNDS = 59,
    ERR_LN_CONNECT = 60,
    ERR_LN_LISTPEERS = 61,
    ERR_LN_PEERCHANNEL_STATE = 62,

    // Parsing Errors
    ERR_INI_PARSING = 65,
    ERR_JSON_PARSING = 66,

    // NNG Related Errors
    ERR_NNG_SEND = 67,
    ERR_NNG_BINDING = 68,

    // PTHREAD Errors
    ERR_PTHREAD_LAUNCHING = 69,
    ERR_PTHREAD_JOINING = 70,

    // SQLITE Errors
    ERR_SQL = 71,

    // Standard Errors
    ERR_MEMORY_ALLOC = 72,
    ERR_ARGS_NULL = 73,
    ERR_ARG_SIZE_TOO_LONG = 74,

    // Network errors
    ERR_PORT_BINDING = 75,

    // VDXF ID Related errors
    ERR_NO_DEALERS_FOUND = 101,
    ERR_NO_TABLES_FOUND = 102,
    ERR_UPDATEIDENTITY = 103,
    ERR_SENDCURRENCY = 104,
    ERR_PLAYER_NOT_EXISTS = 105,
    ERR_T_PLAYER_INFO_NULL = 106,
    ERR_T_PLAYER_INFO_CORRUPTED = 107,
    ERR_T_TABLE_INFO_NULL = 108,
    ERR_TABLE_DECODING_FAILED = 109,
    ERR_PAYIN_TX_INVALID_FUNDS = 110,
    ERR_TABLE_IS_FULL = 111,
    ERR_PA_EXISTS = 112,
    ERR_WRONG_PA_TX_ID_FORMAT = 113,
    ERR_DUP_PAYIN_UPDATE_REQ = 114,
    ERR_PA_NOT_ADDED_TO_TABLE = 115,
    ERR_ID_NOT_FOUND = 116,
    ERR_INVALID_PLAYER_ID = 117,
    ERR_PLAYER_DECK_SHUFFLING = 118,
    ERR_GAME_ID_NOT_FOUND = 119,
    ERR_T_PLAYER_INFO_UPDATE = 120,
    ERR_NO_PAYIN_DATA = 121,
    ERR_TABLE_LAUNCH = 122,
    ERR_GAME_STATE_UPDATE = 123,
    ERR_DECK_BLINDING_DEALER = 124,
    ERR_DECK_BLINDING_CASHIER = 125,
    ERR_INVALID_TABLE_STATE = 126,
    ERR_BV_UPDATE = 127,
    ERR_NO_TX_INFO_AVAILABLE = 128,
    ERR_CARD_DECODING_FAILED = 129,
    ERR_TABLE_IS_NOT_STARTED = 130,
    ERR_CONFIG_PLAYER_ARGS = 131,
    ERR_CONFIG_DEALER_ARGS = 132,
    ERR_CONFIG_CASHIER_ARGS = 133,
    ERR_CONFIG_BLOCKCHAIN_ARGS = 134,
    ERR_IDS_NOT_CONFIGURED = 135,
    ERR_ID_AUTH = 136,
    ERR_ADDR_AUTH = 137,
    ERR_DEALER_UNREGISTERED = 138,
    ERR_NULL_KEY = 139,
    ERR_NULL_ID = 140,
    ERR_TABLE_UNREGISTERED = 141,
    ERR_DUPLICATE_PLAYERID = 142,
    ERR_PLAYER_NOT_ADDED = 143,
    ERR_CASHIERS_ID_NOT_FOUND = 144,
    ERR_DEALERS_ID_NOT_FOUND = 145,
    ERR_NO_DEALERS_REGISTERED = 146
} bet_error_t;

// clang-format on
const char *bet_err_str(int32_t err_no);
#endif
