

#include <stdio.h>
#include <string.h>

#include <udptrex.h>

int main(void) {
    int ret;

    // udptrex_context_t * udptrex_start_context(udptrex_dir_t mode, size_t msg_size, size_t msg_count, uint16_t port)
    udptrex_context_t *ctx =udptrex_start_context(
         UDPTREX_DIR_RECV, // udptrex_dir_t mode = [UDPTREX_DIR_RECV, UDPTREX_DIR_SEND],
                        32, // size_t msg_size (max)
                         3, //size_t msg_count_log2, // TODO make this into an enum
                     42042  //uint16_t port)
    );
    printf("\n\nI: udptrex_context_t *ctx = %p\n", (void *)ctx);
    if(ctx == NULL)
        printf("E: ctx == NULL");

    // int udptrex_stop_context(udptrex_context_t *ctx);
    ret = udptrex_stop_context(ctx);


    uint32_t itm_count = 0;
    while(1) {
        uint8_t *itm = (uint8_t *)udptrex_recv1(ctx);
        if(itm) {
            for(int ix=0; ix<sizeof(itm); ix++) {
                printf("itm_count=0: itm[%2i] = %u\n", ix, itm[ix]);
            }
            if(++itm_count >= 5)
                break;
        }
        lfqueue_sleep(10);
    }

    printf("%s: udptrex_stop_context returned %i\n", ret ? "E" : "I", ret);

    return 0;
}

