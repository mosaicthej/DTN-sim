/*Connor Tamme lrk312 11328286
 * Mark Jia mij623 11271998*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <protocol.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#define MINPORT 31000


sem_t socketSem;

int numbNodes;
int destinations;
int bufSize;
int speed;
int range;
int timeSteps;

int numbPayloadSent;
int numbHeaderSent;
int numbDelivered;

/*Ensures each thread is doing the right thing at the right time*/
void *Signaller(void *arg){
    int toMove, toSend;
    int currentTimeSteps = 0;
    int i, toMakeMSG;
    MSG msg;
    int sendingSockets[MAXNODES + 1];
    int mySocket;
    char port[21];
    char waiting = 0;
	struct addrinfo setupData, *results, *res;
    int newfd, fdmax;
    socklen_t addrlen;
    struct sockaddr_storage remoteaddr;
    fd_set master, read_fds;
    long ID = (long) arg;
    toMove = 0;
    toSend = numbNodes;

    /*Make your own socket. Need semaphore to ensure that all sockets are 
     * made before making connections*/
    sem_wait(&socketSem);
    #ifdef DEBUG
    printf("Signaller in sems\n"); fflush(stdout);
    #endif
    memset(&setupData, 0, sizeof(setupData));
	setupData.ai_family = AF_INET;
	setupData.ai_socktype = SOCK_STREAM;
	setupData.ai_flags = AI_PASSIVE;
    sprintf(port, "%ld", MINPORT + ID); 
    if (getaddrinfo(NULL, port, &setupData, &results) == -1){
		printf("Error: Failed to get address info\n"); fflush(stdout);
		exit(1);
	}
	for (res = results; res != NULL; res = res->ai_next){
		if ((mySocket = socket(res->ai_family, res->ai_socktype,\
        res->ai_protocol)) == -1){
            continue;
		}
        if (bind(mySocket, res->ai_addr, res->ai_addrlen) == -1){
            close(mySocket);
            continue;
        }
        break;
	}
    if (res == NULL){
        printf("Error: Failed to create socket\n"); fflush(stdout);
        exit(1);
    }
    freeaddrinfo(res);
    if (listen(mySocket, MAXNODES) == -1){
        printf("Error: Failed to listen\n"); fflush(stdout);
        exit(1);
    }
    #ifdef DEBUG
    printf("Signaller done sems\n"); fflush(stdout);
    #endif
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(mySocket, &master);
    read_fds = master;
    fdmax = mySocket;
    sem_post(&socketSem);
    sleep(2);
    sem_wait(&socketSem);
    sem_post(&socketSem);

    /*Make a socket and connect it to each other node*/
    #ifdef DEBUG
    printf("Signaller past sems\n"); fflush(stdout);
    #endif
    for (i = 0; i < numbNodes; i++){
        memset(&setupData, 0, sizeof(setupData));
        setupData.ai_family = AF_INET;
        setupData.ai_socktype = SOCK_STREAM;
        setupData.ai_flags = AI_PASSIVE;
        sprintf(port, "%d", MINPORT + i);
        if (getaddrinfo(NULL, port, &setupData, &results) == -1){
            printf("Error: Failed to get address info\n");fflush(stdout);
            exit(1);
        }
        for (res = results; res != NULL; res = res->ai_next){
            if ((sendingSockets[i] = socket(res->ai_family, res->ai_socktype,\
            res->ai_protocol)) == -1){
                continue;
            }
            if (connect(sendingSockets[i] , res->ai_addr, res->ai_addrlen)\
            == -1){
                close(sendingSockets[i]);
                continue;
            }
            break;
        }
        if (res == NULL){
            printf("Error: Failed to create socket\n");fflush(stdout);
            exit(1);
        }
        freeaddrinfo(res);
    }

    /*Code will run until all timesteps are done*/
    while (currentTimeSteps <= timeSteps){
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            printf("Error: Failed to select\n");
            exit(1);
        }

        /*Checks all incoming fds*/
        for (i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                /*If the message is from the main socket it is a new
                * connection*/
                if (i == mySocket){
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(mySocket, (struct sockaddr *)&remoteaddr,\
                    &addrlen);
                    if (newfd == -1){
                        printf("Error: Failed to accept connection\n");
                        fflush(stdout);
                    }
                    else{
                        FD_SET(newfd, &master);
                        if (newfd > fdmax){
                            fdmax = newfd;
                        }
                        #ifdef DEBUG
                        printf("Established new connection\n");fflush(stdout);
                        #endif
                    }
                }
                else{
                    recv(i, &msg, sizeof(msg), 0);
                    #ifdef DEBUG
                    printf("I am %ld and I got %d from %d\n",ID, msg.type,\
                    msg.srcID);
                    #endif
                    /*MOVE messages are if that node has finished moving*/
                    if (msg.type == MOVE){
                        toMove--;
                        
                    }
                    /*DATA messages mean a node is done sending*/
                    if (msg.type == DATA){
                        waiting = 0;
                        toSend++;
                    }
                }
            }
        }
        if (toMove == 0 && toSend >= numbNodes){
            /*If everyone has moved and sent then start a new timestep*/
            currentTimeSteps++;
            if (currentTimeSteps > timeSteps){
                break;
            }
            printf("Beginning Timestep: %d\n", currentTimeSteps);
            fflush(stdout);
            
            toMove = numbNodes - destinations;
            toSend = 0;
            toMakeMSG = rand()%numbNodes;
            msg.type = MOVE;
            msg.srcID = ID;
            for (i = destinations; i < numbNodes; i++){
                send(sendingSockets[i], &msg, sizeof(msg), 0);
            }
            msg.type = NONE;
        }
        if (toMove == 0 && !waiting){
            /*If everyone has moved and you aren't waiting on someone to
             * finish sending then a new person can be told to send*/
            #ifdef DEBUG
            printf("sending signal to send to %d\n", toSend);
            #endif
            if (toSend == toMakeMSG){
                msg.type = MAKEMSG;
            }
            else{
                msg.type = NONE;
            }
            msg.srcID = ID;
            send(sendingSockets[toSend], &msg, sizeof(msg), 0);
            waiting = 1;
            
        }
    }
    /*Tell all nodes to DIE once all timesteps are done*/
    msg.type = DIE;
    msg.srcID = ID;
    for (i = 0; i < numbNodes; i++){
        send(sendingSockets[i], &msg, sizeof(msg), 0);
    }
    close(mySocket);
    for (i = 0; i < numbNodes; i++){
        close(sendingSockets[i]);
    }
    printf("Number of Payloads Sent: %d\n", numbPayloadSent);
    printf("Number of Headers Sent: %d\n", numbHeaderSent);
    printf("Number of Packets Delivered: %d\n", numbDelivered);

    return NULL;
}

/*The code for each of the nodes*/
void *Node(void *arg){
    MSG *buffer;
    MSG *successBuffer;
    MSG msg;
    char port[21];
    int sendingSockets[MAXNODES + 1];
    char curMsgID = 0;
    int i, y, k, bufPos = 0, sucBufPos = 0, triedSend = 0;
    char notInBuf;
    POS pos;
    int mySocket;
	struct addrinfo setupData, *results, *res;
    int newfd, fdmax;
    socklen_t addrlen;
    struct sockaddr_storage remoteaddr;
    fd_set master, read_fds;
    long ID = (long) arg;
    /*Make your own socket. Need semaphore to ensure that all sockets are 
     * made before making connections*/
    sem_wait(&socketSem);
    #ifdef DEBUG
    printf("Node in sems\n");fflush(stdout);
    #endif

    memset(&setupData, 0, sizeof(setupData));
	setupData.ai_family = AF_INET;
	setupData.ai_socktype = SOCK_STREAM;
	setupData.ai_flags = AI_PASSIVE;
    sprintf(port, "%ld", MINPORT + ID);
    if (getaddrinfo(NULL, port, &setupData, &results) == -1){
		printf("Error: Failed to get address info\n");fflush(stdout);
		exit(1);
	}
	for (res = results; res != NULL; res = res->ai_next){
		if ((mySocket = socket(res->ai_family, res->ai_socktype,\
        res->ai_protocol)) == -1){
            continue;
		}
        if (bind(mySocket, res->ai_addr, res->ai_addrlen) == -1){
            close(mySocket);
            continue;
        }
        break;
	}
    if (res == NULL){
        printf("Error: Failed to create socket\n");fflush(stdout);
        exit(1);
    }
    freeaddrinfo(res);
    if (listen(mySocket, MAXNODES) == -1){
        printf("Error: Failed to listen\n");fflush(stdout);
        exit(1);
    }
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(mySocket, &master);
    read_fds = master;
    fdmax = mySocket;
    #ifdef DEBUG
    printf("Node done sems\n");fflush(stdout);
    #endif
    sem_post(&socketSem);
    sleep(2);
    sem_wait(&socketSem);
    sem_post(&socketSem);
    #ifdef DEBUG
    printf("Node past sems\n");fflush(stdout);
    #endif
    /*Make a socket and connect it to each other node*/
    for (i = 0; i < numbNodes; i++){
        if (i == ID){
            continue;
        }
        memset(&setupData, 0, sizeof(setupData));
        setupData.ai_family = AF_INET;
        setupData.ai_socktype = SOCK_STREAM;
        setupData.ai_flags = AI_PASSIVE;
        sprintf(port, "%d", MINPORT + i); 
        if (getaddrinfo(NULL, port, &setupData, &results) == -1){
            printf("Error: Failed to get address info\n");fflush(stdout);
            exit(1);
        }
        for (res = results; res != NULL; res = res->ai_next){
            if ((sendingSockets[i] = socket(res->ai_family, res->ai_socktype,\
            res->ai_protocol)) == -1){
                continue;
            }
            if (connect(sendingSockets[i] , res->ai_addr, res->ai_addrlen)\
            == -1){
                close(sendingSockets[i]);
                continue;
            }
            break;
        }
        if (res == NULL){
            printf("Error: Failed to create socket\n");fflush(stdout);
            exit(1);
        }
        freeaddrinfo(res);
    }
    /*Make a socket and connect it to the signaller*/
    memset(&setupData, 0, sizeof(setupData));
    setupData.ai_family = AF_INET;
    setupData.ai_socktype = SOCK_STREAM;
    setupData.ai_flags = AI_PASSIVE;
    sprintf(port, "%d", MINPORT + MAXNODES);
    if (getaddrinfo(NULL, port, &setupData, &results) == -1){
        printf("Error: Failed to get address info\n");fflush(stdout);
        exit(1);
    }
    for (res = results; res != NULL; res = res->ai_next){
        if ((sendingSockets[MAXNODES] = socket(res->ai_family,\
        res->ai_socktype,res->ai_protocol)) == -1){
            continue;
        }
        if (connect(sendingSockets[MAXNODES] , res->ai_addr, \
        res->ai_addrlen) == -1){
            close(sendingSockets[MAXNODES]);
            continue;
        }
        break;
    }
    if (res == NULL){
        printf("Error: Failed to create socket\n");fflush(stdout);
        exit(1);
    }
    freeaddrinfo(res);
    pos.x = rand()%1001;
    pos.y = rand()%1001;
    buffer = (MSG*)calloc(bufSize, sizeof(MSG));
    successBuffer = (MSG*)calloc(bufSize, sizeof(MSG));
    while(1){
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
            printf("Error: Failed to select\n");
            exit(1);
        }
        for (k = 0; k <= fdmax; k++){
            if(FD_ISSET(k, &read_fds)){
                /*If the message is from the main socket it is a new
                * connection*/
                if (k == mySocket){
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(mySocket, (struct sockaddr *)&remoteaddr,\
                    &addrlen);
                    if (newfd == -1){
                        printf("Error: Failed to accept connection\n");
                        fflush(stdout);
                    }
                    else{
                        FD_SET(newfd, &master);
                        if (newfd > fdmax){
                            fdmax = newfd;
                        }
                        #ifdef DEBUG
                        printf("Established new connection on %ld\n", ID);
                        fflush(stdout);
                        #endif
                    }
                }
                else{
                    recv(k, &msg, sizeof(msg), 0);
                    #ifdef DEBUG
                    printf("I am %ld and I got %d from %d\n",ID, msg.type,\
                    msg.srcID);
                    #endif
                    /*WANT messages ask if you want the message. Send back a 
                     * yes or no*/
                    if (msg.type == WANT){
                        notInBuf = 1;
                        for (i = 0; i < bufSize; i++){
                            if (buffer[i].srcID == msg.srcID && \
                            buffer[i].msgID == msg.msgID){
                                notInBuf = 0;
                                break;
                            }
                            if (successBuffer[i].srcID == msg.srcID && \
                            successBuffer[i].msgID == msg.msgID){
                                notInBuf = 0;
                                break;
                            }
                        }
                        if (notInBuf){
                            
                            msg.type = ACCEPT;
                        }
                        else{
                            msg.type = REJECT;
                        }
                        msg.destID = msg.srcID;
                        msg.srcID = ID;
                        send(sendingSockets[msg.destID], &msg, sizeof(msg), 0);
                    }
                    /*DATA messages are new data to buffer*/
                    if (msg.type == DATA){
                        
                        if (msg.destID == ID){
                            printf("Packet delivered to node %ld\n", ID);
                            memcpy(&successBuffer[sucBufPos], &msg, \
                            sizeof(MSG));
                            sucBufPos = (sucBufPos + 1) % bufSize;
                            numbDelivered++;
                        }
                        else{
                            memcpy(&buffer[bufPos], &msg, sizeof(msg));
                            bufPos = (bufPos + 1) % bufSize;
                        }
                    }
                    /*POSANS messages are answer to position requests*/
                    if (msg.type == POSANS){
                        #ifdef DEBUG
                        printf("I am %ld is processing POSANS from %d\n",
                            ID, msg.srcID);
                        #endif
                        if (abs(msg.pos.x - pos.x) + abs(msg.pos.y - pos.y)\
                        <= range){
                            msg.type = WANT;
                            y = msg.srcID;
                            printf("Node: %ld in range of node %d\n", ID, y);
                            /*If within range ask them if they want any of
                             * your messages*/
                            for (i = 0; i < bufSize; i++){
                                if (buffer[i].byteMap[y] == 0){
                                    msg.srcID = buffer[i].srcID;
                                    msg.msgID = buffer[i].msgID;
                                    msg.destID = buffer[i].destID;
                                    msg.pos.x = i;
                                    numbHeaderSent++;
                                    send(sendingSockets[y], &msg, \
                                    sizeof(msg), 0);
                                }
                                
                            }
                        }
                        triedSend++;
                    }
                    /*A POSREQ asks for your position*/
                    if (msg.type == POSREQ){

                        msg.pos.x = pos.x;
                        msg.pos.y = pos.y;
                        msg.type = POSANS;
                        msg.destID = msg.srcID;
                        msg.srcID = ID;
                        send(sendingSockets[msg.destID], &msg, sizeof(msg), 0);
                        #ifdef DEBUG
                        printf("%ld sent pos answers to %d.\n", ID, \
                        msg.destID);
                        #endif
                    }
                    /*ACCEPT means they want that message*/
                    if (msg.type == ACCEPT){
                        /*Send the desired packet to them.*/
                        buffer[msg.pos.x].byteMap[msg.srcID] = 1;
                        numbPayloadSent++;
                        send(sendingSockets[msg.srcID], &buffer[msg.pos.x], \
                        sizeof(msg), 0);
                    }
                    /*DIE means the program is finished running*/
                    if (msg.type == DIE){
                        close(mySocket);
                        for (i = 0; i < numbNodes; i++){
                            close(sendingSockets[i]);
                        }
                        close(sendingSockets[MAXNODES]);
                        return NULL;
                    }
                    /*NONE and MAKEMSG messages are from the signaller. NONE 
                     * means its your turn to send, and MAKEMSG means it is 
                     * your turn to send and to make a new message*/
                    if (msg.type == NONE || msg.type == MAKEMSG){
                        if (msg.type == MAKEMSG){
                            #ifdef DEBUG
                            printf("I am %ld, processing this MAKEMSG\n", ID);
                            #endif
                            buffer[bufPos].srcID = ID;
                            buffer[bufPos].destID = rand()%destinations;
                            buffer[bufPos].type = DATA;
                            buffer[bufPos].msgID = curMsgID;
                            curMsgID = (curMsgID + 1)%256;
                            memset(&buffer[bufPos].byteMap, 0, MAXNODES);
                            bufPos = (bufPos + 1) % bufSize;
                        }
                        triedSend = 0;
                        msg.type = POSREQ;
                        msg.srcID = ID;
                        #ifdef DEBUG
                        printf("This message is from %d\n", msg.srcID);
                        #endif
                        for (i = 0; i < numbNodes; i++){
                            if (i == ID){
                                #ifdef DEBUG
                                printf("%ld: I don't want to send to myself \
                                (POSREQ), check for %d others!\n", ID, \
                                numbNodes);
                                #endif
                                continue;
                            }
                            msg.destID = i;
                            #ifdef DEBUG
                            printf("%ld: Ha, sending POSREQ %d to %d\n", ID,\
                            msg.type, i);
                            #endif
                            send(sendingSockets[i], &msg, sizeof(msg), 0);
                        }
                    }
                    /*MOVE is from signaller and it means it is time to move*/
                    if (msg.type == MOVE){
                        /*Start of movement*/
                        msg.pos.x = pos.x;
                        msg.pos.y = pos.y;
                        i = rand()%8;
                        for (y = 0; y < speed; y++){
                            if (ID < destinations){
                                break;
                            }

                            if (i == 0){
                                if (pos.y == 1000){
                                    break;
                                }
                                pos.y++;
                            }
                            if (i == 1){
                                if (pos.x == 1000 || pos.y == 1000){
                                    break;
                                }
                                pos.x++;
                                pos.y++;
                            }
                            if (i == 2){
                                if (pos.x == 1000){
                                    break;
                                }
                                pos.x++;
                            }
                            if (i == 3){
                                if (pos.x == 1000 || pos.y == 0){
                                    break;
                                }
                                pos.x++;
                                pos.y--;
                            }
                            if (i == 4){
                                if (pos.y == 0){
                                    break;
                                }
                                pos.y--;
                            }
                            if (i == 5){
                                if (pos.x == 0 || pos.y == 0){
                                    break;
                                }
                                pos.x--;
                                pos.y--;
                            }
                            if (i == 6){
                                if (pos.x == 0){
                                    break;
                                }
                                pos.x--;
                            }
                            if (i == 7){
                                if (pos.x == 0 || pos.y == 1000){
                                    break;
                                }
                                pos.x--;
                                pos.y++;
                            }
                        }
                        printf("Node: %ld is moving from x:%d y:%d to x:%d \
y:%d\n",ID, msg.pos.x,msg.pos.y, pos.x, pos.y);fflush(stdout);
                        msg.type = MOVE;
                        send(sendingSockets[MAXNODES], &msg, sizeof(msg), 0);
                    }
                }
            }
        }
        if (triedSend >= numbNodes - 1){
            msg.type = DATA;
            msg.srcID = ID;
            triedSend = 0;
            send(sendingSockets[MAXNODES], &msg, sizeof(msg), 0);
        }
    }  
}   

int main (int argc, char* argv[]){
    long i;
    pthread_t thread;
    srand(time(NULL));
    sem_init(&socketSem, 0, 0);
    if (argc != 7){
        printf("Incorrect arguement count. Should have 6 in the format \
(number of nodes, number of destinations, timesteps, speed, range, buffer\
size)\n");
        fflush(stdout);
        return 1;
    }
    numbNodes = atoi(argv[1]);
    if (numbNodes > 512 || numbNodes < 0){
        printf("Number of nodes out of range (must be 0 < nodes < 512)\n");
        return 1;
    }
    destinations = atoi(argv[2]);
    if (destinations > numbNodes || numbNodes < 0){
        printf("Number of destinations out of range (must be 0 < dests < \
numbNodes)\n");
        return 1;
    }
    timeSteps = atoi(argv[3]);
    if (timeSteps < 0){
        printf("Timesteps out of range (must be 0 < timesteps)\n");
        return 1;
    }
    speed = atoi(argv[4]);
    if (speed < 0){
        printf("Speed out of range (must be 0 < speed)\n");
        return 1;
    }
    range = atoi(argv[5]);
    if (range < 0){
        printf("Range out of range (must be 0 < range)\n");
        return 1;
    }
    bufSize = atoi(argv[6]);
    if (bufSize < 0){
        printf("Buffer size out of range (must be 0 < buffer size)\n");
        return 1;
    }

    for (i = 0; i < numbNodes; i++){
        pthread_create(&thread, NULL, (void*(*)(void*))Node, (void*)i);
    }
    i = MAXNODES;
    pthread_create(&thread, NULL, (void*(*)(void*)) Signaller, (void*)i);
    sem_post(&socketSem);
    pthread_join(thread, NULL);
    return 0;
}
