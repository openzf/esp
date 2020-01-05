#ifndef __SYSTEM_MODULE_H__
#define __SYSTEM_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

void system_module_init(void (*arg_system_ready_fun)(void));

#ifdef __cplusplus
}
#endif
#endif