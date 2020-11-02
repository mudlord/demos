#ifndef SYNC_DEVICE_H
#define SYNC_DEVICE_H



#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* configure inline keyword */
#if (!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901L)) && !defined(__cplusplus)
#if defined(_MSC_VER) || defined(__GNUC__) || defined(__SASC)
#define inline __inline
#else
 /* compiler does not support inline */
#define inline
#endif
#endif

/* configure lacking CRT features */
#ifdef _MSC_VER
#define strdup _strdup
/* int is 32-bit for both x86 and x64 */
typedef unsigned int uint32_t;
#define UINT32_MAX UINT_MAX
#elif defined(__GNUC__)
#include <stdint.h>
#elif defined(M68000)
typedef unsigned int uint32_t;
#endif

#ifndef SYNC_PLAYER

/* configure socket-stack */
#ifdef _WIN32
 #define WIN32_LEAN_AND_MEAN
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <winsock2.h>
 #include <windows.h>
 #include <limits.h>
#elif defined(USE_AMITCP)
 #include <sys/socket.h>
 #include <proto/exec.h>
 #include <proto/socket.h>
 #include <netdb.h>
 #define SOCKET int
 #define INVALID_SOCKET -1
 #define select(n,r,w,e,t) WaitSelect(n,r,w,e,t,0)
 #define closesocket(x) CloseSocket(x)
#else
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <unistd.h>
 #define SOCKET int
 #define INVALID_SOCKET -1
 #define closesocket(x) close(x)
#endif

int sync_set_key(struct sync_track*, const struct track_key*);
int sync_del_key(struct sync_track*, int);
static inline int is_key_frame(const struct sync_track* t, int row)
{
    return sync_find_key(t, row) >= 0;
}


struct sync_cb {
    void (*pause)(void*, int);
    void (*set_row)(void*, int);
    int (*is_playing)(void*);
};
#define SYNC_DEFAULT_PORT 1338
int sync_connect(struct sync_device*, const char*, unsigned short);
int sync_update(struct sync_device*, int, struct sync_cb*, void*);
void sync_save_tracks(const struct sync_device*);
void sync_save_tracks_mem(const struct sync_device* d);

#endif /* !defined(SYNC_PLAYER) */


struct sync_io_cb {
    void* (*open)(const char* filename, const char* mode);
    size_t(*read)(void* ptr, size_t size, size_t nitems, void* stream);
    int (*close)(void* stream);
};
void sync_set_io_cb(struct sync_device* d, struct sync_io_cb* cb);

struct sync_device {
	char *base;
	struct sync_track **tracks;
	size_t num_tracks;

#ifndef SYNC_PLAYER
	int row;
	SOCKET sock;
#else
	struct sync_io_cb io_cb;
#endif
};


enum key_type {
    KEY_STEP,   /* stay constant */
    KEY_LINEAR, /* lerp to the next value */
    KEY_SMOOTH, /* smooth curve to the next value */
    KEY_RAMP,
    KEY_TYPE_COUNT
};

struct track_key {
    int row;
    float value;
    enum key_type type;
};

struct sync_track {
    char* name;
    struct track_key* keys;
    int num_keys;
};
struct sync_device;
struct sync_track;

struct sync_device* sync_create_device(const char*);
void sync_destroy_device(struct sync_device*);

int sync_find_key(const struct sync_track*, int);
static inline int key_idx_floor(const struct sync_track* t, int row)
{
    int idx = sync_find_key(t, row);
    if (idx < 0)
        idx = -idx - 2;
    return idx;
}
const struct sync_track* sync_get_track_mem(struct sync_device* d,
    const char* name, unsigned char* data, int len);
const struct sync_track* sync_get_track(struct sync_device*, const char*);
double sync_get_val(const struct sync_track*, double);

#endif /* SYNC_DEVICE_H */
