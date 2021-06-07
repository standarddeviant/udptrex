#ifndef UDPTREX_HEADER_GUARD
#define UDPTREX_HEADER_GUARD

#include <stdint.h>
#include <pthread.h>
// #include <pa_ringbuffer.h>
#include <lfqueue.h>
#include <sds.h>



#ifndef MSG_COUNT_LOG2_MAX
/* this value is the maxmimum value for msg_count_log2
So a value of 10 means the maximum value for msg_count is
2.0 ** msg_count_log2, or 2.0 ** 10 = 1024
*/
#define MSG_COUNT_LOG2_MAX (10)
#endif


/* enum fields                         */
typedef enum udptrex_dir_t {
    UDPTREX_DIR_RECV,
    UDPTREX_DIR_SEND,
    UDPTREX_NUM_MODES
}udptrex_dir_t;
typedef enum udptrex_thr_state_t {
    UDPTREX_THR_NOT_STARTED,
    UDPTREX_THR_STARTED,
    UDPTREX_THR_RUNNING,
    UDPTREX_THR_DONE,
    UDPTREX_NUM_THR_STATES
}udptrex_thr_state_t;


typedef struct {
    udptrex_dir_t       dir;
    uint16_t            port;
    uint32_t            thr_id;
    pthread_t           thread;
    lfqueue_t           q;
    volatile udptrex_thr_state_t thr_state;
    volatile uint8_t             running;
} udptrex_context_t;

udptrex_context_t * udptrex_start_context(udptrex_dir_t mode, uint16_t port);
int udptrex_stop_context(udptrex_context_t *ctx);
int udptrex_get_qsize(udptrex_context_t *ctx);
int udptrex_send1(udptrex_context_t *ctx, void *itm, size_t len);
void * udptrex_recv1(udptrex_context_t *ctx, size_t *len);
sds udptrex_recv1_sds(udptrex_context_t *ctx);
int udptrex_free1(void *itm);
int udptrex_free1_sds(sds);


// pthread_create(&fileio_thread, NULL, *fileio_function, (void *) &(thr));


#endif
