/** @file 
  * maps @c ipError_t type values to printable strings
*/

#ifndef lint
  static char *what = { "@(#)ipError\t\t\tDKFZ (Dept. MBI)\t"__DATE__"\t$Revision$" };
#endif

#include <stdio.h>

#include "ipTypes.h"

/** maps ipError_t type values to printable strings
 *  @param error	 the error number
 *  @return the printable string
 *
 *  AUTHOR & DATE
 */

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
