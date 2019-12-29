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
#include "payment.h"
#include "bet.h"
#include "client.h"
#include "commands.h"
#include "common.h"
#include "network.h"
bits256 Host_rhashes[256];
bits256 Clientrhash;

/***************************************************************
Here contains the functions which are specific to DCV
****************************************************************/
int32_t BET_DCV_create_invoice_request(struct privatebet_info *bet,
                                       int32_t playerid, int32_t amount) {
    int32_t retval = 1, bytes;
    cJSON *betInfo = NULL;
    char *rendered = NULL;

    betInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(betInfo, "method", "invoiceRequest_player");
    cJSON_AddNumberToObject(betInfo, "playerID", playerid);
    cJSON_AddNumberToObject(betInfo, "betAmount", amount);

    rendered = cJSON_Print(betInfo);

    bytes = nn_send(bet->pubsock, rendered, strlen(rendered), 0);

    if (bytes < 0) {
        retval = -1;
        printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
        goto end;
    }

end:
    return retval;
}

int32_t BET_DCV_invoice_pay(struct privatebet_info *bet,
                            struct privatebet_vars *vars, int playerid,
                            int amount) {
    pthread_t pay_t;
    int32_t retval = 1;
    printf("%s::%d\n", __FUNCTION__, __LINE__);
    retval = BET_DCV_create_invoice_request(bet, playerid, amount);
    if (OS_thread_create(&pay_t, NULL, (void *)BET_DCV_paymentloop,
                         (void *)bet) != 0) {
        // exit(-1);
        retval = -1;
        printf("%s::%d::Invoice payment is failed\n", __FUNCTION__, __LINE__);
    }
    if (pthread_join(pay_t, NULL)) {
        printf("\nError in joining the main thread for player %d",
               bet->myplayerid);
        retval = -1;
    }

    return retval;
}

int32_t BET_DCV_pay(cJSON *argjson, struct privatebet_info *bet,
                    struct privatebet_vars *vars) {

    cJSON *invoiceInfo = NULL, *payResponse = NULL;
    int argc, retval = 1;
    char **argv = NULL, *invoice = NULL;

    printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));

    argv = (char **)malloc(4 * sizeof(char *));
    argc = 3;
    for (int i = 0; i < 4; i++) {
        argv[i] = (char *)malloc(sizeof(char) * 1000);
    }
    invoice = jstr(argjson, "invoice");
    invoiceInfo = cJSON_Parse(invoice);

    strcpy(argv[0], "lightning-cli");
    strcpy(argv[1], "pay");
    sprintf(argv[2], "%s", jstr(invoiceInfo, "bolt11"));
    argv[3] = NULL;

    payResponse = cJSON_CreateObject();
    make_command(argc, argv, &payResponse);

    if (jint(payResponse, "code") != 0) {
        retval = -1;
        printf("\n%s:%d: Message:%s", __FUNCTION__, __LINE__,
               jstr(payResponse, "message"));
        goto end;
    }

    if (strcmp(jstr(payResponse, "status"), "complete") == 0)
        printf("\nPayment Success");

end:
    if (argv) {
        for (int i = 0; i < 4; i++) {
            if (argv[i])
                free(argv[i]);
        }
        free(argv);
    }

    return retval;
}

void BET_DCV_paymentloop(void *_ptr) {
    int32_t recvlen;
    uint8_t flag = 1;
    void *ptr;
    cJSON *msgjson = NULL;
    char *method = NULL;
    struct privatebet_info *bet = _ptr;
    while (flag) {

        if (bet->subsock >= 0 && bet->pushsock >= 0) {
            recvlen = nn_recv(bet->pullsock, &ptr, NN_MSG, 0);
            if (((msgjson = cJSON_Parse(ptr)) != 0) && (recvlen > 0)) {
                if ((method = jstr(msgjson, "method")) != 0) {
                    if (strcmp(method, "invoice") == 0) {
                        BET_DCV_pay(msgjson, bet, NULL);
                        flag = 0;
                    }
                }

                free_json(msgjson);
            }
        }
    }
}

/***************************************************************
Here contains the functions which are specific to players and BVV
****************************************************************/

int32_t BET_player_create_invoice(cJSON *argjson, struct privatebet_info *bet,
                                  struct privatebet_vars *vars, char *deckid) {
    int argc, bytes, retval = 1;
    char **argv = NULL, *rendered = NULL;
    cJSON *invoiceInfo = NULL, *invoice = NULL;

    printf("%s::%d::%s\n", __FUNCTION__, __LINE__, cJSON_Print(argjson));

    if (jint(argjson, "playerid") == bet->myplayerid) {

        vars->player_funds += jint(argjson, "betAmount");

        argv = (char **)malloc(6 * sizeof(char *));
        for (int i = 0; i < 6; i++) {
            argv[i] = (char *)malloc(sizeof(char) * 1000);
        }

        strcpy(argv[0], "lightning-cli");
        strcpy(argv[1], "invoice");

        sprintf(argv[2], "%ld",
                (long int)jint(argjson, "betAmount") * mchips_msatoshichips);
        sprintf(argv[3], "%s_%d", deckid, jint(argjson, "betAmount"));
        sprintf(argv[4], "\"Winning claim\"");
        argv[5] = NULL;
        argc = 5;

        invoice = cJSON_CreateObject();
        make_command(argc, argv, &invoice);

        printf("%s::%d::invoice::%s\n", __FUNCTION__, __LINE__,
               cJSON_Print(invoice));

        invoiceInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(invoiceInfo, "method", "invoice");
        cJSON_AddNumberToObject(invoiceInfo, "playerid", bet->myplayerid);
        cJSON_AddStringToObject(invoiceInfo, "label", argv[3]);
        cJSON_AddStringToObject(invoiceInfo, "invoice", cJSON_Print(invoice));

        rendered = cJSON_Print(invoiceInfo);
        bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);
        if (bytes < 0) {
            retval = -1;
            printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
            goto end;
        }
    }
end:

    if (argv) {
        for (int i = 0; i < 5; i++) {
            if (argv[i])
                free(argv[i]);
        }
        free(argv);
    }
    return retval;
}

int32_t BET_player_create_invoice_request(cJSON *argjson,
                                          struct privatebet_info *bet,
                                          int32_t amount) {
    int32_t retval = 1, bytes;
    cJSON *betInfo = NULL;
    char *rendered = NULL;

    betInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(betInfo, "method", "invoiceRequest");
    cJSON_AddNumberToObject(betInfo, "round", jint(argjson, "round"));
    cJSON_AddNumberToObject(betInfo, "playerID", bet->myplayerid);
    cJSON_AddNumberToObject(betInfo, "betAmount", amount);

    rendered = cJSON_Print(betInfo);

    bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

    if (bytes < 0) {
        retval = -1;
        printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
        goto end;
    }

end:
    return retval;
}

int32_t BET_player_create_betting_invoice_request(cJSON *argjson,
                                                  cJSON *actionResponse,
                                                  struct privatebet_info *bet,
                                                  int32_t amount) {
    int32_t retval = 1, bytes;
    cJSON *betInfo = NULL;
    char *rendered = NULL;

    betInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(betInfo, "method", "bettingInvoiceRequest");
    cJSON_AddNumberToObject(betInfo, "round", jint(argjson, "round"));
    cJSON_AddNumberToObject(betInfo, "playerID", bet->myplayerid);
    cJSON_AddNumberToObject(betInfo, "invoice_amount", amount);
    cJSON_AddItemToObject(betInfo, "actionResponse", actionResponse);

    rendered = cJSON_Print(betInfo);

    bytes = nn_send(bet->pushsock, rendered, strlen(rendered), 0);

    if (bytes < 0) {
        retval = -1;
        printf("\n%s:%d: Failed to send data", __FUNCTION__, __LINE__);
        goto end;
    }

end:
    return retval;
}

int32_t BET_player_invoice_pay(cJSON *argjson, struct privatebet_info *bet,
                               struct privatebet_vars *vars, int amount) {
    pthread_t pay_t;
    int32_t retval = 1;

    retval = BET_player_create_invoice_request(argjson, bet, amount);
    if (OS_thread_create(&pay_t, NULL, (void *)BET_player_paymentloop,
                         (void *)bet) != 0) {
        printf("%s::%d::%d\n", __FUNCTION__, __LINE__, retval);
    }
    if (pthread_join(pay_t, NULL)) {
        printf("\nError in joining the main thread for player %d",
               bet->myplayerid);
        retval = -1;
    }
    return retval;
}

void BET_player_paymentloop(void *_ptr) {
    int32_t recvlen, retval = 1;
    uint8_t flag = 1;
    void *ptr;
    cJSON *msgjson = NULL;
    char *method = NULL;
    struct privatebet_info *bet = _ptr;
    msgjson = cJSON_CreateObject();
    while (flag) {

        if (bet->subsock >= 0 && bet->pushsock >= 0) {
            recvlen = nn_recv(bet->subsock, &ptr, NN_MSG, 0);
            if (((msgjson = cJSON_Parse(ptr)) != 0) && (recvlen > 0)) {
                if ((method = jstr(msgjson, "method")) != 0) {
                    if (strcmp(method, "invoice") == 0) {
                        retval = BET_p2p_invoice(msgjson, bet, NULL);
                        flag = 0;
                        break;
                    } else {
                        printf("%s::%d::%s\n", __FUNCTION__, __LINE__,
                               cJSON_Print(msgjson));
                    }
                }
            }
        }
    }
}

/*
Below are the payment related REST API's
*/

int32_t BET_rest_player_create_invoice_request_round(struct lws *wsi,
                                                     cJSON *argjson,
                                                     int32_t amount,
                                                     int32_t option) {
    int32_t retval = 1;
    cJSON *betInfo = NULL;

    betInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(betInfo, "method", "invoiceRequest");
    cJSON_AddNumberToObject(betInfo, "round", jint(argjson, "round"));
    cJSON_AddNumberToObject(
        betInfo, "playerID",
        BET_player[jint(argjson, "gui_playerID")]->myplayerid);
    cJSON_AddNumberToObject(betInfo, "betAmount", amount);
    cJSON_AddNumberToObject(betInfo, "option", option);
    cJSON_AddItemToObject(betInfo, "payment_params", argjson);
    lws_write(wsi, cJSON_Print(betInfo), strlen(cJSON_Print(betInfo)), 0);

    return retval;
}

int32_t BET_rest_player_create_invoice_request(struct lws *wsi, cJSON *argjson,
                                               int32_t amount) {
    int32_t retval = 1;
    cJSON *betInfo = NULL;

    betInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(betInfo, "method", "invoiceRequest");
    cJSON_AddNumberToObject(betInfo, "round", jint(argjson, "round"));
    cJSON_AddNumberToObject(
        betInfo, "playerID",
        BET_player[jint(argjson, "gui_playerID")]->myplayerid);
    cJSON_AddNumberToObject(betInfo, "betAmount", amount);
    cJSON_AddItemToObject(betInfo, "payment_params", argjson);
    lws_write(wsi, cJSON_Print(betInfo), strlen(cJSON_Print(betInfo)), 0);

    return retval;
}

int32_t BET_rest_DCV_create_invoice_request(struct lws *wsi, int32_t amount,
                                            int32_t playerID) {
    int32_t retval = 1;

    cJSON *invoiceRequestInfo = NULL;

    invoiceRequestInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(invoiceRequestInfo, "method",
                            "winningInvoiceRequest");
    cJSON_AddNumberToObject(invoiceRequestInfo, "playerID", playerID);
    cJSON_AddNumberToObject(invoiceRequestInfo, "winningAmount", amount);

    lws_write(wsi, cJSON_Print(invoiceRequestInfo),
              strlen(cJSON_Print(invoiceRequestInfo)), 0);

    return retval;
}
