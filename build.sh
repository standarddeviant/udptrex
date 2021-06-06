
gcc                           \
    -o fixme_static_lib       \
    -I ./include              \
    ./src/pa_ringbuffer.c     \
    ./src/udptrex.c           \
    -lm

