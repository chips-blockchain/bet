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
#include "table.h"
#include "bet.h"
#include "common.h"
#include "network.h"

void bet_info_set(struct privatebet_info *bet, char *game, int32_t range,
		  int32_t numrounds, int32_t maxplayers)
{
	safecopy(bet->game, game, sizeof(bet->game));
	bet->range = range;
	bet->numrounds = numrounds;
	bet->maxplayers = maxplayers;
}
