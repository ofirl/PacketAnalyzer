#ifndef LIST_H_
#define LIST_H_

#include "PacketsManager.h"

struct ListNode {
	struct ListNode* next;
	struct ListNode* prev;
	Packet* content;
};

typedef struct ListNode* ListNode;

struct List {
	ListNode head;
	ListNode tail;
};

typedef struct List* List;

#endif /* LIST_H_ */
