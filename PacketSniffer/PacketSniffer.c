#include "Globals.h"

int packetCount = 0;

/* callback function that is passed to pcap_loop(..) and called each time
 * a packet is recieved */
void packetReceived(u_char *useless,const struct pcap_pkthdr* pkthdr,const u_char*
        packet)
{
    packetCount++;
    printf("got a packet! + (%d)\n", packetCount);

	//creating the new packet
    Packet* newPacket = MakeNewPacket();
    newPacket->header = pkthdr; //TODO : delete
    newPacket->content = packet;

    /* declare pointers to packet headers */
    const struct sniff_ethernet *ethernet;  /* The ethernet header [1] */
    const struct sniff_ip *ip;              /* The IP header */
    const struct sniff_tcp *tcp;            /* The TCP header */
    const char *payload;                    /* Packet payload */

    int size_ip;
    int size_tcp;
    int size_payload;

    //Analyze packet

    /* define ethernet header */
    ethernet = (struct sniff_ethernet*)(packet);

    /* define/compute ip header offset */
    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if (size_ip < 20) {
    	printf("   * Invalid IP header length: %u bytes\n", size_ip);
    	return;
    }

    /* print source and destination IP addresses */
    //printf("       From: %s\n", inet_ntoa(ip->ip_src));
    //printf("         To: %s\n", inet_ntoa(ip->ip_dst));

    /*- determine protocol *//*
    switch(ip->ip_p) {
    case IPPROTO_TCP:
    	printf("   Protocol: TCP\n");
    	break;
    case IPPROTO_UDP:
    	printf("   Protocol: UDP\n");
    	return;
    case IPPROTO_ICMP:
    	printf("   Protocol: ICMP\n");
    	return;
    case IPPROTO_IP:
    	printf("   Protocol: IP\n");
    	return;
    default:
    	printf("   Protocol: unknown\n");
    	return;
    }*/

    /*
     *  OK, this packet is TCP.
     */

    /* define/compute tcp header offset */
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20) {
    	printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
    	return;
    }

    //printf("   Src port: %d\n", ntohs(tcp->th_sport));
    //printf("   Dst port: %d\n", ntohs(tcp->th_dport));

    /* define/compute tcp payload (segment) offset */
    payload = (u_char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

    /* compute tcp payload (segment) size */
    size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

    /*
     * Print payload data; it might be binary, so don't just
     * treat it as a string.
     */
    if (size_payload > 0) {
    	printf("   Payload (%d bytes):\n", size_payload);
    	//print_payload(payload, size_payload);
    }

    newPacket->ethernetHeader = ethernet;
    newPacket->ipHeader = ip;
    newPacket->size_ip = size_ip;
    newPacket->size_tcp = size_tcp;
    newPacket->size_payload = size_payload;
    newPacket->tcpHeader = tcp;
    newPacket->payload = payload;
    newPacket->timeStamp = (int64_t) pkthdr->ts.tv_sec;

    //adding the packet to the SnifferList
    pthread_mutex_lock(SnifferListLock);
    AddNode(newPacket, SnifferList);
    pthread_mutex_unlock(SnifferListLock);
}

int main(int argc,char **argv)
{
    if(argc != 2){ fprintf(stdout,"Usage: %s numpackets\n",argv[0]);return 0;}

    SnifferList = makeNewList();
    SavedPackets = makeNewList(); //TODO : check malloc's, its probably redundant to malloc this

    SnifferListLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(SnifferListLock,NULL);

    int argTest = 8; //TODO : change?
    pthread_create(&packetListManager, NULL, listManagerThreadStart, &argTest);
    pthread_create(&packetSniffer, NULL,listenThreadStart,argv[1]);

    pthread_join(packetSniffer, NULL);
    pthread_join(packetListManager, NULL);

    writeListToFile();

    return 0;
}

void* listenThreadStart(void* num)
{
	char *dev;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* descr;
	char filter_exp[] = "tcp";
	struct bpf_program fp;			/* compiled filter program (expression) */
	bpf_u_int32 net;			/* ip */
	bpf_u_int32 mask;			/* subnet mask */

	/* grab a device to peak into... */
	dev = pcap_lookupdev(errbuf);
	if(dev == NULL)
	{
		printf("%s\n",errbuf);
		exit(1);
	}

	printf("Device: %s\n", dev);
	printf("Filter expression: %s\n", filter_exp);

	/* open device for reading */
	descr = pcap_open_live(dev,BUFSIZ,0,-1,errbuf);
	if(descr == NULL)
	{ printf("pcap_open_live(): %s\n",errbuf); exit(1); }

	/* get network number and mask associated with capture device */
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n",
				dev, errbuf);
		net = 0;
		mask = 0;
	}

	/* compile the filter expression */
	if (pcap_compile(descr, &fp, filter_exp, 0, net) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n",
				filter_exp, pcap_geterr(descr));
		exit(EXIT_FAILURE);
	}

	/* apply the compiled filter */
	if (pcap_setfilter(descr, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n",
				filter_exp, pcap_geterr(descr));
		exit(EXIT_FAILURE);
	}

	/* allright here we call pcap_loop(..) and pass in our callback function */
	/* int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)*/
	/* If you are wondering what the user argument is all about, so am I!!   */
	pcap_loop(descr,0,packetReceived,NULL);

	fprintf(stdout,"\nDone processing packets... wheew!\n");
	return 0;
}

void* listManagerThreadStart(void* arg)
{
	//int timeout = *(int*)arg;
	List tempList = makeNewList();
	//List SnifferListCopy = makeNewList();
	int gotSignal = 0;

	while(1)
	{
		//TODO : delete
		//debug purpose
		gotSignal = 1;
		sleep(10);
		/*
		int input;
		scanf("%d", &input);
		if (input)
		{
			gotSignal = 1;
		}*/

		//listen to packets (or other method of communication) with timeout
		//TODO : add this section, if received signal set gotSignal to 1

		//add SnifferList to SavedPackets
		//get the list and get the sniffer a new list - critical code!
		pthread_mutex_lock(SnifferListLock);
		tempList->head = SnifferList->head;
		tempList->tail = SnifferList->tail;
		SnifferList->head = NULL;
		SnifferList->tail = NULL;
		pthread_mutex_unlock(SnifferListLock);
		//end of critical segment, now we will work with the list

		//tempList = SnifferListCopy;
		//FreeList(SnifferListCopy, 0); //free the previous list
		//SnifferListCopy = makeNewList();
		if (tempList->head != NULL)
			SavedPackets = AppendLists(tempList, SavedPackets);

		//clean SavedPackets
		//TODO : free packets according to their time

		//check for signal - if so, write to file
		if (gotSignal)
		{
			if (!writeListToFile())
			{
				printf("error writing to file\n");
				return 0;
			}
			printf("wrote to file\n");
			gotSignal = 0;
			//return (void*)1;
		}
	}
	return 0;
}
