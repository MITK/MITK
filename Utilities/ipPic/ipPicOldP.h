
/*
 * $RCSfile$
 *--------------------------------------------------------------------
 * DESCRIPTION
 *   the old pic header
 *
 * $Log$
 * Revision 1.1  1997/09/06 19:09:59  andre
 * Initial revision
 *
 * Revision 0.0  1993/03/26  12:56:26  andre
 * Initial revision
 *
 *
 *--------------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#ifndef _ipPicOldP_h
#define _ipPicOldP_h

typedef struct
  {
    ipUInt4_t id,
              dummy1,
              dummy2,
              conv,
              rank,
              n1,
              n2,
              n3,
              n4,
              n5,
              n6,
              n7,
              n8,
              type,
              ntxt,
              ltxt;
  } _ipPicOldHeader;
#endif    /* ifdef _ipPicOldP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
