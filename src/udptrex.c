// Server side implementation of UDP client-server model
// and
// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>

#include <udptrex.h>


#define PORT     8080
#define MAXLINE 1024


void * udptrex_recv_thread_func(void *void_ctx);
void * udptrex_send_thread_func(void *void_ctx);

/* power-of-two convenience functions */
uint32_t u32_set_bits(uint32_t i);
uint32_t u32_msb(uint32_t x);
uint32_t u32_is_pow2(uint32_t i);
uint32_t u32_force_pow2(uint32_t i, uint32_t up);
uint32_t u32_set_bits(uint32_t i) {
     // Java: use int, and use >>> instead of >>. Or use Integer.bitCount()
     // C or C++: use uint32_t
     i = i - ((i >> 1) & 0x55555555);        // add pairs of bits
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
     i = (i + (i >> 4)) & 0x0F0F0F0F;        // groups of 8
     return (i * 0x01010101) >> 24;          // horizontal sum of bytes
}
uint32_t u32_msb(uint32_t x) {
    static const uint32_t bval[] =
    { 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

    unsigned int base = 0;
    if (x & 0xFFFF0000) { base += 32/2; x >>= 32/2; }
    if (x & 0x0000FF00) { base += 32/4; x >>= 32/4; }
    if (x & 0x000000F0) { base += 32/8; x >>= 32/8; }
    return base + bval[x];
}
uint32_t u32_is_pow2(uint32_t i) {
    return (1 == u32_set_bits(i));
}
uint32_t u32_force_pow2(uint32_t i, uint32_t up) {
    if(u32_is_pow2(i)) return i;
    return (uint32_t) up ?
        1 << (u32_msb(i) + 1) :
        1 << (u32_msb(i) + 0) ;
}



// Driver code
void * udptrex_recv_thread_func(void *void_ctx) {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;
    udptrex_context_t *c = (udptrex_context_t *)void_ctx;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    socklen_t len;
    ssize_t n;

    len = sizeof(cliaddr); //len is value/resuslt
    n = recvfrom(
        sockfd,
        (char *)buffer, MAXLINE,
        MSG_WAITALL,
        ( struct sockaddr *) &cliaddr,
        &len
    );
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    sendto(
        sockfd,
        (const char *)hello,
        strlen(hello),
        MSG_WAITALL,
        (const struct sockaddr *) &cliaddr,
        len
    );
    printf("Hello message sent.\n");
    
    return NULL;
}



#define PORT     8080
#define MAXLINE 1024

// Driver code
void * udptrex_send_thread_func(void *void_ctx) {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from client";
    struct sockaddr_in     servaddr;
    udptrex_context_t *c = (udptrex_context_t *)void_ctx;    


    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    ssize_t n;
    socklen_t len;
    
    sendto(sockfd,
        (const char *)hello, strlen(hello),
        MSG_WAITALL,
        (const struct sockaddr *) &servaddr,
        sizeof(servaddr)
    );
    printf("Hello message sent.\n");
        
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;
}


udptrex_context_t * udptrex_create_context(
        udptrex_mode_t mode,
        ring_buffer_size_t msg_size,
        ring_buffer_size_t msg_count_log2,
        uint16_t port)
{
    ring_buffer_size_t rbuf_sz;
    udptrex_context_t *ctx = calloc(1, sizeof(udptrex_context_t));
    if(NULL == ctx) {
        return NULL; /* error */
    }

    assert(msg_count_log2 <= MSG_COUNT_LOG2_MAX);

    ctx->data_ptr = calloc((size_t)(1<<msg_count_log2), (size_t)msg_size);
    if(NULL == ctx->data_ptr) {
        free(ctx);
        return NULL;
    }

    /* mirror + default variables */
    ctx->mode = mode;
    ctx->thr_state = UDPTREX_THR_NOT_STARTED;

    /* init rbuf */
    rbuf_sz = PaUtil_InitializeRingBuffer(
        &(ctx->rbuf), msg_size, 1<<msg_count_log2, ctx->data_ptr
    );
    // assert rbuf_sz == (msg_size * (1<<msg_count_log2))

    return ctx;
}


int udptrex_destroy_context(udptrex_context_t *ctx) {
    int ret_out = 0;
    if(NULL == ctx          ) ret_out = -1; // TODO

    if(NULL == ctx->data_ptr) ret_out = -2; // TODO
    else free(ctx->data_ptr); // TODO check output of free

    free((void *)ctx); // TODO check output of free
    
    return ret_out;
}


udptrex_context_t * udptrex_start_context(udptrex_mode_t mode, size_t msg_size, size_t msg_count, uint16_t port) {
    void * thr_arg;
    int ret=1;
    void * (*func_ptr)(void *) =
        (UDPTREX_MODE_RECV==mode) ? 
        &udptrex_recv_thread_func :
        &udptrex_send_thread_func;

    // create context with thread info and ringbuffer
    udptrex_context_t *ctx = udptrex_create_context(mode, msg_size, msg_count, port);
    if(ctx == NULL) {
        return NULL;
    }

    // start background thread
    ret = pthread_create(
        &(ctx->thread),             // ptr to thread obj
        NULL,                       // ptr to thread params
        *udptrex_recv_thread_func,  // ptr to thread func (weird syntax)
        (void *)ctx                 // ptr to thread arg (convenient)
    );
    if(ret) {
        // unable to properly start thread
        udptrex_destroy_context(ctx);
        // TODO - communicate error value of ret
        return NULL;
    }

    return ctx;
}


int udptrex_stop_context(udptrex_context_t *ctx) {
    int ret;
    if(NULL == ctx) return -1; // TODO
    ret = pthread_cancel(ctx->thread);
    if(ret) return ret;
    ret = udptrex_destroy_context(ctx);
    if(ret) return ret;

    return 0;
}


// typedef struct {
//     unsigned int flags;
//     unsigned int status;
//     pthread_id_np_t tid;
//     pthread_t thread;
//     PaUtilRingBuffer *rbuf;
// } udptrex_context_t;

