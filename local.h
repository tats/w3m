/* $Id: local.h,v 1.3 2001/11/20 17:49:23 ukai Exp $ */
/*
 * w3m local.h
 */

#ifndef LOCAL_H
#define LOCAL_H

#include <sys/types.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
typedef struct dirent Directory;
#else				/* not HAVE_DIRENT_H */
#include <sys/dir.h>
typedef struct direct Directory;
#endif				/* not HAVE_DIRENT_H */
#include <sys/stat.h>

#ifndef S_IFMT
#define S_IFMT  0170000
#endif				/* not S_IFMT */
#ifndef S_IFREG
#define S_IFREG 0100000
#endif				/* not S_IFREG */

#define NOT_REGULAR(m)	(((m) & S_IFMT) != S_IFREG)
#define IS_DIRECTORY(m) (((m) & S_IFMT) == S_IFDIR)

#ifndef S_ISDIR
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif				/* not S_IFDIR */
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif				/* not S_ISDIR */

#ifdef HAVE_READLINK
#ifndef S_IFLNK
#define S_IFLNK 0120000
#endif				/* not S_IFLNK */
#ifndef S_ISLNK
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#endif				/* not S_ISLNK */
#endif				/* not HAVE_READLINK */

#endif				/* not LOCAL_H */
