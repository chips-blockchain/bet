#include "bet.h"
#include "common.h"

//Keys under a given namespace
#define iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP "chips.vrsc::bet.cashiers"
#define CASHIERS_KEY "iJ3WZocnjG9ufv7GKUA4LijQno5gTMb7tP"

//Datatypes used
#define iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c "vrsc::data.type.string"
#define STRING_VDXF_ID "iK7a5JNJnbeuYWVHCDRpJosj3irGJ5Qa8c" 

//Identitites
#define CASHIERS_ID "cashiers.cashiers.chips10sec@"

#define CASHIERS_CASHIERS_VDXF_ID "i6CS9ewyp4oWozG2eceXPk3uSHg3dihdPg"


cJSON* get_cashiers();
cJSON* get_onchain_cashiers();
void update_cashiers();
