#ifndef __DEV_SNTP_H__
#define __DEV_SNTP_H__

#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif
    void dev_sntp_init();
    int dev_sntp_get_time(struct tm *time_buffer);
#ifdef __cplusplus
}
#endif
#endif