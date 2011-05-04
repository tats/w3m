/* $Id: hash.h,v 1.6 2003/09/24 18:48:59 ukai Exp $ */
#ifndef HASH_H
#define HASH_H

/* hash table */

#define defhash(keytype,type,sym) \
typedef struct HashItem_##sym { \
  keytype key; \
  type value; \
  struct HashItem_##sym *next; \
} HashItem_##sym;  \
typedef struct Hash_##sym { \
  int size; \
  struct HashItem_##sym **tab; \
} Hash_##sym; \
extern Hash_##sym *newHash_##sym(int size); \
extern void putHash_##sym(Hash_##sym *t, keytype key, type value); \
extern type getHash_##sym(Hash_##sym *t, keytype key, type failval);

defhash(char *, int, si)
defhash(char *, char *, ss)
defhash(char *, void *, sv)
defhash(int, void *, iv)
#define defhashfunc(keytype,type,sym) \
Hash_##sym * \
newHash_##sym(int size)\
{\
  struct Hash_##sym *hash;\
  int i;\
\
  hash = (Hash_##sym*)GC_malloc(sizeof(Hash_##sym));\
  hash->size = size;\
  hash->tab = (HashItem_##sym**)GC_malloc(size*sizeof(HashItem_##sym*));\
  for (i = 0; i < size; i++)\
    hash->tab[i] = NULL;\
  return hash;\
}\
\
static HashItem_##sym* \
lookupHash_##sym(Hash_##sym *t, keytype key, int *hashval_return)\
{\
  HashItem_##sym *hi;\
\
  *hashval_return = hashfunc(key)%t->size;\
  for (hi = t->tab[*hashval_return]; hi != NULL; hi = hi->next) {\
    if (keycomp(hi->key,key))\
      return hi;\
  }\
  return NULL;\
}\
\
void \
putHash_##sym(Hash_##sym *t, keytype key, type value)\
{\
  int h;\
  HashItem_##sym *hi;\
\
  hi = lookupHash_##sym(t,key,&h);\
  if (hi) {\
    hi->value = value;\
    return;\
  }\
\
  hi = (HashItem_##sym*)GC_malloc(sizeof(HashItem_##sym));\
  hi->key = key;\
  hi->value = value;\
  hi->next = t->tab[h];\
  t->tab[h] = hi;\
}\
\
type \
getHash_##sym(Hash_##sym *t, keytype key, type failval)\
{\
  int h;\
  HashItem_##sym *hi;\
\
  hi = lookupHash_##sym(t,key,&h);\
  if (hi == NULL)\
    return failval;\
  return hi->value;\
}
#define defhashfunc_i(keytype,type,sym) \
Hash_##sym * \
newHash_##sym(int size)\
{\
  struct Hash_##sym *hash;\
  int i;\
\
  hash = (Hash_##sym*)GC_malloc(sizeof(Hash_##sym));\
  hash->size = size;\
  hash->tab = (HashItem_##sym**)GC_malloc(size*sizeof(HashItem_##sym*));\
  for (i = 0; i < size; i++)\
    hash->tab[i] = NULL;\
  return hash;\
}\
\
static HashItem_##sym* \
lookupHash_##sym(Hash_##sym *t, keytype key, int *hashval_return)\
{\
  HashItem_##sym *hi;\
\
  *hashval_return = key%t->size;\
  for (hi = t->tab[*hashval_return]; hi != NULL; hi = hi->next) {\
    if (hi->key == key)\
      return hi;\
  }\
  return NULL;\
}\
\
void \
putHash_##sym(Hash_##sym *t, keytype key, type value)\
{\
  int h;\
  HashItem_##sym *hi;\
\
  hi = lookupHash_##sym(t,key,&h);\
  if (hi) {\
    hi->value = value;\
    return;\
  }\
\
  hi = (HashItem_##sym*)GC_malloc(sizeof(HashItem_##sym));\
  hi->key = key;\
  hi->value = value;\
  hi->next = t->tab[h];\
  t->tab[h] = hi;\
}\
\
type \
getHash_##sym(Hash_##sym *t, keytype key, type failval)\
{\
  int h;\
  HashItem_##sym *hi;\
\
  hi = lookupHash_##sym(t,key,&h);\
  if (hi == NULL)\
    return failval;\
  return hi->value;\
}
#endif				/* not HASH_H */
