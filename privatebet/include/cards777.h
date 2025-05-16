int32_t bet_permutation(int32_t *permi, int32_t numcards);
void bet_r_permutation(int32_t *permi, int32_t numcards, int32_t *r_permi);
int32_t bet_cipher_create(bits256 privkey, bits256 destpub, uint8_t *cipher, uint8_t *data, int32_t datalen);
uint8_t *bet_decrypt(uint8_t *decoded, int32_t maxsize, bits256 senderpub, bits256 mypriv, uint8_t *ptr,
		     int32_t *recvlenp);
