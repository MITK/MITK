/*
 * ipPicAnnotations.h
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  declarations for the annotations in the pic format
 *
 * AUTHOR & DATE
 *  Andre Schroeter	09.01.95
 *
 * UPDATES
 *  a short history of the file
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept.MBI) HEIDELBERG, FRG
 */

#ifndef _ipPicAnnotations_h
#define _ipPicAnnotations_h

typedef enum
  {
    ipPicAUnknown	= 0,
    ipPicAText		= 1
  } ipPicAnnotations_t;

typedef struct
  {
    ipUInt4_t  xpos;
    ipUInt4_t  ypos;
         char *text;
  } ipPicAnnotation;

#endif  /* _ipPicAnnotations_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
