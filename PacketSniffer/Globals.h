#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/un.h>

#include <pcap.h>
#include <pthread.h>

#include "ListManager.h"
#include "PacketsManager.h"

pthread_mutex_t* SnifferListLock;

pthread_t packetSniffer;
void* listenThreadStart(void* num);

pthread_t packetListManager;
void* listManagerThreadStart(void* arg);

struct timeval* timeout;
pcap_t* descr;

void freeAll();
