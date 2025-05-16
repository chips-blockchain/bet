#include "bet.h"
void libgfshare_init();
struct gfshare_ctx_bet *_gfshare_init_core(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					   void *space, int32_t spacesize);
struct gfshare_ctx_bet *gfshare_initenc(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					void *space, int32_t spacesize);
struct gfshare_ctx_bet *gfshare_initdec(uint8_t *sharenrs, uint32_t sharecount, uint32_t size, void *space,
					int32_t spacesize);
struct gfshare_ctx_bet *gfshare_sg777_initdec(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size,
					      void *space, int32_t spacesize);
void gfshare_free(struct gfshare_ctx_bet *ctx);
void gfshare_enc_setsecret(struct gfshare_ctx_bet *ctx, uint8_t *secret);
void gfshare_encgetshare(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share);
void gfshare_dec_newshares(struct gfshare_ctx_bet *ctx, uint8_t *sharenrs);
void gfshare_dec_giveshare(struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share);
void gfshare_decextract(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t *secretbuf);
int32_t gfshare_calc_sharenrs(uint8_t *sharenrs, int32_t N, uint8_t *data, int32_t datasize);
int32_t gfshare_init_sharenrs(uint8_t sharenrs[255], uint8_t *orig, int32_t m, int32_t n);
uint8_t *gfshare_recoverdata(uint8_t *shares[], uint8_t *sharenrs, int32_t M, uint8_t *recover, int32_t datasize,
			     int32_t N);
void gfshare_calc_share(uint8_t *buffer, int32_t size, int32_t M, uint32_t ilog, uint8_t *share);
void gfshare_calc_shares(uint8_t *shares, uint8_t *secret, int32_t size, int32_t width, int32_t M, int32_t N,
			 uint8_t *sharenrs, uint8_t *space, int32_t spacesize);
