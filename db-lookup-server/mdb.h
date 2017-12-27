/*
 * mdb.h
 */

#ifndef _MDB_H_
#define _MDB_H_

#include <stdio.h>
#include "mylist.h"

struct MdbRec {
    char name[16];
    char  msg[24];
};

// initialize MdbRec with name and msg as empty strings
static inline void initMdbRec(struct MdbRec *rec) {
	rec->name[0] = 0;
	rec->msg[0] = 0;
}

// load contents of db and store in list
int loadmdb(FILE *fp, struct List *dest);

// free all data in linked list
void freemdb(struct List *list);

#endif /* _MDB_H_ */
