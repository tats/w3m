/* $Id: history.h,v 1.5 2002/01/26 17:24:01 ukai Exp $ */
#ifndef HISTORY_H
#define HISTORY_H

#include "textlist.h"
#include "hash.h"

#define HIST_LIST_MAX GENERAL_LIST_MAX
#define HIST_HASH_SIZE 127

typedef ListItem HistItem;

typedef GeneralList HistList;

typedef struct {
    HistList *list;
    HistItem *current;
    Hash_sv *hash;
} Hist;

extern Hist *newHist(void);
extern Hist *copyHist(Hist *hist);
extern HistItem *unshiftHist(Hist *hist, char *ptr);
extern HistItem *pushHist(Hist *hist, char *ptr);
extern HistItem *pushHashHist(Hist *hist, char *ptr);
extern HistItem *getHashHist(Hist *hist, char *ptr);
extern char *lastHist(Hist *hist);
extern char *nextHist(Hist *hist);
extern char *prevHist(Hist *hist);

#ifdef USE_HISTORY
extern int loadHistory(Hist *hist);
extern void saveHistory(Hist *hist, size_t size);
extern void ldHist(void);
#else				/* not USE_HISTORY */
#define ldHist nulcmd
#endif				/* not USE_HISTORY */

#endif				/* HISTORY_H */
