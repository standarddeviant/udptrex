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


#ifndef MAX_RECV_SZ
#define MAX_RECV_SZ  (1024)
#endif
#ifndef MAX_SEND_SZ
#define MAX_SEND_SZ  (1024)
#endif


void * udptrex_recv_thread_func(void *void_ctx);
void * udptrex_send_thread_func(void *void_ctx);


void * udptrex_recv_thread_func(void *void_ctx) {

    int sockfd, ret;
    char recvbuf[MAX_RECV_SZ];
    struct sockaddr_in servaddr, cliaddr;
    udptrex_context_t *ctx = (udptrex_context_t *)void_ctx;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(ctx->port);

    // Bind the socket with the server address
    if(bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("sockfd = %i\n", sockfd);
    
    socklen_t len;
    ssize_t nbytes_recv;

    while(ctx && ctx->running) {
        len = sizeof(cliaddr); //len is value/resuslt
        nbytes_recv = recvfrom(
            sockfd,
            (char *)recvbuf,
            MAX_RECV_SZ,
            0,
            (struct sockaddr *) &cliaddr,
            &len
        );
        printf("DBG: buf=%p, nbytes_recv=%zu\n", recvbuf, nbytes_recv);
        ret = udptrex_send1(ctx, recvbuf, nbytes_recv);
        if(ret)
            printf("E: udptrex_send1 returned %i\n", ret);
    }
    printf("closing socket\n");
    close(sockfd);

    return NULL;
}


// Driver code
void * udptrex_send_thread_func(void *void_ctx) {
    int sockfd;
    char sendbuf[MAX_SEND_SZ];
    char *hello = "Hello from client";
    struct sockaddr_in     servaddr;
    udptrex_context_t *ctx = (udptrex_context_t *)void_ctx;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ctx->port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    
    ssize_t n;
    socklen_t len;

    while(ctx && ctx->running) {
        size_t len;
        void *itm;
        itm = udptrex_recv1(ctx, &len);
        if(itm) {
            sendto(sockfd,
                (const char *)itm,
                len,
                MSG_WAITALL,
                (const struct sockaddr *) &servaddr,
                sizeof(servaddr)
            );
        }
    }

    close(sockfd);
    return 0;
}


udptrex_context_t * udptrex_create_context(
        udptrex_dir_t dir,
        uint16_t port)
{
    /* allocate */
    udptrex_context_t *ctx = calloc(1, sizeof(udptrex_context_t));
    if(NULL == ctx)
        return NULL; /* error // TODO disambiguate */

    /* mirror + default variables */
    ctx->dir = dir;
    ctx->thr_state = UDPTREX_THR_NOT_STARTED;
    ctx->running = 1;
    ctx->port = port;

    /* init lfqueue */
    if(lfqueue_init(&(ctx->q)) == -1)
        return NULL; /* error // TODO disambiguate */

    return ctx;
}


int udptrex_destroy_context(udptrex_context_t *ctx) {
    int ret_out = 0;
    if(NULL == ctx          ) return -1; // TODO

    while(udptrex_get_qsize(ctx)) {
        udptrex_free1_sds(udptrex_recv1_sds(ctx));
    }

    free((void *)ctx); // TODO check output of free

    return 0;
}


udptrex_context_t * udptrex_start_context(udptrex_dir_t dir, uint16_t port) {
    void * thr_arg;
    int ret=1;
    void * (*func_ptr)(void *) =
        (UDPTREX_DIR_RECV==dir) ? 
        &udptrex_recv_thread_func :
        &udptrex_send_thread_func;

    // create context with thread info and ringbuffer
    udptrex_context_t *ctx = udptrex_create_context(dir, port);
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
    ctx->running = 0;
    ret = pthread_cancel(ctx->thread);
    if(ret) return ret;
    ret = udptrex_destroy_context(ctx);
    if(ret) return ret;

    return 0;
}


int udptrex_get_qsize(udptrex_context_t *ctx) {
    if(ctx) {
        return lfqueue_size(&(ctx->q));
    }
    return -1;
}


int udptrex_send1(udptrex_context_t *ctx, void *itm, size_t len) {
    if(ctx) {
        sds s = sdsnewlen((const void *)itm, (size_t)len);
        if(s) {
            return lfqueue_enq(&(ctx->q), (void *)s);
        }
    }
    return -1; // TODO
}


void * udptrex_recv1(udptrex_context_t *ctx, size_t *len) {
    if(ctx) {
        const sds s = (sds)lfqueue_deq(&(ctx->q));
        if(s) {
            *len = sdslen(s);
            return (void *)s;
        }
    }
    return NULL; // TODO...
}


sds udptrex_recv1_sds(udptrex_context_t *ctx) {
    if(ctx) {
        sds s = (sds)lfqueue_deq(&(ctx->q));
        return s;
    }
    return NULL; // TODO...
}


int udptrex_free1(void *itm) {
    if(itm) {
        sds s = (sds)itm;
        sdsfree(s);
        return 0;
    }
    return -1; // TODO
}


int udptrex_free1_sds(sds s) {
    if(s) {
        sdsfree(s);
        return 0;
    }
    return -1; // TODO
}
