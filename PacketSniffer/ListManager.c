#include "ListManager.h"
#include <time.h>

#define BUFSIZE 500
#define TIMETOREMOVEPACKETS 20 //10 min = 600 sec

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
	newNode->next = NULL;
	newNode->prev = NULL;
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
		//printing the packet, note : if payload exists the file might not be UTF-8
		int printTest = fprintf(f, "%s\n", ParsePacket(currentNode->content));

		if (printTest == 0)
			return 0;

		currentNode = currentNode->next;
		count++;
	}

	printf("wrote %d packets to file\n", count);
	fclose(f);

	return 1;
}

void FreeList(List list, int freeItems)
{
	if (freeItems)
		FreeListItems(list->head);

	free(list);
}

void FreeListItems(ListNode head)
{
	ListNode currentNode = head;
	while (currentNode->next != NULL) //go over the list and freeing memory
	{
		currentNode = currentNode->next;
		FreePacket(currentNode->prev->content);

		free(currentNode->prev);
	}

	FreePacket(currentNode->content); //free the last one
	free(currentNode);
}

int CleanList()
{
	int count = 0;
	ListNode currentNode = SavedPackets->tail;
	time_t currentTime = time(&currentTime);

	if (currentNode == NULL)
		return 0;

	while (currentNode != NULL &&
			(int64_t)currentTime - currentNode->content->timeStamp > TIMETOREMOVEPACKETS)
	{
		count++;
		currentNode = currentNode->prev;

		//SavedPackets->tail = SavedPackets->tail->prev;
		//SavedPackets->tail->next = NULL;

		//FreePacket(currentNode->content);
		//free (currentNode);
	}

	if (count == 0)
		return 0;

	if (currentNode == NULL)
	{
		FreeListItems(SavedPackets->head);
		SavedPackets->head = NULL;
		SavedPackets->tail = SavedPackets->head;
	}
	else
	{
		SavedPackets->tail = currentNode;
		FreeListItems(currentNode->next);
		SavedPackets->tail->next = NULL;
	}

	printf("Cleaned %d packets\n", count);
	return count;
}
