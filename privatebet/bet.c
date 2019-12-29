/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

// https://lists.linuxfoundation.org/pipermail/lightning-dev/2016-January/000403.html
// ^ is multisig

// jl777: oracle needs to include other data like deckid also, timestamp! thanks
// cryptographer dealer needs to timestamp and sign players need to sign their
// actions and gameeval deterministic sort new method for layered dealing, old
// method for layered shuffle
// libscott [11:08 PM]
// the observer is the chain. the state machine doesnt need to be executed on
// chain, but the HEAD state of the game should be notarised on a regular basis
// considering the case where the dealer uniquely generates the blinding value
// for each card and generates the M of N shard of it and distributes it among
// the players...

//[9:09]
// to get to know the card at any given time the player must know atleast M
// shards from it's peers..

//[11:08]
// Ie, it's the responsibility of each player to notarise that state after each
// move is made

// redo unpaid deletes
//  from external: git submodule add
//  https://github.com/ianlancetaylor/libbacktrace.git

#include "bet.h"
#include "../includes/curl/curl.h"
#include "../log/macrologger.h"
#include "cards777.h"
#include "cashier.h"
#include "client.h"
#include "commands.h"
#include "common.h"
#include "gfshare.h"
#include "host.h"
#include "network.h"
#include "table.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct privatebet_info *BET_player_global = NULL;
struct privatebet_vars *Player_VARS_global = NULL;

bits256 Myprivkey, Mypubkey;
int32_t IAMHOST;
uint16_t LN_port;
int32_t Gamestart, Gamestarted, Lastturni;
uint8_t sharenrs[256];
bits256 deckid;
char *LN_idstr, Host_ipaddr[64], Host_peerid[67], Host_channel[64];
int32_t Num_hostrhashes, Chips_paid;
bits256 playershares[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS];

int32_t IAMLP;
int32_t Maxplayers = 10;
int32_t permis_d[CARDS777_MAXCARDS], permis_b[CARDS777_MAXCARDS];
bits256 *allshares = NULL;
bits256 v_hash[CARDS777_MAXCARDS][CARDS777_MAXCARDS];
bits256 g_hash[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS];
struct enc_share *g_shares = NULL;

char *rootAddress = "RSdMRYeeouw3hepxNgUzHn34qFhn1tsubb"; // donation Address

int main(int argc, char **argv) {
  uint16_t port = 7797 + 1, cashier_port = 7901;
  char bindaddr[128] /*="ipc:///tmp/bet.ipc"*/,
      bindaddr1[128] /*="ipc:///tmp/bet1.ipc"*/, hostip[20];
  uint32_t i, range, numplayers;
  int32_t pubsock = -1, subsock = -1, pullsock = -1, pushsock = -1;
  pthread_t dcv_thrd, bvv_thrd, player_thrd, dcv_backend, bvv_backend, player_backend,
      live_thrd;

  /*
  char *msig="bQJTo8knsbSoU7k9oGADa6qfWGWyJtxC3o";
  char *toaddress="bGmKoyJEz4ESuJCTjhVkgEb2Qkt8QuiQzQ";

  cJSON *temp=BET_createrawmultisigtransaction(0.025,toaddress,msig);
  printf("%s::%d::%s\n",__FUNCTION__,__LINE__,cJSON_Print(temp));
  */
#if 1
  if (argc >= 2)
    strcpy(hostip, argv[2]);
  OS_init();
  libgfshare_init();
  OS_randombytes((uint8_t *)&range, sizeof(range));
  OS_randombytes((uint8_t *)&numplayers, sizeof(numplayers));

  range = (range % 52) + 1;
  numplayers = (numplayers % (CARDS777_MAXPLAYERS - 1)) + 2;
  range = 52;
  numplayers = 2;
  Maxplayers = 2;

  BET_check_sync();

  if ((argc >= 2) && (strcmp(argv[1], "dcv") == 0)) {

#if 1
    /* This code is for sockets*/

    BET_transportname(0, bindaddr, hostip, port);
    pubsock = BET_nanosock(1, bindaddr, NN_PUB);

    BET_transportname(0, bindaddr1, hostip, port + 1);
    pullsock = BET_nanosock(1, bindaddr1, NN_PULL);

#endif

    BET_dcv = calloc(1, sizeof(struct privatebet_info));
    DCV_VARS = calloc(1, sizeof(struct privatebet_vars));

    BET_dcv->pubsock = pubsock;   // BET_nanosock(1,bindaddr,NN_PUB);
    BET_dcv->pullsock = pullsock; // BET_nanosock(1,bindaddr1,NN_PULL);
    BET_dcv->maxplayers =
        (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
    BET_dcv->maxchips = CARDS777_MAXCHIPS;
    BET_dcv->chipsize = CARDS777_CHIPSIZE;
    BET_dcv->numplayers = 0;
    BET_dcv->myplayerid = -2;
    BET_dcv->cardid = -1;
    BET_dcv->turni = -1;
    BET_dcv->no_of_turns = 0;
    BET_betinfo_set(BET_dcv, "demo", range, 0, Maxplayers);

    if (OS_thread_create(&live_thrd, NULL, (void *)BET_dcv_live_loop,
                         (void *)BET_dcv) != 0) {
      printf("error launching BET_clientloop BET_hostloop");
      exit(-1);
    }

    if (OS_thread_create(&dcv_backend, NULL, (void *)BET_dcv_backend_loop,
                         (void *)BET_dcv) != 0) {
      printf("error launching BET_clientloop BET_hostloop");
      exit(-1);
    }
    if (OS_thread_create(&dcv_thrd, NULL, (void *)BET_dcv_frontend_loop, NULL) !=
        0) {
      printf("error launching BET_hostloop for pub.%d pull.%d\n",
             BET_dcv->pubsock, BET_dcv->pullsock);
      exit(-1);
    }

    if (pthread_join(live_thrd, NULL)) {
      printf("\nError in joining the main thread for bvvv");
    }
    if (pthread_join(dcv_backend, NULL)) {
      printf("\nError in joining the main thread for bvvv");
    }

    if (pthread_join(dcv_thrd, NULL)) {
      printf("\nError in joining the main thread for dcv");
    }
  } else if ((argc == 3) && (strcmp(argv[1], "bvv") == 0)) {
#if 1
    /* This code is for sockets*/
    BET_transportname(0, bindaddr, hostip, port);
    subsock = BET_nanosock(0, bindaddr, NN_SUB);

    BET_transportname(0, bindaddr1, hostip, port + 1);
    pushsock = BET_nanosock(0, bindaddr1, NN_PUSH);

#endif
#if 1
    BVV_VARS = calloc(1, sizeof(*BVV_VARS));
    BET_bvv = calloc(1, sizeof(struct privatebet_info));
    BET_bvv->subsock = subsock /*BET_nanosock(0,bindaddr,NN_SUB)*/;
    BET_bvv->pushsock = pushsock /*BET_nanosock(0,bindaddr1,NN_PUSH)*/;
    BET_bvv->maxplayers =
        (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
    BET_bvv->maxchips = CARDS777_MAXCHIPS;
    BET_bvv->chipsize = CARDS777_CHIPSIZE;
    BET_bvv->numplayers = numplayers;
    BET_bvv->myplayerid = -1;
    BET_betinfo_set(BET_bvv, "demo", range, 0, Maxplayers);
#endif

    if (OS_thread_create(&bvv_thrd, NULL, (void *)BET_bvv_backend_loop,
                         (void *)BET_bvv) != 0) {
      printf("error launching BET_clientloop for sub.%d push.%d\n",
             BET_bvv->subsock, BET_bvv->pushsock);
      exit(-1);
    }

    if (OS_thread_create(&bvv_backend, NULL, (void *)BET_bvv_frontend_loop,
                         NULL) != 0) {
      printf("error launching BET_hostloop for pub.%d pull.%d\n",
             BET_bvv->subsock, BET_bvv->pushsock);
      exit(-1);
    }
    if (pthread_join(bvv_backend, NULL)) {
      printf("\nError in joining the main thread for bvvv");
    }
    if (pthread_join(bvv_thrd, NULL)) {
      printf("\nError in joining the main thread for bvvv");
    }
  } else if ((argc == 3) && (strcmp(argv[1], "player") == 0)) {
    BET_transportname(0, bindaddr, hostip, port);
    subsock = BET_nanosock(0, bindaddr, NN_SUB);

    BET_transportname(0, bindaddr1, hostip, port + 1);
    pushsock = BET_nanosock(0, bindaddr1, NN_PUSH);

    Player_VARS_global = calloc(1, sizeof(*Player_VARS_global));

    BET_player_global = calloc(1, sizeof(struct privatebet_info));
    BET_player_global->subsock = subsock /*BET_nanosock(0,bindaddr,NN_SUB)*/;
    BET_player_global->pushsock =
        pushsock /*BET_nanosock(0,bindaddr1,NN_PUSH)*/;
    BET_player_global->maxplayers =
        (Maxplayers < CARDS777_MAXPLAYERS) ? Maxplayers : CARDS777_MAXPLAYERS;
    BET_player_global->maxchips = CARDS777_MAXCHIPS;
    BET_player_global->chipsize = CARDS777_CHIPSIZE;
    BET_player_global->numplayers = numplayers;
    BET_betinfo_set(BET_player_global, "demo", range, 0, Maxplayers);

    if (OS_thread_create(&player_thrd, NULL, (void *)BET_player_backend_loop,
                         (void *)BET_player_global) != 0) {
      printf("\nerror in launching BET_p2p_clientloop_test");
      exit(-1);
    }

    if (OS_thread_create(&player_backend, NULL,
                         (void *)BET_player_frontend_loop, NULL) != 0) {
      printf("error launching BET_hostloop for pub.%d pull.%d\n",
             BET_player_global->subsock, BET_player_global->pushsock);
      exit(-1);
    }

    if (pthread_join(player_thrd, NULL)) {
      printf("\nError in joining the main thread for player");
    }

    if (pthread_join(player_backend, NULL)) {
      printf("\nError in joining the main thread for player %d", i);
    }
  }
#if 0
	else if(strcmp(argv[1],"cashier")==0)
	{
		
		
		BET_transportname(0,bindaddr,hostip,cashier_port);
		pubsock = BET_nanosock(1,bindaddr,NN_PUB);
		
		BET_transportname(0,bindaddr1,hostip,cashier_port+1);
		pullsock = BET_nanosock(1,bindaddr1,NN_PULL);
		
		cashier_info=calloc(1,sizeof(struct cashier));
	
	    cashier_info->pubsock = pubsock;//BET_nanosock(1,bindaddr,NN_PUB);
	    cashier_info->pullsock = pullsock;//BET_nanosock(1,bindaddr1,NN_PULL);
	    if (OS_thread_create(&cashier_t,NULL,(void *)BET_cashier_loop,(void *)cashier_info) != 0 )
		{
			printf("\nerror in launching cashier");
			exit(-1);
		}
		
		
		if(pthread_join(cashier_t,NULL))
		{
		printf("\nError in joining the main thread for cashier");
		}
		
	}
#endif
  else {
    printf("\nInvalid Usage");
    printf("\nFor DCV: ./bet dcv");
    printf("\nFor BVV: ./bet bvv");
    printf("\nFor Player: ./bet player player_id");
  }
#endif
  return 0;
}

bits256 curve25519_fieldelement(bits256 hash) {
  hash.bytes[0] &= 0xf8, hash.bytes[31] &= 0x7f, hash.bytes[31] |= 0x40;
  return (hash);
}

bits256 card_rand256(int32_t privkeyflag, int8_t index) {
  bits256 randval;
  OS_randombytes(randval.bytes, sizeof(randval));
  if (privkeyflag != 0)
    randval.bytes[0] &= 0xf8, randval.bytes[31] &= 0x7f,
        randval.bytes[31] |= 0x40;
  randval.bytes[30] = index;
  return (randval);
}

struct pair256 deckgen_common(struct pair256 *randcards, int32_t numcards) {
  int32_t i;
  struct pair256 key, tmp;
  key.priv = curve25519_keypair(&key.prod);
  for (i = 0; i < numcards; i++) {
    tmp.priv = card_rand256(1, i);
    tmp.prod = curve25519(tmp.priv, curve25519_basepoint9());
    randcards[i] = tmp;
  }
  return (key);
}

void deckgen_common2(struct pair256 *randcards, int32_t numcards) {
  for (int32_t i = 0; i < numcards; i++)
    randcards[i].priv = curve25519_keypair(&randcards[i].prod);
}

void dekgen_vendor_perm(int numcards) { BET_permutation(permis_d, numcards); }

struct pair256 deckgen_player(bits256 *playerprivs, bits256 *playercards,
                              int32_t *permis, int32_t numcards) {
  int32_t i;
  struct pair256 key, randcards[256];
  key = deckgen_common(randcards, numcards);
  BET_permutation(permis, numcards);
  for (i = 0; i < numcards; i++) {
    playerprivs[i] = randcards[i].priv; // permis[i]
    playercards[i] = curve25519(playerprivs[i], key.prod);
  }
  return (key);
}
int32_t sg777_deckgen_vendor(
    int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards,
    bits256 *playercards,
    bits256 deckid) // given playercards[], returns cardprods[] and finalcards[]
{
  static struct pair256 randcards[256];
  static bits256 active_deckid, hash_temp[CARDS777_MAXCARDS];
  int32_t retval = 1;
  bits256 hash, xoverz, tmp[256];

  if (bits256_cmp(deckid, active_deckid) != 0)
    deckgen_common2(randcards, numcards);
  else {
    retval = -1;
    goto end;
  }

  for (int32_t i = 0; i < numcards; i++) {
    xoverz = xoverz_donna(curve25519(randcards[i].priv, playercards[i]));
    vcalc_sha256(0, hash.bytes, xoverz.bytes, sizeof(xoverz));
    hash_temp[i] = hash; // optimization
    tmp[i] = fmul_donna(curve25519_fieldelement(hash), randcards[i].priv);
  }

  for (int32_t i = 0; i < numcards; i++) {
    finalcards[i] = tmp[permis_d[i]];
    g_hash[playerid][i] = hash_temp[permis_d[i]]; // optimization
    cardprods[i] =
        randcards[i].prod; // same cardprods[] returned for each player
  }
end:
  return retval;
}

struct pair256 p2p_bvv_init(bits256 *keys, struct pair256 b_key,
                            bits256 *blindings, bits256 *blindedcards,
                            bits256 *finalcards, int32_t numcards,
                            int32_t numplayers, int32_t playerid,
                            bits256 deckid) {
  int32_t i, j, M;
  uint8_t space[8192];
  bits256 cardshares[CARDS777_MAXPLAYERS];
  struct enc_share temp;

  for (i = 0; i < numcards; i++) {
    blindings[i] = rand256(1);
    blindedcards[i] = fmul_donna(finalcards[permis_b[i]], blindings[i]);
  }

  M = (numplayers / 2) + 1;

  gfshare_calc_sharenrs(sharenrs, numplayers, deckid.bytes,
                        sizeof(deckid)); // same for all players for this round

  for (i = 0; i < numcards; i++) {
    gfshare_calc_shares(cardshares[0].bytes, blindings[i].bytes,
                        sizeof(bits256), sizeof(bits256), M, numplayers,
                        sharenrs, space, sizeof(space));
    // create combined allshares
    for (j = 0; j < numplayers; j++) {
      // printf("%s --> ",bits256_str(hexstr,cardshares[j]));
      BET_ciphercreate(b_key.priv, keys[j], temp.bytes, cardshares[j].bytes,
                       sizeof(cardshares[j]));
      memcpy(
          g_shares[numplayers * numcards * playerid + i * numplayers + j].bytes,
          temp.bytes, sizeof(temp));
    }
  }
  // when all players have submitted their finalcards, blinding vendor can send
  // encrypted allshares for each player, see cards777.c
  return b_key;
}
