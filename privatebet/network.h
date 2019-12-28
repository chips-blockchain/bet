#include "bet.h"
char *BET_transportname(int32_t bindflag, char *str, char *ipaddr,
                        uint16_t port);
int32_t BET_nanosock(int32_t bindflag, char *endpoint, int32_t nntype);
