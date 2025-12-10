#ifdef __vita__

#include "thread.h"
#include <psp2/kernel/threadmgr.h>
#include <stdlib.h>

#define JS_THREAD_STACK_SIZE (256 * 1024)

struct vd_thread {
    SceUID tid;
    void* (*func)(void*);
    void* arg;
};

struct vd_mutex {
    SceUID mid;
};

static int vita_thread_entry(SceSize args, void* argp) {
    vd_thread* t = *(vd_thread**)argp;
    t->func(t->arg);
    return 0;
}

vd_thread* vd_thread_create(void* (*func)(void*), void* arg) {
    vd_thread* t = malloc(sizeof(vd_thread));
    if (!t) return NULL;

    t->func = func;
    t->arg = arg;

    t->tid = sceKernelCreateThread(
        "VdThread",
        vita_thread_entry,
        0x10000100,
        JS_THREAD_STACK_SIZE,
        0,
        0,
        NULL
    );

    if (t->tid < 0) {
        free(t);
        return NULL;
    }

    vd_thread* t_ptr = t;
    if (sceKernelStartThread(t->tid, sizeof(vd_thread*), &t_ptr) < 0) {
        sceKernelDeleteThread(t->tid);
        free(t);
        return NULL;
    }

    return t;
}

void vd_thread_join(vd_thread* t) {
    if (t && t->tid >= 0) {
        sceKernelWaitThreadEnd(t->tid, NULL, NULL);
    }
}

void vd_thread_destroy(vd_thread* t) {
    if (t) {
        if (t->tid >= 0) {
            sceKernelDeleteThread(t->tid);
        }
        free(t);
    }
}

void vd_thread_yield(void) {
    sceKernelDelayThread(1000);
}

vd_mutex* vd_mutex_create(void) {
    vd_mutex* m = malloc(sizeof(vd_mutex));
    if (!m) return NULL;

    m->mid = sceKernelCreateMutex("VdMutex", 0, 0, NULL);
    if (m->mid < 0) {
        free(m);
        return NULL;
    }

    return m;
}

void vd_mutex_lock(vd_mutex* m) {
    if (m && m->mid >= 0) {
        sceKernelLockMutex(m->mid, 1, NULL);
    }
}

void vd_mutex_unlock(vd_mutex* m) {
    if (m && m->mid >= 0) {
        sceKernelUnlockMutex(m->mid, 1);
    }
}

void vd_mutex_destroy(vd_mutex* m) {
    if (m) {
        if (m->mid >= 0) {
            sceKernelDeleteMutex(m->mid);
        }
        free(m);
    }
}

#endif /* __vita__ */
