#ifndef __PROTOCOL__
#define __PROTOCOL__
#define MAXNODES 512
#define NONE 0
#define POSREQ 1
#define DATA 2
#define REJECT 3
#define DIE 4
typedef struct msg{
    uint16_t destID;
    uint16_t srcID;
    char msgID;
    char type;
    char byteMap[MAXNODES];
    uint16_t payloadSize;
    POS pos;
} MSG;

typedef struct pos{
    uint16_t x;
    uint16_t y;
} POS;

#endif
