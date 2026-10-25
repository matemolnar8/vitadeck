#ifndef __vita__

#include "thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

struct vd_thread {
    pthread_t handle;
};

struct vd_mutex {
    pthread_mutex_t handle;
};

vd_thread *vd_thread_create(void *(*func)(void *), void *arg)
{
    vd_thread *t = malloc(sizeof(vd_thread));
    if (!t) return NULL;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        free(t);
        return NULL;
    }
    int rc = pthread_attr_setstacksize(&attr, VD_THREAD_STACK_SIZE);
    if (rc == 0) rc = pthread_create(&t->handle, &attr, func, arg);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        free(t);
        return NULL;
    }
    return t;
}

void vd_thread_join(vd_thread *t)
{
    if (t) pthread_join(t->handle, NULL);
}

void vd_thread_destroy(vd_thread *t)
{
    free(t);
}

void vd_thread_yield(void)
{
    sched_yield();
}

void vd_sleep_ms(int ms)
{
    struct timespec ts = {.tv_sec = ms / 1000, .tv_nsec = (long)(ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

vd_mutex *vd_mutex_create(void)
{
    vd_mutex *m = malloc(sizeof(vd_mutex));
    if (!m) return NULL;
    if (pthread_mutex_init(&m->handle, NULL) != 0) {
        free(m);
        return NULL;
    }
    return m;
}

void vd_mutex_lock(vd_mutex *m)
{
    if (m) pthread_mutex_lock(&m->handle);
}

void vd_mutex_unlock(vd_mutex *m)
{
    if (m) pthread_mutex_unlock(&m->handle);
}

void vd_mutex_destroy(vd_mutex *m)
{
    if (m) {
        pthread_mutex_destroy(&m->handle);
        free(m);
    }
}

#endif /* !__vita__ */
