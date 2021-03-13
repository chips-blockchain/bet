#pragma once

typedef int32_t b_status_t;

typedef enum {
     B_SUCCESS = 0x1,
     B_ERR = 0x2,
     B_CONNECT_ERR = 0x3,
     B_SEND_ERR = 0x4,
     B_RCV_ERR = 0x5
} b_error_e;
