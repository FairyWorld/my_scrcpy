#include "thread.h"

#include <assert.h>
#include <SDL2/SDL_thread.h>

#include "log.h"

bool
sc_thread_create(struct sc_thread *thread, sc_thread_fn fn, const char *name,
                 void *userdata) {
    SDL_Thread *sdl_thread = SDL_CreateThread(fn, name, userdata);
    if (!sdl_thread) {
        return false;
    }

    thread->thread = sdl_thread;
    return true;
}

void
sc_thread_join(struct sc_thread *thread, int *status) {
    SDL_WaitThread(thread->thread, status);
}

bool
sc_mutex_init(struct sc_mutex *mutex) {
    SDL_mutex *sdl_mutex = SDL_CreateMutex();
    if (!sdl_mutex) {
        return false;
    }

    mutex->mutex = sdl_mutex;
#ifndef NDEBUG
    mutex->locker = 0;
#endif
    return true;
}

void
sc_mutex_destroy(struct sc_mutex *mutex) {
    SDL_DestroyMutex(mutex->mutex);
}

void
sc_mutex_lock(struct sc_mutex *mutex) {
    int r = SDL_LockMutex(mutex->mutex);
#ifndef NDEBUG
    if (r) {
        LOGC("Could not lock mutex: %s", SDL_GetError());
        abort();
    }

    mutex->locker = sc_thread_get_id();
#else
    (void) r;
#endif
}

void
sc_mutex_unlock(struct sc_mutex *mutex) {
#ifndef NDEBUG
    mutex->locker = 0;
#endif
    int r = SDL_UnlockMutex(mutex->mutex);
#ifndef NDEBUG
    if (r) {
        LOGC("Could not lock mutex: %s", SDL_GetError());
        abort();
    }
#else
    (void) r;
#endif
}

sc_thread_id
sc_thread_get_id(void) {
    return SDL_ThreadID();
}

#ifndef NDEBUG
bool
sc_mutex_held(struct sc_mutex *mutex) {
    return mutex->locker == sc_thread_get_id();
}
#endif

bool
sc_cond_init(struct sc_cond *cond) {
    SDL_cond *sdl_cond = SDL_CreateCond();
    if (!sdl_cond) {
        return false;
    }

    cond->cond = sdl_cond;
    return true;
}

void
sc_cond_destroy(struct sc_cond *cond) {
    SDL_DestroyCond(cond->cond);
}

void
sc_cond_wait(struct sc_cond *cond, struct sc_mutex *mutex) {
    int r = SDL_CondWait(cond->cond, mutex->mutex);
#ifndef NDEBUG
    if (r) {
        LOGC("Could not wait on condition: %s", SDL_GetError());
        abort();
    }

    mutex->locker = sc_thread_get_id();
#else
    (void) r;
#endif
}

bool
sc_cond_timedwait(struct sc_cond *cond, struct sc_mutex *mutex, uint32_t ms) {
    int r = SDL_CondWaitTimeout(cond->cond, mutex->mutex, ms);
#ifndef NDEBUG
    if (r < 0) {
        LOGC("Could not wait on condition with timeout: %s", SDL_GetError());
        abort();
    }

    mutex->locker = sc_thread_get_id();
#endif
    assert(r == 0 || r == SDL_MUTEX_TIMEDOUT);
    return r == 0;
}

void
sc_cond_signal(struct sc_cond *cond) {
    int r = SDL_CondSignal(cond->cond);
#ifndef NDEBUG
    if (r) {
        LOGC("Could not signal a condition: %s", SDL_GetError());
        abort();
    }
#else
    (void) r;
#endif
}

void
sc_cond_broadcast(struct sc_cond *cond) {
    int r = SDL_CondBroadcast(cond->cond);
#ifndef NDEBUG
    if (r) {
        LOGC("Could not broadcast a condition: %s", SDL_GetError());
        abort();
    }
#else
    (void) r;
#endif
}
