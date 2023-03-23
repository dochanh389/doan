#ifndef __HTTP_SERVER_APP_H__
#define __HTTP_SERVER_APP_H__


    typedef void(*http_get_callback_t)(void);
    void start_webserver(void);
    void stop_webserver(void);
    void http_set_callback_bmp180(void *cb);
    void bmp180_response(char *data, int len);
#endif