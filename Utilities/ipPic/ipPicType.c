/*
 * ipPicType.c
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  initialisation of an array that maps the
 *  ipPicType_t types to printable strings
 *
 * FUNCTION DECLARATION
 *  char *ipPicTypeName( ipUInt4_t type )
 *
 * PARAMETERS
 *   type		the type
 *
 * RETURN VALUES
 *  the printable string
 *
 * AUTHOR & DATE
 *  Andre Schroeter	08.05.94
 *
 * UPDATES
 *  none
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1994 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipPicType\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include "ipPic.h"

static char *_ipPicTypeName[_ipPicTypeMax] =
{
  "ipPicUnknown",
  "ipPicBool",
  "ipPicASCII",
  "ipPicInt",
  "ipPicUInt",
  "ipPicFloat",
  "ipPicUnknown",
  "ipPicNonUniform",
  "ipPicTSV"
};

char *ipPicTypeName( ipUInt4_t type )
{
  if( type > _ipPicTypeMax )
    return( _ipPicTypeName[0] );
  else
    return( _ipPicTypeName[type] );
}
