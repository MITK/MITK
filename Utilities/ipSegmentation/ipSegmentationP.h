/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
#ifndef IPSEGMENTATIONP_H
#define IPSEGMENTATIONP_H

#include <mitkIpPic.h>
#include "ipSegmentationError.h"

#ifndef IPSEGMENTATION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

#define ipMITKSegmentationTYPE ipUInt1_t
#define ipMITKSegmentationTYPE_ID ipPicUInt
#define ipMITKSegmentationBPE 8
#define tagSEGMENTATION_EMPTY "SEGMENTATION_EMPTY"

enum LogicalOp {
    IPSEGMENTATION_OP  /*! copy of input image */,
    IPSEGMENTATION_AND   /*! logical and operator */,
    IPSEGMENTATION_OR  /*! the or operator */,
    IPSEGMENTATION_XOR  /*! the exclusive-or operator */
};

extern void ipMITKSegmentationUndoSave (ipPicDescriptor* segmentation);
extern ipBool_t ipMITKSegmentationUndoIsEnabled (ipPicDescriptor* segmentation);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* IPSEGMTATION_H */

#define AND(TYPE, PIC, VALUE, X, Y)                                       \
{                                                                         \
  TYPE* data = (TYPE*) (PIC)->data;                                       \
  ipUInt4_t  width = (PIC)->n[0];                                         \
  int temp =  (int) data [width * (ipUInt4_t) Y + (ipUInt4_t)  X];        \
  temp &= (int) VALUE;                                                    \
  data [width * (ipUInt4_t) Y + (ipUInt4_t)  X] = temp;                   \
}                                                                         \

#define OR(TYPE, PIC, VALUE, X, Y)                                        \
{                                                                         \
  TYPE* data = (TYPE*) (PIC)->data;                                       \
   ipUInt4_t  width = (PIC)->n[0];                                        \
  int temp =  (int) data [width * (ipUInt4_t) Y + (ipUInt4_t)  X];        \
  temp |= (int) VALUE;                                                    \
   data [width * (ipUInt4_t) Y + (ipUInt4_t)  X]  = temp;                 \
}                                                                         \

#define  XOR(TYPE, PIC, VALUE, X, Y)                                      \
{                                                                         \
  TYPE* data = (TYPE*) (PIC)->data;                                       \
  ipUInt4_t width = (PIC)->n[0];                                          \
  int temp =  (int) data [width * (ipUInt4_t) Y + (ipUInt4_t)  X];        \
  temp ^= (int) VALUE;                                                    \
  data [width * (ipUInt4_t) Y + (ipUInt4_t)  X]  = temp;                  \
}                                                                         \

#define OP(TYPE, PIC, VALUE, X, Y)                                        \
{                                                                         \
  TYPE* data = (TYPE*) (PIC)->data;                                       \
  ipUInt4_t width = (PIC)->n[0];                                          \
  data [width * (ipUInt4_t) Y + (ipUInt4_t)  X] = VALUE;                  \
}                                                                         \

#define MASK(TYPE, PIC, MSK, VALUE, X, Y, CMD)          \
{                    \
if (!(MSK) || (((ipMITKSegmentationTYPE *)(MSK)->data)[(PIC)->n[0] * (ipUInt4_t) (Y) + (ipUInt4_t)  (X)] > 0.0)) \
CMD(TYPE, PIC, VALUE, X, Y)              \
}

#endif
