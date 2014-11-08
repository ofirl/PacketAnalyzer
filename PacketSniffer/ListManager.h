#ifndef LISTMANAGER_H_
#define LISTMANAGER_H_

#include <stdlib.h>
#include <stdio.h>

#include "List.h"
#include "Packet.h"

#define SAVEFILE "/home/ofirl/Desktop/test/SavedPackets.txt"

List SnifferList;
List SavedPackets;

List makeNewList();
List AddNode(Packet* packet, List list);
List AppendLists(List list1, List list2);
int writeListToFile();
void FreeList(List list, int freeItems);
void FreeListItems(ListNode head);
int CleanList();

#endif /* LISTMANAGER_H_ */
