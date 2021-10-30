#include "misc.h"
#include "../includes/curl/curl.h"
#include "../includes/curl/easy.h"

int32_t hexstr_to_str(char *input, char *output)
{
	char code[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	int32_t loop, flag;
	int i;

	i = 0;
	loop = 0;
	int num = 0;
	while (input[loop] != '\0') {
		flag = 0;
		for (int32_t j = 0; j < 16; j++) {
			if (code[j] == input[loop]) {
				if ((loop % 2) == 0)
					num = j * 16;
				else {
					num = num + j;
					sprintf((char *)(output + i), "%c", num);
					i++;
					num = 0;
				}
				flag = 1;
				break;
			}
		}
		loop++;
		if (flag == 0)
			break;
	}
	output[i++] = '\0';
	return flag;
}

void str_to_hexstr(char *input, char *output)
{
	int loop;
	int i;

	i = 0;
	loop = 0;

	while (input[loop] != '\0') {
		sprintf((char *)(output + i), "%02x", input[loop]);
		loop++;
		i += 2;
	}
	output[i++] = '\0';
}

void delete_file(char *file_name)
{
	if (remove(file_name) != 0) {
		dlg_warn("Intermediate file %s is not removed", file_name);
	}
}

int check_url(const char *url)
{
	CURL *curl = NULL;
	CURLcode response;

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		response = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	return (response == CURLE_OK) ? 1 : 0;
}
