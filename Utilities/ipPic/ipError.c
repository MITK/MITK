/*
 * ipError.c
 *---------------------------------------------------------------------
 * DESCRIPTION
 *  maps ipError_t type values to printable strings
 *
 * FUNCTION DECLARATION
 *  char *ipErrorStr( ipError_t error )
 *
 * PARAMETERS
 *   error		the error number
 *
 * RETURN VALUES
 *  the printable string
 *
 * AUTHOR & DATE
 *  Andre Schroeter	05.04.1995
 *
 * UPDATES
 *  none
 *
 *---------------------------------------------------------------------
 * COPYRIGHT (c) 1995 by DKFZ (Dept. MBI) HEIDELBERG, FRG
 */
#ifndef lint
  static char *what = { "@(#)ipError\t\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>

#include "ipTypes.h"

char *ipErrorStr( ipError_t error )
{
  char *str;

  switch( error )
    {
      case ipError :
        str = "generic error";
        break;
      case ipOK :
        str = "No Error";
        break;
      default:
        str = "ipError: Unknown Error";
        fprintf( stderr, "ipErrorStr: sorry, unknown error (#%i)\n", error );
        break;
    }

  return( str );
}
