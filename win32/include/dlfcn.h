#ifndef __DLFCN_H__
# define __DLFCN_H__
/*
 * $Id: dlfcn.h,v 1.1 2001/09/05 19:10:00 cwolf Exp $
 * $Name:  $
 * 
 *
 */
extern void *dlopen  (const char *file, int mode);
extern int   dlclose (void *handle);
extern void *dlsym   (void * handle, const char * name);
extern char *dlerror (void);

#define RTLD_NOW 0

#endif /* __DLFCN_H__ */
