#include "PacketsManager.h"

Packet* MakeNewPacket()
{
	Packet* newPacket = (Packet*) malloc(sizeof(Packet));
	return newPacket;
}

char* ParsePacket(Packet* packet)
{
	char* parsedPacket;
	//struct sniff_ethernet *ethernet = packet->ethernetHeader; //place holder for future use
	struct sniff_ip *ip = packet->ipHeader;
	struct sniff_tcp *tcp = packet->tcpHeader;
	u_char *payload = packet->payload;
	//char *ipSrc = (char*) malloc(sizeof(char) * 40);
	//char *ipDst = (char*) malloc(sizeof(char) * 40);
	char ipSrc[40];
	char ipDst[40];

	asprintf(&ipSrc, "%s", inet_ntoa(ip->ip_src));
	asprintf(&ipDst, "%s", inet_ntoa(ip->ip_dst));

	asprintf(&parsedPacket, "       From: %s\n         To: %s\n"
			"   Src port: %d\n   Dst port: %d\n"
			"   Payload (%d bytes):\n%s",
			ipSrc, ipDst,
			ntohs(tcp->th_sport), ntohs(tcp->th_dport),
			packet->size_payload, payload);

	//free(ipSrc);
	//free(ipDst);

	//print_payload(payload, packet->size_payload);

	return parsedPacket;
}

void FreePacket(Packet* packet)
{
	free(packet);
}

/*
 * print data in rows of 16 bytes: offset   hex   ascii
 *
 * 00000   47 45 54 20 2f 20 48 54  54 50 2f 31 2e 31 0d 0a   GET / HTTP/1.1..
 */
void print_hex_ascii_line(const u_char *payload, int len, int offset)
{

	int i;
	int gap;
	const u_char *ch;

	/* offset */
	printf("%05d   ", offset);

	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");

	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");

	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");
}

/*
 * print packet payload data (avoid printing binary data)
 */
void print_payload(const u_char *payload, int len)
{

	int len_rem = len;
	int line_width = 16;			/* number of bytes per line */
	int line_len;
	int offset = 0;					/* zero-based offset counter */
	const u_char *ch = payload;

	if (len <= 0)
		return;

	/* data fits on one line */
	if (len <= line_width) {
		print_hex_ascii_line(ch, len, offset);
		return;
	}

	/* data spans multiple lines */
	for ( ;; ) {
		/* compute current line length */
		line_len = line_width % len_rem;
		/* print line */
		print_hex_ascii_line(ch, line_len, offset);
		/* compute total remaining */
		len_rem = len_rem - line_len;
		/* shift pointer to remaining bytes to print */
		ch = ch + line_len;
		/* add offset */
		offset = offset + line_width;
		/* check if we have line width chars or less */
		if (len_rem <= line_width) {
			/* print last line and get out */
			print_hex_ascii_line(ch, len_rem, offset);
			break;
		}
	}
}
