#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H



#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))

// #define HTTPD_RESP_USE_STRLEN   (-1)
#define WIFI_MAX_SCAN_NUM       (20)



void WIFI_Init(void);
void HTTP_Client_Init(void);
void HTTP_Server_Init(void);
#endif