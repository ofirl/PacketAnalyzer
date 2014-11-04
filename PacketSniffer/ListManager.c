#include "ListManager.h"

#define BUFSIZE 500

//returns a new malloced list
List makeNewList()
{
	List newList = (List) malloc(sizeof(struct List));
	newList->head = NULL;
	newList->tail = newList->head;

	return newList;
}

//add the node to the start of the list
List AddNode(Packet* packet, List list)
{
	ListNode newNode = (ListNode) malloc(sizeof(struct ListNode));
	newNode->content = packet;

	newNode->next = list->head;

	if (list->head != NULL)
		list->head->prev = newNode;
	else
		list->tail = newNode;

	list->head = newNode;

	return list;
}

// appends the two lists, adds list2 to the end of list1.
List AppendLists(List list1, List list2)
{
	struct sniff_ip* ip = list1->head->content->ipHeader;

	printf("test0 :\n       From: %s\n", inet_ntoa(ip->ip_src));
	printf("         To: %s\n", inet_ntoa(ip->ip_dst));

	if (list2->head != NULL)
	{
		list1->tail->next = list2->head;
		list2->head->prev = list1->tail;
		list1->tail = list2->tail;
	}

	list2->head = list1->head;
	list2->tail = list1->tail;

	printf("lists appended\n");

	return list2;
}

int writeListToFile()
{
	FILE *f = fopen(SAVEFILE, "w+");
	int count = 0;

	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    return 0;
	}

	ListNode currentNode = SavedPackets->head;
	while (currentNode != NULL)
	{
		//TODO : print the packet here
		int printTest = fprintf(f, "%s\n", ParsePacket(currentNode->content));

		if (printTest == 0)
			return 0;

		currentNode = currentNode->next;
		count++;

		if (count == 1000) //TODO : delete, debug purpose
			break;
	}

	printf("wrote %d packets to file\n", count);
	fclose(f);

	return 1;
}

void FreeList(List list, int freeItems)
{
	if (freeItems)
	{
		ListNode currentNode = list->head;
		while (currentNode->next != NULL) //go over the list and freeing memory
		{
			currentNode = currentNode->next;
			FreePacket(currentNode->content);

			free(currentNode->prev);
		}

		FreePacket(currentNode->content); //free the last one
		free(currentNode);
	}

	free(list);
}

