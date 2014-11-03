#ifndef PACKETSMANAGER_H_
#define PACKETSMANAGER_H_

#include <pcap.h>
#include "Packet.h"

Packet* MakeNewPacket();
char* ParsePacket(Packet* packet);
void FreePacket(Packet* packet);
void print_payload(const u_char *payload, int len);

#endif /* PACKETSMANAGER_H_ */
