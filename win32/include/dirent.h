#ifndef _DIRENT_H
# define _DIRENT_H
/* 
 * $Id: dirent.h,v 1.1 2001/09/05 19:10:00 cwolf Exp $
 * $Name:  $
 * 
 * Portions copyright Apache Software Foundation
 *
 * Structures and types used to implement opendir/readdir/closedir 
 * on Windows 95/NT. 
 */ 
#include <io.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 


/* struct dirent - same as Unix */ 
struct dirent 
{
    long d_ino; /* inode (always 1 in WIN32) */ 
    off_t d_off; /* offset to this dirent */ 
    unsigned short d_reclen; /* length of d_name */ 
    char d_name[_MAX_FNAME+1]; /* filename (null terminated) */ 
}; 


/* typedef DIR - not the same as Unix */ 
typedef struct 
{
    long handle; /* _findfirst/_findnext handle */ 
    short offset; /* offset into directory */ 
    short finished; /* 1 if there are not more files */ 
    struct _finddata_t fileinfo; /* from _findfirst/_findnext */ 
    char *dir; /* the dir we are reading */ 
    struct dirent dent; /* the dirent to return */ 
} DIR; 


/* Function prototypes */ 
DIR *opendir(char *); 
struct dirent *readdir(DIR *); 
int closedir(DIR *);


#define __S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
#define S_ISREG(mode)    __S_ISTYPE((mode), _S_IFREG)

#endif /* _DIRENT_H */
