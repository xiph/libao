#ifndef __OS_TYPES_H__
#define __OS_TYPES_H__
/*
 * $Id: os_types.h,v 1.2 2002/07/19 08:34:43 msmith Exp $
 * $Name:  $
 * 
 * win32 types 
 * 04 Sept 2001 - Chris Wolf create.
 */

typedef unsigned char			uint_8;
typedef unsigned short		uint_16;
typedef unsigned int			uint_32;
typedef signed   char			sint_32;
typedef signed   short		sint_16;
typedef signed   int			sint_8;

#define snprintf _snprintf
#define M_PI            3.14159265358979323846  /* pi */
#define DLLENTRY __declspec(dllexport)

  // Temporarily hardcode this location
#define AO_PLUGIN_PATH "c:\\Program Files\\Common Files\\Xiph\\ao"

#define SHARED_LIB_EXT ".dll"

#endif /* __OS_TYPES_H__ */
