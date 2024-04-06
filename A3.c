
#define MINSOCK 30000

int numbNodes;
int destinations;
int bufSize;
int speed;
int range;
int timeSteps;

int numbPayloadSent;
int numbHeaderSent;
int numbDelivered;

void Signaller(int ID){
    int recvSoFar;
    int currentTimeSteps;
    int i;
    MSG msg;
    /*TODO: make socket on port 30000 + ID*/
    while (currentTimeSteps < timeSteps){
        for (i = 0; i < numbNodes; i++){
            sendTo port 30000 + i
        }
        recvSofar = 0;
        while (recvSoFar < numbNodes){
            recv()
            recvSoFar++;
        }
        for (i = 0; i <numbNodes; i ++){
            sendTo port 30000 + i;
            recv()
        }
        currentTimeSteps++;
    }
    msg.type = DIE;
    for (i = 0; i < numbNodes; i++){
        sendTo port 30000 + i
    }
    return;
}

void Node(int ID){
    MSG *buffer;
    MSG *successBuffer;
    MSG msg;
    char curMsgID = 0;
    int i, y;
    char notInBuf;
    POS myPos;
    /*TODO set up rand*/
    myPos.x = Rand()%1001;
    myPos.y = Rand()%1001;
    buffer = calloc(bufSize, sizeof(MSG));
    successBuffer = calloc(bufSize, sizeof(MSG));
    /*TODO make socket on port MONSOCK + ID*/
    while(1){
        while (msg.srcID != numbNodes){
            /*TODO set up recv*/
            recv(msg)
            if (msg.type == POSREQ){
                notInBuf = 1;
                for (i = 0; i < bufSize. i++){
                    if (buffer[i].srcID == msg.srcID && buffer[i].msgID == \
                    msg.msgID){
                        notInBuf = 0;
                        break;
                    }
                    if (successbuffer[i].srcID == msg.srcID && \
                    successbuffer[i].msgID == msg.msgID){
                        notInBuf = 0;
                        break;
                    }
                }
                if (notInBuf){
                    memcpy(&msg.pos, &pos, sizeof(pos));
                    msg.destID = msg.srcID;
                    msg.srcID = ID;
                    msg.type = DATA;
                }
                else{
                    msg.type = REJECT;
                }
                /*TODO set up send*/
                send(msg to dest)
            }
            if (msg.type == DATA){
                if (msg.destID == ID){
                    memmove(&successBuffer, &successBuffer[1], sizeof(MSG) * (bufSize-1));
                    memcpy(&successBuffer, &msg, sizeof(MSG));
                    numbDelivered++;
                }
                else{
                    memmove(&buffer, &buffer[1], sizeof(msg) * (bufSize-1));
                    memcpy(&buffer, &msg, sizeof(msg));
                }
            }
            if (msg.type == DIE){
                return;
            }
        }
        if (msg.type == MOVE){
        /*Start of movement*/
        i = Rand()&8;
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
        }
        else(){
        msg.srcID = ID;
        msg.destID = Rand()%destinations;
        msg.type = DATA;
        msg.msgID = curMsgID;
        curMsgID = (curMsgID + 1)%256;
        memset(&msg.byteMap, 0, MAXNODES);
        memmove(&buffer, &buffer[1], sizeof(msg) * (bufSize-1));
        memcpy(&buffer, &msg, sizeof(msg));
        for (i = 0; i < numbNodes; i++){
            if (ID == i){
                continue;
            }
            msg.destID = i;
            msg.type = POSREQ;
            send(msg to port 30000 + i)
            recv(msg)
            numbHeaderSent++;
            if (msg.type != REJECT && abs(msg.pos.x - pos.x) + abs(msg.pos.y - pos.y) <= range){
                for (y = 0; y < bufSize; y++){
                    if (buffer[y].byteMap[i] == 0){
                        buffer[y].byteMap[i] = 1;
                        send(buffer[y] to port 30000 + i)
                        numbPayloadSent++;
                    }
                }
            }
        }
        }
        send(to signaller I done moving or sending)
    }
}   

int main (int argc, char* argv[]){

    /*TODO add for loop to make threads*/
    return 0;    
}
