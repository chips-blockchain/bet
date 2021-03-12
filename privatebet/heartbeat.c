#include "bet.h"
#include "host.h"
#include "heartbeat.h"

#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>

int32_t active_players = 0;
int32_t player_status[CARDS777_MAXPLAYERS] = { 0 };

void bet_dcv_reset_player_status(struct privatebet_info *bet)
{
	for (int32_t i = 0; i < bet->maxplayers; i++) {
		player_status[i] = 0;
	}
}

void bet_dcv_player_disconnection_action(struct privatebet_info *bet)
{
	struct privatebet_vars *vars = dcv_vars;
	int32_t active_players = 0;

	for (int i = 0; i < bet->maxplayers; i++) {
		if (player_status[i] == 0) {
			for (int j = vars->round; j < CARDS777_MAXROUNDS; j++) {
				vars->bet_actions[i][j] = fold;
			}
		} else {
			active_players++;
		}
	}
	if (active_players == 1) {
		bet_evaluate_hand(bet, vars);
	} else if (active_players == 0) {
		printf("%s::%d::Using DRP by running ./bet game solve players can claim their funds back", __FUNCTION__,
		       __LINE__);
	}
}

void bet_dcv_publish_player_active_info(struct privatebet_info *bet)
{
	cJSON *active_info = NULL;
	cJSON *players_status_info = NULL;
	int bytes, active_players = 0;

	active_info = cJSON_CreateObject();
	cJSON_AddStringToObject(active_info, "method", "active_player_info");
	players_status_info = cJSON_CreateArray();

	for (int i = 0; i < bet->maxplayers; i++) {
		cJSON_AddItemToArray(players_status_info, cJSON_CreateNumber(player_status[i]));
		if (player_status[i] == 1) {
			active_players++;
		}
	}
	if (active_players < bet->maxplayers) {
		printf("%s::%d::Some players got disconnected DCV needs to take an action", __FUNCTION__, __LINE__);
		bet_dcv_player_disconnection_action(bet);
	}
	cJSON_AddItemToObject(active_info, "player_status", players_status_info);

	bytes = nn_send(bet->pubsock, cJSON_Print(active_info), strlen(cJSON_Print(active_info)), 0);
	if (bytes < 0) {
		printf("%s::%d::There is a problem in sending the data\n", __FUNCTION__, __LINE__);
	}
}

void bet_dcv_heartbeat_loop(void *_ptr)
{
	cJSON *live_info = NULL;
	int32_t bytes;
	struct privatebet_info *bet = _ptr;

	live_info = cJSON_CreateObject();
	cJSON_AddStringToObject(live_info, "method", "is_player_active");

	while (1) {
		if (heartbeat_on == 1) {
			bet_dcv_reset_player_status(bet);
			bytes = nn_send(bet->pubsock, cJSON_Print(live_info), strlen(cJSON_Print(live_info)), 0);
			if (bytes < 0) {
				printf("%s::%d::Error in sending the data\n", __FUNCTION__, __LINE__);
			}
			sleep(5);
			bet_dcv_publish_player_active_info(bet);
		}
	}
}

void bet_dcv_update_player_status(cJSON *argjson)
{
	int32_t playerid;

	playerid = jint(argjson, "playerid");
	player_status[playerid] = 1;
}

void bet_dcv_heartbeat_thread(struct privatebet_info *bet)
{
	pthread_t live_thrd;

	if (OS_thread_create(&live_thrd, NULL, (void *)bet_dcv_heartbeat_loop, (void *)bet) != 0) {
		printf("error launching bet_dcv_heartbeat_loop\n");
		exit(-1);
	}

	if (pthread_join(live_thrd, NULL)) {
		printf("\nError in joining the main thread for bet_dcv_heartbeat_loop");
	}
}
