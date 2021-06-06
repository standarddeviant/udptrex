#ifndef UDPTREX_HEADER_GUARD
#define UDPTREX_HEADER_GUARD

#include <stdint.h>
#include <pthread.h>
#include <pa_ringbuffer.h>


#ifndef MSG_COUNT_LOG2_MAX
/* this value is the maxmimum value for msg_count_log2
So a value of 10 means the maximum value for msg_count is
2.0 ** msg_count_log2, or 2.0 ** 10 = 1024
*/
#define MSG_COUNT_LOG2_MAX (10)
#endif


/* enum fields                         */
typedef enum udptrex_mode_t {
    UDPTREX_MODE_RECV,
    UDPTREX_MODE_SEND,
    UDPTREX_NUM_MODES
}udptrex_mode_t;
typedef enum udptrex_thr_state_t {
    UDPTREX_THR_NOT_STARTED,
    UDPTREX_THR_STARTED,
    UDPTREX_THR_RUNNING,
    UDPTREX_THR_DONE,
    UDPTREX_NUM_THR_STATES
}udptrex_thr_state_t;


typedef struct {
    udptrex_mode_t      mode;
    udptrex_thr_state_t thr_state;
    uint16_t            port;
    uint32_t            thr_id;
    pthread_t           thread;
    PaUtilRingBuffer    rbuf;
    void *              data_ptr;  
} udptrex_context_t;

udptrex_context_t * udptrex_start_context(udptrex_mode_t mode, size_t msg_size, size_t msg_count, uint16_t port);
int udptrex_stop_context(udptrex_context_t *ctx);


// pthread_create(&fileio_thread, NULL, *fileio_function, (void *) &(thr));


#endif
