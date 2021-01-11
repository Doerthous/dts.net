#ifndef INET_H_
#define INET_H_

union _4barr
{
    uint32_t mem;
    struct {
        uint8_t arr[4];
    } arr;
};

union _2barr
{
    uint16_t mem;
    struct {
        uint8_t arr[2];
    } arr;
};

static uint32_t htonl(uint32_t hostlong)
{
    uint8_t b;
    union _4barr *ba = (union _4barr *)(&hostlong);
    b = ba->arr.arr[0];
    ba->arr.arr[0] = ba->arr.arr[3];
    ba->arr.arr[3] = b;
    b = ba->arr.arr[1];
    ba->arr.arr[1] = ba->arr.arr[2];
    ba->arr.arr[2] = b;
    return hostlong;
}
static inline uint16_t htons(uint16_t hostshort)
{
    uint8_t b;
    union _2barr *ba = (union _2barr *)(&hostshort);
    b = ba->arr.arr[0];
    ba->arr.arr[0] = ba->arr.arr[1];
    ba->arr.arr[1] = b;
    return hostshort;
}
static inline uint32_t ntohl(uint32_t netlong)
{
    return htonl(netlong);
}
static inline uint16_t ntohs(uint16_t netshort)
{
    return htons(netshort);
}

#endif // INET_H_
