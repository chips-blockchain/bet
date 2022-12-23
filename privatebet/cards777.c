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
#include "common.h"
#include "gfshare.h"
#include "cards777.h"

int32_t bet_permutation(int32_t *permi, int32_t numcards)
{
	uint32_t x;
	int32_t i, nonz, n, pos, desti[CARDS777_MAXCARDS];
	uint8_t mask[CARDS777_MAXCARDS / 8 + 1];
	memset(desti, 0, sizeof(desti));
	for (i = 0; i < numcards; i++)
		desti[i] = i;
	n = numcards;
	i = 0;
	while (n > 0) {
		OS_randombytes((uint8_t *)&x, sizeof(x));
		pos = (x % n);
		// fprintf(stderr,"%d ",pos);
		if (desti[pos] == -1) {
			dlg_error("n.%d unexpected pos.%d\n", n, pos);
			continue;
		}
		permi[i++] = desti[pos];
		desti[pos] = desti[n - 1];
		desti[n - 1] = -1;
		n--;
	}
	memset(mask, 0, sizeof(mask));
	for (i = 0; i < numcards; i++)
		SETBIT(mask, permi[i]);
	for (i = nonz = 0; i < numcards; i++)
		if (GETBIT(mask, i) == 0)
			dlg_error("err.%d ", i), nonz++;
	if (nonz != 0) {
		for (i = 0; i < numcards; i++)
			dlg_info("%d ", desti[i]);
		dlg_info("missing bits.%d\n", nonz);
		return (-nonz);
	} else if ((0)) {
		for (i = 0; i < numcards; i++)
			dlg_info("%d ", permi[i]);
		dlg_info("PERMI.%d\n", numcards);
	}
	return 0;
}

void bet_r_permutation(int32_t *permi, int32_t numcards, int32_t *r_permi)
{
	for (int32_t i = 0; i < numcards; i++) {
		r_permi[permi[i]] = i;
	}
}

int32_t bet_cipher_create(bits256 privkey, bits256 destpub, uint8_t *cipher, uint8_t *data, int32_t datalen)
{
	int32_t msglen;
	uint32_t crc32;
	uint8_t *nonce, *buffer, *ptr;
	msglen = datalen;
	nonce = cipher;
	OS_randombytes(nonce, crypto_box_NONCEBYTES);
	ptr = &cipher[crypto_box_NONCEBYTES];
	buffer = malloc(datalen + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + 256);
	msglen = _SuperNET_cipher(nonce, ptr, data, msglen, destpub, privkey, buffer);
	crc32 = calc_crc32(0, ptr, msglen);
	msglen += crypto_box_NONCEBYTES;
	if ((0)) {
		char str[65];
		fprintf(stderr, "encode into msglen.%d crc32.%u [%llx] %s\n", msglen, crc32, *(long long *)cipher,
			bits256_str(str, curve25519(privkey, destpub)));
	}
	free(buffer);
	return (msglen);
}

uint8_t *bet_decrypt(uint8_t *decoded, int32_t maxsize, bits256 senderpub, bits256 mypriv, uint8_t *ptr,
		     int32_t *recvlenp)
{
	uint8_t *nonce, *cipher, *dest = 0;
	int32_t recvlen, cipherlen;
	recvlen = *recvlenp;
	nonce = ptr;
	cipher = &ptr[crypto_box_NONCEBYTES];
	cipherlen = (recvlen - crypto_box_NONCEBYTES);

	if ((0)) {
		int32_t i;
		char str[65];
		for (i = 0; i < recvlen; i++)
			dlg_info("%02x", ptr[i]);
		dlg_info(" decrypt [%llx] recvlen.%d crc32.%u %s\n", *(long long *)ptr, recvlen,
			 calc_crc32(0, cipher, cipherlen), bits256_str(str, curve25519(mypriv, senderpub)));
	}
	if (cipherlen > 0 && cipherlen <= maxsize) {
		if ((dest = _SuperNET_decipher(nonce, cipher, decoded, cipherlen, senderpub, mypriv)) != 0) {
			recvlen = (cipherlen - crypto_box_ZEROBYTES);
			if ((0)) {
				int32_t i;
				for (i = 0; i < recvlen; i++)
					dlg_info("%02x", dest[i]);
				dlg_info(" decrypted");
			}
		}
	} else
		dlg_info("cipher.%d too big for %d\n", cipherlen, maxsize);
	*recvlenp = recvlen;
	return (dest);
}
