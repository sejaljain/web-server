#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mdb.h"
#include "mylist.h"


int loadmdb(FILE *fp, struct List *dest);
void freemdb(struct List *list);


int loadmdb(FILE *fp, struct List *dest) {

	int c;
	int count = 0;
	char name[16];
	char message[24];
	int index = 0;

	// initialize node containing MdbRec 
	struct MdbRec *entry = calloc(1, sizeof(struct MdbRec));
	if (entry == NULL) {
		perror("Calloc returned NULL");
		exit(1);
	}
	initMdbRec(entry);
	struct Node *node = addFront(dest, entry);
	struct Node *prev = NULL;

	// iterate over file character by character
	while((c = getc(fp)) != EOF) {
		// handle first 16 characters for name
		if(!(count%2)) {
			if(index < 15) {
				name[index] = c;
				index++;
			}
			else {
				memcpy(entry->name, name, sizeof(name));
				memset(&name[0], 0, sizeof(name));

				count++;
				index = 0;
			}

		}
		// handle following 24 characters for message
		else if(count%2) {
			if(index < 23) {
				message[index] = c;
				index++;
			}
			else {
				memcpy(entry->msg, message, sizeof(message));
				memset(&message[0], 0, sizeof(message));

				// create next node with MdbRec
				prev = node; 
				entry = calloc(1, sizeof(struct MdbRec));
				if (entry == NULL) {
					perror("Calloc returned NULL");
					exit(1);
				}
				initMdbRec(entry);
				node = addAfter(dest, prev, entry);

				count++;
				index = 0;
			}
		}

	}

	// delete and free last empty node
	
	prev = NULL;
	node = dest->head;
	while(node->next) {
		prev = node;
		node = node->next;
	}
	prev->next = NULL;
	free(node->data);
	free(node);

	return 1;

}

void freemdb(struct List *list) {
	// free all data in list
	while(list->head) {
		void *data = list->head->data;
		struct Node *new = list->head->next;
		free(list->head);
		list->head = new;
		free(data);
	}
}

