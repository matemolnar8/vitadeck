#ifndef VD_THREAD_H
#define VD_THREAD_H

typedef struct vd_thread vd_thread;
typedef struct vd_mutex vd_mutex;

// Thread
vd_thread* vd_thread_create(void* (*func)(void*), void* arg);
void vd_thread_join(vd_thread* t);
void vd_thread_destroy(vd_thread* t);
void vd_thread_yield(void);

// Mutex
vd_mutex* vd_mutex_create(void);
void vd_mutex_lock(vd_mutex* m);
void vd_mutex_unlock(vd_mutex* m);
void vd_mutex_destroy(vd_mutex* m);

#endif /* VD_THREAD_H */
