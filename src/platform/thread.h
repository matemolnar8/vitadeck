#ifndef VD_THREAD_H
#define VD_THREAD_H

// Stack of every vd_thread, the JS thread included; QuickJS's stack limit is derived from it.
// Overridable so a host harness can run on the Vita stack size.
#define VD_VITA_THREAD_STACK_SIZE (256 * 1024)
#ifndef VD_THREAD_STACK_SIZE
#ifdef __vita__
#define VD_THREAD_STACK_SIZE VD_VITA_THREAD_STACK_SIZE
#else
#define VD_THREAD_STACK_SIZE (1024 * 1024)
#endif
#endif

typedef struct vd_thread vd_thread;
typedef struct vd_mutex vd_mutex;

// Thread
vd_thread *vd_thread_create(void *(*func)(void *), void *arg);
void vd_thread_join(vd_thread *t);
void vd_thread_destroy(vd_thread *t);
void vd_thread_yield(void);
void vd_sleep_ms(int ms);

// Mutex
vd_mutex *vd_mutex_create(void);
void vd_mutex_lock(vd_mutex *m);
void vd_mutex_unlock(vd_mutex *m);
void vd_mutex_destroy(vd_mutex *m);

#endif /* VD_THREAD_H */
