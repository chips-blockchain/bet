#include "bet.h"
int32_t hexstr_to_str(char *input, char *output);
void str_to_hexstr(char *input, char *output);
void delete_file(char *file_name);
int check_url(const char *url);
void float_to_uint32(int *s, int *m, int *e, float number);
void float_to_uint32_s(struct float_num *t, float number);
