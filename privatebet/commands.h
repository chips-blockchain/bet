char *chipsln_command(void *ctx,cJSON *argjson,char *remoteaddr,uint16_t port);
cJSON *chipsln_issue(char *buf);
cJSON *chipsln_noargs(char *method);
cJSON *chipsln_strarg(char *method,char *str);
cJSON *chipsln_strnum(char *method,char *str,uint64_t num);
cJSON *chipsln_numstr(char *method,uint64_t num,char *str);
cJSON *chipsln_getinfo();
cJSON *chipsln_help();
cJSON *chipsln_stop();
cJSON *chipsln_newaddr();
cJSON *chipsln_getnodes();
cJSON *chipsln_getpeers();
cJSON *chipsln_getchannels();
cJSON *chipsln_devblockheight();
cJSON *chipsln_delinvoice(char *label);
cJSON *chipsln_delpaidinvoice(char *label);
cJSON *chipsln_waitanyinvoice(char *label);
cJSON *chipsln_waitinvoice(char *label);
cJSON *chipsln_getlog(char *level);
cJSON *chipsln_close(char *idstr);
cJSON *chipsln_devrhash(char *secret);
cJSON *chipsln_addfunds(char *rawtx);
cJSON *chipsln_fundchannel(char *idstr,uint64_t satoshi);
cJSON *chipsln_listinvoice(char *label);
cJSON *chipsln_invoice(uint64_t msatoshi,char *label);
bits256 chipsln_rhash_create(uint64_t satoshis,char *label);
cJSON *chipsln_withdraw(uint64_t satoshi,char *address);
cJSON *chipsln_getroute(char *idstr,uint64_t msatoshi);
cJSON *chipsln_connect(char *ipaddr,uint16_t port,char *destid);
cJSON *chipsln_sendpay(cJSON *routejson,bits256 rhash);
char *privatebet_command(void *ctx,cJSON *argjson,char *remoteaddr,uint16_t port);
char *pangea_command(void *ctx,cJSON *argjson,char *remoteaddr,uint16_t port);
char *stats_JSON(void *ctx,char *myipaddr,int32_t mypubsock,cJSON *argjson,char *remoteaddr,uint16_t port);
int32_t BET_peer_state(char *peerid,char *statestr);
int32_t BET_channel_status(char *peerid,char *channel,char *status);
int64_t BET_peer_chipsavail(char *peerid,int32_t chipsize);

int32_t BET_get_chips_blockheight();
int32_t BET_get_ln_blockheight();
void BET_check_sync();
void BET_check_sync();
void BET_listunspent();
double BET_getbalance();
int32_t BET_lock_transaction(int32_t fundAmount);



