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

/*
 * gfshare functions in this file is Copyright Daniel Silverstone
 * <dsilvers@digital-scurf.org>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#include "gfshare.h"
#include "../includes/ppapi/c/pp_stdint.h"
#include "bet.h"
uint8_t BET_logs[256], BET_exps[510];

void libgfshare_init()
{
	uint32_t i, x = 1;
	for (i = 0; i < 255; i++) {
		BET_exps[i] = x;
		BET_logs[x] = i;
		x <<= 1;
		if (x & 0x100)
			x ^= 0x11d; // Unset the 8th bit and mix in 0x1d
	}
	for (i = 255; i < 510; i++)
		BET_exps[i] = BET_exps[i % 255];
	BET_logs[0] = 0; // can't log(0) so just set it neatly to 0
}

struct gfshare_ctx_bet *_gfshare_init_core(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					   void *space, int32_t spacesize)
{
	struct gfshare_ctx_bet *ctx;
	int32_t allocsize;
	allocsize = (int32_t)(sizeof(struct gfshare_ctx_bet) + threshold * size);
	if (allocsize > spacesize) {
		dlg_info("malloc allocsize %d vs spacesize.%d\n", allocsize, spacesize);
		ctx = malloc(allocsize);
		if (ctx == NULL)
			return NULL; // errno should still be set from XMALLOC()
		ctx->allocsize = allocsize;
	} else
		ctx = space;
	memset(ctx, 0, allocsize);
	ctx->sharecount = sharecount;
	ctx->threshold = threshold;
	ctx->size = size;
	memcpy(ctx->sharenrs, sharenrs, sharecount);
	ctx->buffersize = threshold * size;
	return (ctx);
}

// Initialise a gfshare context for producing shares
struct gfshare_ctx_bet *gfshare_initenc(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					void *space, int32_t spacesize)
{
	uint32_t i;
	for (i = 0; i < sharecount; i++) {
		if (sharenrs[i] == 0) {
			// can't have x[i] = 0 - that would just be a copy of the secret, in
			// theory (in fact, due to the way we use exp/log for multiplication
			// and treat log(0) as 0, it ends up as a copy of x[i] = 1)
			errno = EINVAL;
			return NULL;
		}
	}
	return (_gfshare_init_core(sharenrs, sharecount, threshold, size, space, spacesize));
}

// Initialise a gfshare context for recombining shares
struct gfshare_ctx_bet *gfshare_initdec(uint8_t *sharenrs, uint32_t sharecount, uint32_t size, void *space,
					int32_t spacesize)
{
	struct gfshare_ctx_bet *ctx = _gfshare_init_core(sharenrs, sharecount, sharecount, size, space, spacesize);
	if (ctx != NULL)
		ctx->threshold = 0;
	return (ctx);
}

// Initialise a gfshare context for recombining shares
struct gfshare_ctx_bet *gfshare_sg777_initdec(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					      void *space, int32_t spacesize)
{
	struct gfshare_ctx_bet *ctx = _gfshare_init_core(sharenrs, sharecount, threshold, size, space, spacesize);
	if (ctx != NULL)
		ctx->threshold = 0;
	return (ctx);
}

// Free a share context's memory
void gfshare_free(struct gfshare_ctx_bet *ctx)
{
	OS_randombytes(ctx->buffer, ctx->buffersize);
	OS_randombytes(ctx->sharenrs, ctx->sharecount);
	if (ctx->allocsize != 0) {
		OS_randombytes((uint8_t *)ctx, sizeof(struct gfshare_ctx_bet));
		free(ctx);
	} else
		OS_randombytes((uint8_t *)ctx, sizeof(struct gfshare_ctx_bet));
}

// --------------------------------------------------------[ Splitting ]----

// Provide a secret to the encoder. (this re-scrambles the coefficients)
void gfshare_enc_setsecret(struct gfshare_ctx_bet *ctx, uint8_t *secret)
{
	memcpy(ctx->buffer + ((ctx->threshold - 1) * ctx->size), secret, ctx->size);
	OS_randombytes(ctx->buffer, (ctx->threshold - 1) * ctx->size);
}

// Extract a share from the context. 'share' must be preallocated and at least
// 'size' bytes long. 'sharenr' is the index into the 'sharenrs' array of the
// share you want.
void gfshare_encgetshare(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)
{
	if (_logs == 0)
		_logs = BET_logs;
	if (_exps == 0)
		_exps = BET_exps;

	uint32_t pos, coefficient, ilog = _logs[ctx->sharenrs[sharenr]];
	uint8_t *share_ptr, *coefficient_ptr = ctx->buffer;

	for (pos = 0; pos < ctx->size; pos++)
		share[pos] = *(coefficient_ptr++);
	for (coefficient = 1; coefficient < ctx->threshold; coefficient++) {
		share_ptr = share;
		for (pos = 0; pos < ctx->size; pos++) {
			uint8_t share_byte = *share_ptr;
			if (share_byte != 0)
				share_byte = _exps[ilog + _logs[share_byte]];
			*share_ptr++ = share_byte ^ *coefficient_ptr++;
		}
	}
}

// ----------------------------------------------------[ Recombination ]----

// Inform a recombination context of a change in share indexes
void gfshare_dec_newshares(struct gfshare_ctx_bet *ctx, uint8_t *sharenrs)
{
	memcpy(ctx->sharenrs, sharenrs, ctx->sharecount);
}

// Provide a share context with one of the shares. The 'sharenr' is the index
// into the 'sharenrs' array
void gfshare_dec_giveshare(struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)
{
	memcpy(ctx->buffer + (sharenr * ctx->size), share, ctx->size);
}

// Extract the secret by interpolating the shares. secretbuf must be allocated
// and at least 'size' bytes
void gfshare_decextract(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t *secretbuf)
{
	uint32_t i, j;
	uint8_t *secret_ptr, *share_ptr, sharei, sharej;
	if (_logs == 0)
		_logs = BET_logs;
	if (_exps == 0)
		_exps = BET_exps;
	for (i = 0; i < ctx->size; i++)
		secretbuf[i] = 0;
	for (i = 0; i < ctx->sharecount; i++) {
		// Compute L(i) as per Lagrange Interpolation
		unsigned Li_top = 0, Li_bottom = 0;
		if ((sharei = ctx->sharenrs[i]) != 0) {
			for (j = 0; j < ctx->sharecount; j++) {
				if (i != j && sharei != (sharej = ctx->sharenrs[j])) {
					if (sharej == 0)
						continue; // skip empty share
					Li_top += _logs[sharej];
					if (Li_top >= 0xff)
						Li_top -= 0xff;
					Li_bottom += _logs[sharei ^ sharej];
					if (Li_bottom >= 0xff)
						Li_bottom -= 0xff;
				}
			}
			if (Li_bottom > Li_top)
				Li_top += 0xff;
			Li_top -= Li_bottom; // Li_top is now log(L(i))
			secret_ptr = secretbuf, share_ptr = ctx->buffer + (ctx->size * i);
			for (j = 0; j < ctx->size; j++) {
				if (*share_ptr != 0)
					*secret_ptr ^= _exps[Li_top + _logs[*share_ptr]];
				share_ptr++, secret_ptr++;
			}
		}
	}
}

int32_t gfshare_calc_sharenrs(uint8_t *sharenrs, int32_t N, uint8_t *data, int32_t datasize)
{
	bits256 hash, hash2;
	uint8_t r;
	int32_t i, j, n = sizeof(hash);
	vcalc_sha256(0, hash.bytes, data, datasize);
	vcalc_sha256(0, hash2.bytes, hash.bytes, sizeof(hash));
	for (i = 0; i < N; i++) {
		while (1) {
			if (n >= sizeof(hash)) {
				vcalc_sha256(0, hash.bytes, hash2.bytes, sizeof(hash2));
				vcalc_sha256(0, hash2.bytes, hash.bytes, sizeof(hash));
				n = 0;
			}
			r = hash2.bytes[n++];
			if ((sharenrs[i] = r) == 0 || sharenrs[i] == 0xff)
				continue;
			for (j = 0; j < i; j++)
				if (sharenrs[j] == sharenrs[i])
					break;
			if (j == i)
				break;
		}
		// dlg_info("%3d ",sharenrs[i]);
	}
	return (N);
}

int32_t gfshare_init_sharenrs(uint8_t sharenrs[255], uint8_t *orig, int32_t m, int32_t n)
{
	uint8_t *randvals, valid[255];
	int32_t i, j, r, remains, orign;
	if (m > n || n >= 0xff) // reserve 255 for illegal sharei
	{
		dlg_info("illegal M.%d of N.%d\n", m, n);
		return (-1);
	}
	randvals = calloc(1, 65536);
	OS_randombytes(randvals, 65536);
	if (orig == 0 && n == m) {
		memset(sharenrs, 0, n);
		for (i = 0; i < 255; i++)
			valid[i] = (i + 1);
		remains = orign = 255;
		for (i = 0; i < n; i++) {
			r = (randvals[i] % remains);
			sharenrs[i] = valid[r];
			dlg_info("%d ", sharenrs[i]);
			valid[r] = valid[--remains];
		}
		dlg_info("FULL SET");
	} else {
		memcpy(valid, orig, n);
		memset(sharenrs, 0, n);
		for (i = 0; i < n; i++)
			dlg_info("%d ", valid[i]);
		dlg_info("valid");
		for (i = 0; i < m; i++) {
			r = rand() % n;
			while ((j = valid[r]) == 0) {
				// dlg_info("i.%d j.%d m.%d n.%d r.%d\n",i,j,m,n,r);
				r = rand() % n;
			}
			sharenrs[i] = j;
			valid[r] = 0;
		}
		for (i = 0; i < n; i++)
			dlg_info("%d ", valid[i]);
		dlg_info("valid");
		for (i = 0; i < m; i++)
			dlg_info("%d ", sharenrs[i]);
		dlg_info("sharenrs vals m.%d of n.%d\n", m, n);
		// getchar();
	}
	free(randvals);
	for (i = 0; i < m; i++) {
		for (j = 0; j < m; j++) {
			if (i == j)
				continue;
			if (sharenrs[i] != 0 && sharenrs[i] == sharenrs[j]) {
				dlg_info("FATAL: duplicate entry sharenrs[%d] %d vs %d "
					 "sharenrs[%d]\n",
					 i, sharenrs[i], sharenrs[j], j);
				return (-1);
			}
		}
	}
	return (0);
}

uint8_t *gfshare_recoverdata(uint8_t *shares[], uint8_t *sharenrs, int32_t M, uint8_t *recover, int32_t datasize,
			     int32_t N)
{
	void *G;
	int32_t i, m = 0;
	uint8_t recovernrs[255], space[8192];
	memset(recovernrs, 0, sizeof(recovernrs));
	for (i = 0; i < N; i++)
		if (shares[i] != 0)
			recovernrs[i] = sharenrs[i], m++;
	if (m >= M) {
		G = gfshare_initdec(recovernrs, N, datasize, space, sizeof(space));
		for (i = 0; i < N; i++)
			if (shares[i] != 0)
				gfshare_dec_giveshare(G, i, shares[i]);
		gfshare_dec_newshares(G, recovernrs);
		gfshare_decextract(BET_logs, BET_exps, G, recover);
		gfshare_free(G);
		return (recover);
	} else
		return (0);
}

void gfshare_calc_share(uint8_t *buffer, int32_t size, int32_t M, uint32_t ilog, uint8_t *share)
{
	uint32_t pos, coefficient;
	uint8_t *share_ptr, share_byte;
	for (pos = 0; pos < size; pos++)
		share[pos] = *(buffer++);
	for (coefficient = 1; coefficient < M; coefficient++) {
		share_ptr = share;
		for (pos = 0; pos < size; pos++) {
			share_byte = *share_ptr;
			if (share_byte != 0)
				share_byte = BET_exps[ilog + BET_logs[share_byte]];
			*share_ptr++ = (share_byte ^ *buffer++);
		}
	}
}

void gfshare_calc_shares(uint8_t *shares, uint8_t *secret, int32_t size, int32_t width, int32_t M, int32_t N,
			 uint8_t *sharenrs, uint8_t *space, int32_t spacesize)
{
	int32_t i;
	uint8_t *buffer;
	// M threshold
	if (M * width > spacesize) {
		buffer = calloc(M, width);
		dlg_info("calloc M.%d width.%d size.%d\n", M, width, N * width);
	} else
		buffer = space;
	memset(shares, 0, N * width);
	memcpy(buffer + ((M - 1) * size), secret, size);
	OS_randombytes(buffer, (M - 1) * size);
	for (i = 0; i < N; i++) {
		// fprintf(stderr,"%d of %d: ",i,N);
		gfshare_calc_share(buffer, size, M, BET_logs[sharenrs[i]], &shares[i * width]);
		// fprintf(stderr,"(%03d %08x)\n",sharenrs[i],calc_crc32(0,&shares[i *
		// width],size));
	}
	if (buffer != space)
		free(buffer);
}
