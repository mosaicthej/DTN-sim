/*Connor Tamme lrk312 11328286
 * Mark Jia mij623 11271998*/

#ifndef __PROTOCOL__
#define __PROTOCOL__
#define MAXNODES 512
#define NONE 0
#define POSREQ 1
#define DATA 2
#define REJECT 3
#define MAKEMSG 4
#define DIE 5
#define MOVE 6
#define ACCEPT 7
#define WANT 8
#define POSANS 9
typedef struct pos{
    uint16_t x;
    uint16_t y;
} POS;
typedef struct msg{
    uint16_t destID;
    uint16_t srcID;
    char msgID;
    char type;
    char byteMap[MAXNODES];
    POS pos;
} MSG;



#endif
