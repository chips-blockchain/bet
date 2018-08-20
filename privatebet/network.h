cJSON *BET_pubkeys(struct privatebet_info *bet);
void BET_message_send(char *debugstr,int32_t sock,cJSON *msgjson,int32_t freeflag,struct privatebet_info *bet);
void BET_shardkey(uint8_t *key,bits256 deckid,int32_t cardi,int32_t playerj);
struct BET_shardsinfo *BET_shardsfind(bits256 deckid,int32_t cardi,int32_t playerj);
void BET_MofN_item(bits256 deckid,int32_t cardi,bits256 *cardpubs,int32_t numcards,int32_t playerj,int32_t numplayers,bits256 shard,int32_t shardi);
bits256 *BET_process_packet(bits256 *cardpubs,bits256 *deckidp,bits256 senderpub,bits256 mypriv,uint8_t *decoded,int32_t maxsize,bits256 mypub,uint8_t *sendbuf,int32_t size,int32_t checkplayers,int32_t range);
void BET_broadcast(int32_t pubsock,uint8_t *decoded,int32_t maxsize,bits256 *playerprivs,bits256 *playerpubs,int32_t numplayers,int32_t numcards,uint8_t *sendbuf,int32_t size,bits256 deckid);
void BET_roundstart(int32_t pubsock,cJSON *deckjson,int32_t numcards,bits256 *privkeys,bits256 *playerpubs,int32_t numplayers,bits256 privkey);
char *BET_transportname(int32_t bindflag,char *str,char *ipaddr,uint16_t port);
int32_t BET_nanosock(int32_t bindflag,char *endpoint,int32_t nntype);
void BET_mofn_send(struct privatebet_info *bet,struct privatebet_vars *vars,int32_t cardi,int32_t playerj,int32_t encryptflag);
int32_t BET_client_deali(cJSON *argjson,struct privatebet_info *bet,struct privatebet_vars *vars,int32_t senderid);

