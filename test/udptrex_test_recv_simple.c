

#include <stdio.h>

#include <udptrex.h>

int main(void) {
    // udptrex_context_t * udptrex_start_context(udptrex_mode_t mode, size_t msg_size, size_t msg_count, uint16_t port)
    udptrex_context_t *ctx =
        udptrex_start_context(
            UDPTREX_MODE_RECV, // udptrex_mode_t mode = [UDPTREX_DIR_RECV, UDPTREX_DIR_SEND],
                           32, // size_t msg_size (max)
                            3, //size_t msg_count_log2, // TODO make this into an enum
                        42042  //uint16_t port)
        );
    if(ctx == NULL) {
        printf("ctx == NULL");
    }

    // int udptrex_stop_context(udptrex_context_t *ctx) {
    return 0;
}
