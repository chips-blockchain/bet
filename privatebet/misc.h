#include "bet.h"
int32_t hexstr_to_str(char *input, char *output);
void str_to_hexstr(char *input, char *output);
void delete_file(char *file_name);
int check_url(const char *url);
void float_to_uint32(uint32_t *s, uint32_t *m, uint32_t *e, float number);
void float_to_uint32_s(struct float_num *t, float number);
float uint32_s_to_float(struct float_num t);
void struct_to_byte_arr(const void *object, size_t size, uint8_t *out);
void cJSON_hex(cJSON *argjson, char **hexstr);
cJSON *hex_cJSON(char *hex_data);
cJSON *struct_table_to_cJSON(struct table *t);
