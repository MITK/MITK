/*
 * ipPicAnnotation.h
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
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */

#ifndef _ipPicAnnotation_h
#define _ipPicAnnotation_h

typedef enum
  {
    ipPicAUnknown	= 0,
    ipPicAText		= 1
  } ipPicAnnotationType_t;

typedef struct
  {
                ipUInt4_t  _magic;

    ipPicAnnotationType_t  type;

                ipUInt4_t  location_x;
                ipUInt4_t  location_y;

                ipUInt4_t  position_x;
                ipUInt4_t  position_y;
                     char *text;
  } ipPicAnnotation_t;

#endif  /* _ipPicAnnotation_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

