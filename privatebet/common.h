#ifndef COMMON_H
#define COMMON_H

#define CARDS777_MAXCARDS 52 //52    //
#define CARDS777_MAXPLAYERS 10 //9   //
#define CARDS777_MAXROUNDS 3 //9   //
#define CARDS777_MAXCHIPS 1000
#define CARDS777_CHIPSIZE (SATOSHIDEN / CARDS777_MAXCHIPS)
#define BET_PLAYERTIMEOUT 15
#define BET_GAMESTART_DELAY 10
#define BET_RESERVERATE 1.025
#define LN_FUNDINGERROR "\"Cannot afford funding transaction\""

extern bits256 v_hash[CARDS777_MAXCARDS][CARDS777_MAXCARDS];
extern bits256 g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
extern int32_t permis_d[CARDS777_MAXCARDS],permis_b[CARDS777_MAXCARDS];
extern bits256 Myprivkey,Mypubkey;
extern int32_t IAMHOST;
extern uint16_t LN_port;
extern int32_t Gamestart,Gamestarted,Lastturni;
extern bits256 deckid;
extern uint8_t sharenrs[256];
extern char *LN_idstr,Host_ipaddr[64],Host_peerid[67],Host_channel[64];
extern int32_t Num_hostrhashes,Chips_paid;
extern bits256 playershares[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

#define hand_size 2

#endif


