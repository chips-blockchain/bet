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
#include "bet.h"
#include "../../includes/curl/curl.h"
#include "macrologger.h"
#include "cards.h"
#include "cashier.h"
#include "client.h"
#include "commands.h"
#include "gfshare.h"
#include "host.h"
#include "network.h"
#include "table.h"
#include "storage.h"
#include "config.h"
#include "heartbeat.h"
#include "misc.h"
#include "help.h"
#include "err.h"
#include "switchs.h"
#include "player.h"
#include "print.h"
#include "test.h"
#include "dealer.h"
#include "blinder.h"
#include "dealer_registration.h"
#include "poker_vdxf.h"
#include "log.h"

#include <string.h>
#include <strings.h>

//#define LIVE_THREAD 0

/* Bet without LN completed development after this block, so only the tx's which generated 
   after this block contain the info of the games played using bet without ln setup.
*/
int64_t sc_start_block = 9693174;

// Betting mode: 0=AUTO (for testing), 1=CLI (read from stdin), 2=GUI (websocket)
int g_betting_mode = BET_MODE_AUTO;

// GUI synchronization for player join approval
pthread_mutex_t gui_join_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gui_join_cond = PTHREAD_COND_INITIALIZER;
int gui_join_approved = 0;  // 0 = waiting, 1 = approved

// GUI synchronization for table finding trigger
pthread_mutex_t gui_table_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gui_table_cond = PTHREAD_COND_INITIALIZER;
int gui_table_requested = 0;  // 0 = waiting, 1 = GUI requested table info

struct privatebet_info *bet_player = NULL;
struct privatebet_vars *player_vars = NULL;

uint8_t sharenrs[256];
bits256 deckid;
bits256 playershares[CARDS_MAXCARDS][CARDS_MAXPLAYERS];
int32_t permis_d[CARDS_MAXCARDS], permis_b[CARDS_MAXCARDS];
bits256 v_hash[CARDS_MAXCARDS][CARDS_MAXCARDS];
bits256 g_hash[CARDS_MAXPLAYERS][CARDS_MAXCARDS];
struct enc_share *g_shares = NULL;

char dealer_ip[20];
char unique_id[65];

struct seat_info player_seats_info[CARDS_MAXPLAYERS];

int32_t player_pos[CARDS_MAXPLAYERS];

/**************************************************************************************************
This value is read from dealer_config.json file, it defines the exact number of players that needs
be joined in order to play the game.
The default value is 2, i.e as atleast two players are required to play the game.
***************************************************************************************************/

int32_t max_players = 2;

static const int32_t poker_deck_size = 52;
int32_t bet_node_type;

static void bet_player_initialize(char *dcv_ip)
{
	// Nanomsg sockets removed - no longer used
	player_vars = calloc(1, sizeof(struct privatebet_vars));

	bet_player = calloc(1, sizeof(struct privatebet_info));
	bet_player->maxplayers = (max_players < CARDS_MAXPLAYERS) ? max_players : CARDS_MAXPLAYERS;
	bet_player->maxchips = CARDS_MAXCHIPS;
	bet_player->chipsize = CARDS_CHIPSIZE;
	bet_player->numplayers = max_players;
	bet_info_set(bet_player, "demo", poker_deck_size, 0, max_players);
}

static void bet_player_deinitialize()
{
	if (bet_player)
		free(bet_player);
	if (player_vars)
		free(player_vars);
}

static void bet_player_thrd(char *dcv_ip)
{
	pthread_t player_thrd, player_backend_write, player_backend_read;

	bet_player_initialize(dcv_ip);
	if (OS_thread_create(&player_thrd, NULL, (void *)bet_player_backend_loop, (void *)bet_player) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}
	if (OS_thread_create(&player_backend_read, NULL, (void *)bet_player_frontend_read_loop, NULL) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}
	if (OS_thread_create(&player_backend_write, NULL, (void *)bet_player_frontend_write_loop, NULL) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}

	if (pthread_join(player_backend_read, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}
	if (pthread_join(player_backend_write, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}
	if (pthread_join(player_thrd, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}

	bet_player_deinitialize();
}

// bet_bvv_thrd, bet_bvv_initialize, bet_bvv_deinitialize removed - nanomsg/pub-sub communication no longer used

// bet_dcv_bvv_initialize removed - dcv_bvv_sock_info struct removed, nanomsg no longer used

static int32_t bet_dcv_initialize(char *dcv_ip)
{
	int32_t retval = OK;
	// Nanomsg sockets removed - no longer used
	bet_dcv = calloc(1, sizeof(struct privatebet_info));
	bet_dcv->maxplayers = (max_players < CARDS_MAXPLAYERS) ? max_players : CARDS_MAXPLAYERS;
	bet_dcv->maxchips = CARDS_MAXCHIPS;
	bet_dcv->chipsize = CARDS_CHIPSIZE;
	bet_dcv->numplayers = 0;
	bet_dcv->myplayerid = -2;
	bet_dcv->cardid = -1;
	bet_dcv->turni = -1;
	bet_dcv->no_of_turns = 0;
	bet_info_set(bet_dcv, "demo", poker_deck_size, 0, max_players);
	bet_dcv->msg = cJSON_CreateObject();

	dcv_vars = calloc(1, sizeof(struct privatebet_vars));

	dcv_vars->turni = 0;
	dcv_vars->round = 0;
	dcv_vars->pot = 0;
	dcv_vars->last_turn = 0;
	dcv_vars->last_raise = 0;
	for (int i = 0; i < CARDS_MAXPLAYERS; i++) {
		dcv_vars->funds[i] = 0;
		for (int j = 0; j < CARDS_MAXROUNDS; j++) {
			dcv_vars->bet_actions[i][j] = 0;
			dcv_vars->betamount[i][j] = 0;
		}
	}

	for (int32_t i = 0; i < CARDS_MAXPLAYERS; i++) {
		player_pos[i] = 0;
	}
	return retval;
}

static void bet_dcv_deinitialize()
{
	if (bet_dcv)
		free(bet_dcv);
	if (dcv_vars)
		free(dcv_vars);
}

static void bet_dcv_thrd(char *dcv_ip)
{
	int32_t retval = OK;
	pthread_t dcv_backend, dcv_thrd;

#ifdef LIVE_THREAD
	pthread_t live_thrd;
#endif

	retval = bet_dcv_initialize(dcv_ip);
	if (retval != OK) {
		dlg_error("%s", bet_err_str(retval));
		exit(-1);
	}
	// bet_dcv_bvv_initialize removed - dcv_bvv_sock_info struct removed

#ifdef LIVE_THREAD
	if (OS_thread_create(&live_thrd, NULL, (void *)bet_dcv_heartbeat_loop, (void *)bet_dcv) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}
#endif
	if (OS_thread_create(&dcv_backend, NULL, (void *)bet_dcv_backend_loop, (void *)bet_dcv) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}
	// bet_dcv_bvv_backend_loop removed - dcv_bvv_sock_info struct removed
	if (OS_thread_create(&dcv_thrd, NULL, (void *)bet_dcv_frontend_loop, NULL) != 0) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_LAUNCHING));
		exit(-1);
	}

	if (pthread_join(dcv_backend, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}
	if (pthread_join(dcv_thrd, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}
	// dcv_bvv_thrd removed - bet_dcv_bvv_backend_loop removed
#ifdef LIVE_THREAD
	if (pthread_join(live_thrd, NULL)) {
		dlg_error("%s", bet_err_str(ERR_PTHREAD_JOINING));
	}
#endif
	bet_dcv_deinitialize();
}

static void bet_set_unique_id()
{
	bits256 randval;
	memset(unique_id, 0x00, sizeof(unique_id));
	OS_randombytes(randval.bytes, sizeof(randval));
	bits256_str(unique_id, randval);
}

static void common_init()
{
	OS_init();
	libgfshare_init();
	// Lightning Network support removed - no longer used
	bet_sqlite3_init();
	bet_parse_cashier_config_ini_file();
}

static void cashier_init()
{
	common_init();
	bet_clear_tables();
}

static void playing_nodes_init()
{
	common_init();
#if 0
	bet_check_cashier_nodes();
#endif
	bet_parse_player_config_ini_file();
}

static void dealer_node_init()
{
	if (0 == strlen(dcv_hosted_gui_url)) {
		sprintf(dcv_hosted_gui_url, "http://%s:1234/", dealer_ip);
	}
	dlg_warn("Delaer GUI URL :: %s", dcv_hosted_gui_url); // dlg_warn is just to highlight the log in the console
	if (0 == check_url(dcv_hosted_gui_url))
		memset(dcv_hosted_gui_url, 0x00, sizeof(dcv_hosted_gui_url));

	bet_set_table_id();
#if 0
	bet_compute_m_of_n_msig_addr();
	bet_game_multisigaddress();
#endif
	bet_init_player_seats_info();
}

static void bet_send_dealer_info_to_cashier(char *dealer_ip)
{
	cJSON *dealer_info = NULL;

	dealer_info = cJSON_CreateObject();
	cJSON_AddStringToObject(dealer_info, "method", "dealer_info");
	cJSON_AddStringToObject(dealer_info, "ip", dealer_ip);

	for (int32_t i = 0; i < no_of_notaries; i++) {
		if (notary_status[i] == 1) {
			bet_msg_cashier(dealer_info, notary_node_ips[i]);
		}
	}
}

static char *bet_pick_dealer()
{
	cJSON *available_dealers = NULL;

	available_dealers = bet_get_available_dealers();
	dlg_info("Available dealers :: %s", cJSON_Print(available_dealers));

	for (int32_t i = 0; i < cJSON_GetArraySize(available_dealers); i++) {
		cJSON *temp = cJSON_GetArrayItem(available_dealers, i);
		if (jint(temp, "dcv_state") == dealer_table_empty) {
			return jstr(temp, "ip");
		}
	}
	return NULL;
}

// Command line argument structure for subcommands
struct bet_args {
	const char *config_file;  // -c, --config: config file path
	const char *table_id;     // -t, --table: table ID
	const char *height;       // -h, --height: block height
	int betting_mode;         // --gui, --cli, --auto
	bool reset;               // --reset (for dealer)
};

// Parse flags starting from index 'start'
static void parse_flags(int argc, char **argv, int start, struct bet_args *args)
{
	memset(args, 0, sizeof(*args));
	args->betting_mode = BET_MODE_GUI; // default
	args->table_id = "t1";             // default table
	
	for (int i = start; i < argc; i++) {
		if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) && i + 1 < argc) {
			args->config_file = argv[++i];
		} else if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--table") == 0) && i + 1 < argc) {
			args->table_id = argv[++i];
		} else if ((strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0) && i + 1 < argc) {
			args->height = argv[++i];
		} else if (strcmp(argv[i], "--gui") == 0) {
			args->betting_mode = BET_MODE_GUI;
		} else if (strcmp(argv[i], "--cli") == 0) {
			args->betting_mode = BET_MODE_CLI;
		} else if (strcmp(argv[i], "--auto") == 0) {
			args->betting_mode = BET_MODE_AUTO;
		} else if (strcmp(argv[i], "--reset") == 0) {
			args->reset = true;
		}
	}
}

// Get subcommand argument (first non-flag after subcommand)
static const char *get_subcmd_arg(int argc, char **argv, int start)
{
	for (int i = start; i < argc; i++) {
		if (argv[i][0] != '-') {
			return argv[i];
		}
	}
	return NULL;
}

void bet_start(int argc, char **argv)
{
	int32_t retval = OK;
	struct bet_args args;
	
	// No arguments provided - show help
	if (argc < 2) {
		bet_command_info();
		return;
	}
	
	const char *cmd = argv[1];

	// Handle newblock command. Still needs the basic config so process_block
	// can find blockchain.ini (for new_block flag) and use the right CLI.
	if (strcmp(cmd, "newblock") == 0 && argc == 3) {
		bet_init_config_paths();
		bet_log_init(NULL);
		bet_parse_verus_ids_keys_config();
		bet_parse_rpc_credentials();
		bet_parse_blockchain_config_ini_file();
		process_block(argv[2]);
		return;
	}

	// Initialize config paths first (needed for logging path)
	bet_init_config_paths();
	
	// Initialize logging BEFORE any dlg calls (default to debug.log, nodes override later)
	bet_log_init(NULL);
	
	// Now initialize other components - all logs will go to file
	bet_parse_verus_ids_keys_config(); // Load Verus IDs and Keys configuration
	bet_parse_rpc_credentials(); // Load RPC credentials for REST API
	bet_set_unique_id();
	bet_parse_blockchain_config_ini_file();
	bet_sqlite3_init();  // Initialize SQLite database for all node types

	//==========================================================================
	// START command: ./bet start <node_type> [options]
	// Node types: player|p, dealer|d, cashier|c
	//==========================================================================
	if (strcmp(cmd, "start") == 0) {
		if (argc < 3) {
			dlg_error("Usage: %s start <player|dealer|cashier> [-c config] [options]", argv[0]);
			return;
		}
		const char *node = argv[2];
		parse_flags(argc, argv, 3, &args);
		
		// Start player node
		if (strcmp(node, "player") == 0 || strcmp(node, "p") == 0) {
			bet_node_type = player;
			
			if (args.config_file) {
				strncpy(verus_player_config_file, args.config_file, PATH_MAX - 1);
				verus_player_config_file[PATH_MAX - 1] = '\0';
			}
			
			// Parse config first to get player_id for logging
			if ((retval = bet_parse_verus_player()) != OK) {
				dlg_error("Failed to parse player configuration: %s", bet_err_str(retval));
				return;
			}
			
			// Initialize logging with player ID
			bet_log_init(player_config.verus_pid);
			dlg_info("Log file: %s", bet_log_get_path());
			
			if (args.config_file) {
				dlg_info("Using player config: %s", verus_player_config_file);
			}
			
			g_betting_mode = args.betting_mode;
			const char *mode_str = (g_betting_mode == BET_MODE_CLI) ? "CLI" :
			                       (g_betting_mode == BET_MODE_AUTO) ? "AUTO" : "GUI";
			dlg_info("Starting player node (mode: %s)", mode_str);
			
			extern int32_t backend_status;
			backend_status = backend_ready;
			dlg_info("Backend status: READY");
			
			pthread_t ws_thread;
			if (g_betting_mode == BET_MODE_GUI) {
				gui_ws_port = player_config.ws_port;
				dlg_info("WebSocket server on port %d", gui_ws_port);
				if (OS_thread_create(&ws_thread, NULL, (void *)bet_player_frontend_loop, NULL) != 0) {
					dlg_error("Failed to start WebSocket thread");
				}
				sleep(1);
			}
			
			bet_player_initialize(NULL);
			retval = handle_verus_player();
			
			if (g_betting_mode == BET_MODE_GUI) {
				pthread_join(ws_thread, NULL);
			}
		}
		// Start dealer node
		else if (strcmp(node, "dealer") == 0 || strcmp(node, "d") == 0) {
			bet_node_type = dealer;
			
			if (args.config_file) {
				strncpy(verus_dealer_config, args.config_file, PATH_MAX - 1);
				verus_dealer_config[PATH_MAX - 1] = '\0';
			}
			
			// Initialize logging - read dealer_id from config for log filename
			dictionary *ini = iniparser_load(verus_dealer_config);
			if (ini) {
				const char *dealer_id = iniparser_getstring(ini, "verus:dealer_id", "dealer");
				bet_log_init(dealer_id);
				iniparser_freedict(ini);
			} else {
				bet_log_init("dealer");
			}
			dlg_info("Log file: %s", bet_log_get_path());
			
			if (args.config_file) {
				dlg_info("Using dealer config: %s", verus_dealer_config);
			}
			
			dlg_info("Starting dealer node%s", args.reset ? " (RESET)" : "");
			retval = bet_parse_verus_dealer_with_reset(args.reset);
			if (retval != OK) {
				dlg_error("Dealer initialization failed: %s", bet_err_str(retval));
			}
		}
		// Start cashier node
		else if (strcmp(node, "cashier") == 0 || strcmp(node, "c") == 0) {
			bet_node_type = cashier;
			
			if (args.config_file) {
				strncpy(cashier_config_ini_file, args.config_file, PATH_MAX - 1);
				cashier_config_ini_file[PATH_MAX - 1] = '\0';
			}
			
			// Initialize logging - use "cashier" as log filename
			bet_log_init("cashier");
			dlg_info("Log file: %s", bet_log_get_path());
			
			if (args.config_file) {
				dlg_info("Using cashier config: %s", cashier_config_ini_file);
			}
			
			dlg_info("Starting cashier node");
			cashier_game_init();
		}
		else {
			dlg_error("Unknown node type: %s (use player, dealer, or cashier)", node);
		}
	}
	//==========================================================================
	// RESET command: ./bet reset <id>
	//==========================================================================
	else if (strcmp(cmd, "reset") == 0) {
		if (argc < 3) {
			dlg_error("Usage: %s reset <id>", argv[0]);
			return;
		}
		const char *id = argv[2];
		if (id_cansignfor(id, &retval)) {
			cJSON *out = update_cmm(id, NULL);
			dlg_info("Reset ID '%s': %s", id, cJSON_Print(out));
		} else {
			dlg_error("Cannot sign for ID: %s", id);
		}
	}
	//==========================================================================
	// SHOW command: ./bet show <id> [--height <block>]
	//==========================================================================
	else if (strcmp(cmd, "show") == 0) {
		if (argc < 3) {
			dlg_error("Usage: %s show <id> [--height <block>]", argv[0]);
			return;
		}
		const char *id = argv[2];
		parse_flags(argc, argv, 3, &args);
		int32_t height = args.height ? atoi(args.height) : chips_get_block_count() - 100;
		poker_print_table_keys(id, height);
	}
	//==========================================================================
	// LIST command: ./bet list <dealers|tables>
	//==========================================================================
	else if (strcmp(cmd, "list") == 0) {
		if (argc < 3) {
			dlg_error("Usage: %s list <dealers|tables>", argv[0]);
			return;
		}
		const char *what = argv[2];
		if (strcmp(what, "dealers") == 0) {
			cJSON *dealers = poker_list_dealers();
			if (dealers) dlg_info("%s", cJSON_Print(dealers));
		} else if (strcmp(what, "tables") == 0) {
			poker_list_tables();
		} else {
			dlg_error("Unknown list type: %s (use dealers or tables)", what);
		}
	}
	//==========================================================================
	// Dealer registration commands (legacy format kept for compatibility)
	//==========================================================================
	else if (strcmp(cmd, "add_dealer") == 0 && (argc == 3 || argc == 4)) {
		retval = add_dealer(argv[2], argc == 4 ? argv[3] : NULL);
	} else if (strcmp(cmd, "register_dealer") == 0 && argc == 3) {
		retval = register_dealer(argv[2]);
	} else if (strcmp(cmd, "deregister_dealer") == 0 && argc == 3) {
		retval = deregister_dealer(argv[2]);
	} else if (strcmp(cmd, "raise_registration_dispute") == 0 && argc == 4) {
		raise_dealer_registration_dispute(argv[2], argv[3]);
	}
	//==========================================================================
	// Game and legacy commands
	//==========================================================================
	else if (strcmp(cmd, "game") == 0) {
		playing_nodes_init();
		bet_handle_game(argc, argv);
	}
	// Transaction commands
	else if (strcmp(cmd, "consolidate") == 0) {
		cJSON *tx = NULL;
		double amount = chips_get_balance() - chips_tx_fee;
		tx = chips_transfer_funds(amount, chips_get_new_address());
		if (tx) {
			dlg_info("Consolidated tx::%s", cJSON_Print(tx));
		}
	} else if (strcmp(cmd, "withdraw") == 0) {
		if (argc == 4) {
			cJSON *tx = NULL;
			double amount = (strcmp(argv[2], "all") == 0) 
				? chips_get_balance() - chips_tx_fee 
				: atof(argv[2]);
			tx = chips_transfer_funds(amount, argv[3]);
			if (tx) {
				dlg_info("tx details::%s", cJSON_Print(tx));
			}
		} else {
			bet_help_withdraw_command_usage();
		}
	} else if (strcmp(cmd, "tx_split") == 0 && argc == 4) {
		do_split_tx_amount(atof(argv[2]), atoi(argv[3]));
	} else if (strcmp(cmd, "spendable") == 0) {
		cJSON *spendable_tx = chips_spendable_tx();
		dlg_info("CHIPS Spendable tx's :: %s\n", cJSON_Print(spendable_tx));
	} else if (strcmp(cmd, "balance") == 0) {
		double balance = chips_get_balance();
		dlg_info("CHIPS Balance: %.8f", balance);
	} else if (strcmp(cmd, "blockcount") == 0) {
		int32_t height = chips_get_block_count();
		dlg_info("Block Height: %d", height);
	}
	// Data extraction and scanning
	else if (strcmp(cmd, "extract_tx_data") == 0) {
		if (argc == 3) {
			cJSON *temp = chips_extract_tx_data_in_JSON(argv[2]);
			if (temp) {
				dlg_info("%s", cJSON_Print(temp));
			}
		} else {
			bet_help_extract_tx_data_command_usage();
		}
	} else if (strcmp(cmd, "scan") == 0) {
		bet_sqlite3_init();
		scan_games_info();
	}
	// Print/display commands
	else if (strcmp(cmd, "print") == 0 && argc > 3) {
		print_vdxf_info(argc, argv);
	} else if (strcmp(cmd, "print_id") == 0 && argc > 3) {
		print_id_info(argc, argv);
	} else if (strcmp(cmd, "print_table_key") == 0 && argc >= 3) {
		print_table_key_info(argc, argv);
	} else if (strcmp(cmd, "print_keys") == 0) {
		// Print all table keys for an ID from a specific block height
		// Usage: ./bet print_keys <id> <block_height>
		if (argc >= 4) {
			const char *id = argv[2];
			int32_t block_height = atoi(argv[3]);
			poker_print_table_keys(id, block_height);
		} else if (argc == 3) {
			// Default to current block - 100
			const char *id = argv[2];
			int32_t block_height = chips_get_block_count() - 100;
			poker_print_table_keys(id, block_height);
		} else {
			printf("Usage: %s print_keys <id> [block_height]\n", argv[0]);
			printf("  id           - Identity to inspect (e.g., t1@, d1@, p1@)\n");
			printf("  block_height - Starting block height (default: current - 100)\n");
			printf("\nExample:\n");
			printf("  %s print_keys t1@ 1000\n", argv[0]);
		}
	}
	// Identity management
	else if (strcmp(cmd, "reset_id") == 0 && argc == 3) {
		if (id_cansignfor(argv[2], &retval)) {
			cJSON *out = update_cmm(argv[2], NULL);
			dlg_info("%s", cJSON_Print(out));
		}
	}
	// Dispute resolution
	else if (strcmp(cmd, "dispute") == 0 && argc >= 3) {
		if (argc >= 4) {
			// dispute raise <game_id> <reason>
			if (strcmp(argv[2], "raise") == 0 && argc >= 5) {
				retval = player_raise_dispute(argv[3], argv[4]);
			}
			// dispute check <game_id>
			else if (strcmp(argv[2], "check") == 0) {
				cJSON *result = player_check_dispute_result(argv[3]);
				if (result) {
					dlg_info("Dispute result: %s", cJSON_Print(result));
				} else {
					dlg_info("No dispute result found for game %s", argv[3]);
				}
			}
		} else {
			dlg_info("Usage:");
			dlg_info("  ./bet dispute raise <game_id> <reason>");
			dlg_info("    reason: no_payout | game_aborted | timeout");
			dlg_info("  ./bet dispute check <game_id>");
		}
	}
	// Help commands
	else if (strcmp(cmd, "h") == 0 || strcmp(cmd, "-h") == 0 || 
		 strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0) {
		if (argc == 3) {
			bet_help_command(argv[2]);
		} else {
			bet_command_info();
		}
	}
	// Unknown command
	else {
		bet_command_info();
	}

end:
	if (retval != OK) {
		dlg_info("Error ::%s", bet_err_str(retval));
	}
}

// test_x() removed - unused function

int main(int argc, char **argv)
{
	bet_start(argc, argv);
	return 0;
}

bits256 curve25519_fieldelement(bits256 hash)
{
	hash.bytes[0] &= 0xf8, hash.bytes[31] &= 0x7f, hash.bytes[31] |= 0x40;
	return (hash);
}

bits256 card_rand256(int32_t privkeyflag, int8_t index)
{
	bits256 randval;
	OS_randombytes(randval.bytes, sizeof(randval));
	if (privkeyflag != 0)
		randval.bytes[0] &= 0xf8, randval.bytes[31] &= 0x7f, randval.bytes[31] |= 0x40;
	randval.bytes[30] = index;
	return (randval);
}

struct pair256 deckgen_common(struct pair256 *randcards, int32_t numcards)
{
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

void deckgen_common2(struct pair256 *randcards, int32_t numcards)
{
	for (int32_t i = 0; i < numcards; i++)
		randcards[i].priv = curve25519_keypair(&randcards[i].prod);
}

struct pair256 deckgen_player(bits256 *playerprivs, bits256 *playercards, int32_t *permis, int32_t numcards)
{
	int32_t i;
	struct pair256 key, randcards[256];
	char hexstr[65];

	key = deckgen_common(randcards, numcards);
	bet_permutation(permis, numcards);
	dlg_info("The player key values");
	dlg_info("priv key::%s", bits256_str(hexstr, key.priv));
	dlg_info("pub key::%s", bits256_str(hexstr, key.prod));

	//dlg_info("The player private key card values");
	for (i = 0; i < numcards; i++) {
		playerprivs[i] = randcards[i].priv; // permis[i]
		playercards[i] = curve25519(playerprivs[i], key.prod);
		//dlg_info("card ::%d::%s",i,bits256_str(hexstr,playercards[i]));
	}
	return (key);
}

int32_t deckgen_vendor(int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards,
			     bits256 *playercards,
			     bits256 deckid) // given playercards[], returns cardprods[] and finalcards[]
{
	int32_t retval = OK;
	static struct pair256 randcards[256];
	static bits256 hash_temp[CARDS_MAXCARDS];
	bits256 hash, xoverz, tmp[256];

	deckgen_common2(randcards, numcards);

	for (int32_t i = 0; i < numcards; i++) {
		xoverz = xoverz_donna(curve25519(randcards[i].priv, playercards[i]));
		vcalc_sha256(0, hash.bytes, xoverz.bytes, sizeof(xoverz));
		hash_temp[i] = hash; // optimization
		tmp[i] = fmul_donna(curve25519_fieldelement(hash), randcards[i].priv);
	}

	for (int32_t i = 0; i < numcards; i++) {
		finalcards[i] = tmp[permis_d[i]]; //sg777 tmp[permis_d[i]]
		g_hash[playerid][i] = hash_temp[permis_d[i]]; // sg777 hash_temp[permis_d[i]]
		cardprods[i] = randcards[i].prod; // same cardprods[] returned for each player
	}
	return retval;
}

struct pair256 p2p_bvv_init(bits256 *keys, struct pair256 b_key, bits256 *blindings, bits256 *blindedcards,
			    bits256 *finalcards, int32_t numcards, int32_t numplayers, int32_t playerid, bits256 deckid)
{
	int32_t i, j, M;
	uint8_t space[8192];
	bits256 cardshares[CARDS_MAXPLAYERS];
	struct enc_share temp;

	for (i = 0; i < numcards; i++) {
		blindings[i] = rand256(1);
		blindedcards[i] = fmul_donna(finalcards[permis_b[i]],
					     blindings[i]); //sg777 fmul_donna(finalcards[i], blindings[i])
	}

	M = (numplayers / 2) + 1;

	gfshare_calc_sharenrs(sharenrs, numplayers, deckid.bytes,
			      sizeof(deckid)); // same for all players for this round

	for (i = 0; i < numcards; i++) {
		gfshare_calc_shares(cardshares[0].bytes, blindings[i].bytes, sizeof(bits256), sizeof(bits256), M,
				    numplayers, sharenrs, space, sizeof(space));
		// create combined allshares
		for (j = 0; j < numplayers; j++) {
			// dlg_info("%s --> ",bits256_str(hexstr,cardshares[j]));
			bet_cipher_create(b_key.priv, keys[j], temp.bytes, cardshares[j].bytes, sizeof(cardshares[j]));
			memcpy(g_shares[numplayers * numcards * playerid + i * numplayers + j].bytes, temp.bytes,
			       sizeof(temp));
		}
	}
	return b_key;
}
