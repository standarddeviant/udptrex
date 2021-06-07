

#include <stdio.h>
#include <string.h>

#include <udptrex.h>

int main(void) {
    int ret;

    // udptrex_context_t * udptrex_start_context(udptrex_dir_t mode, size_t msg_size, size_t msg_count, uint16_t port)
    udptrex_context_t *ctx = udptrex_start_context(
         UDPTREX_DIR_RECV, // udptrex_dir_t mode = [UDPTREX_DIR_RECV, UDPTREX_DIR_SEND],
                        32, // size_t msg_size (max)
                         3, //size_t msg_count_log2, // TODO make this into an enum
                     42042  //uint16_t port)
    );
    printf("\n\nI: udptrex_context_t *ctx = %p\n", (void *)ctx);
    if(ctx == NULL)
        printf("E: ctx == NULL");

    // int udptrex_stop_context(udptrex_context_t *ctx);
    // ret = udptrex_stop_context(ctx);


    uint32_t itm_count = 0;
    while(1) {
        sds s = udptrex_recv1_sds(ctx);
        if(s) {
            printf("len = %u\n", sdslen(s));
            for(int ix=0; ix<sdslen(s); ix++) {
                printf("itm[%2i] = %c\n", ix, s[ix]);
            }
            udptrex_free1_sds(s);
            itm_count++;
            printf("itm_count = %d, qsize = %d\n", itm_count, udptrex_get_qsize(ctx));
            if(itm_count >= 5)
                break;
        }
        lfqueue_sleep(10);
    }

    // ret = udptrex_stop_context(ctx);

    // udptrex_stop_context
    printf("%s: udptrex_stop_context returned %i\n", ret ? "E" : "I", ret);

    return 0;
}

