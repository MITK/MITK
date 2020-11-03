/*============================================================================

 Copyright (c) German Cancer Research Center (DKFZ)
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 - All advertising materials mentioning features or use of this software must
   display the following acknowledgement:

     "This product includes software developed by the German Cancer Research
      Center (DKFZ)."

 - Neither the name of the German Cancer Research Center (DKFZ) nor the names
   of its contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE GERMAN CANCER RESEARCH CENTER (DKFZ) AND
   CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
   BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE GERMAN
   CANCER RESEARCH CENTER (DKFZ) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
   DAMAGE.

============================================================================*/

/*
 * AnnotationP.h
 *---------------------------------------------------------------
 * DESCRIPTION
 *   private header file for the annotation widget
 *
 * Author
 *   a.schroeter
 *
 *---------------------------------------------------------------
 *
 */

#ifndef _XipAnnotationP_h
#define _XipAnnotationP_h

/* superclass */
#include <Xm/XmP.h>

#if( XmVersion >= 1002 )
#include <Xm/PrimitiveP.h>
#endif


/* public header for this widget */
#include "Annotation.h"

/* other headers */


#ifndef XlibSpecificationRelease
#define XrmPermStringToQuark XrmStringToQuark
#endif

/*private macros */
#ifndef Min
#define Min(x, y)       (((x) < (y)) ? (x) : (y))
#endif
#ifndef Max
#define Max(x, y)       (((x) > (y)) ? (x) : (y))
#endif

/* new fields for this widget's class record */
typedef struct _XipAnnotationClassPart
  {
    XtPointer	extension;
  } XipAnnotationClassPart;

/* full class record declaration */
typedef struct _XipAnnotationClassRec
  {
    CoreClassPart		core_class;
    XmPrimitiveClassPart	primitive_class;
    XipAnnotationClassPart	annotation_class;
  } XipAnnotationClassRec;

extern XipAnnotationClassRec xipAnnotationClassRec;

/* new fields for this widget's instance record */
typedef struct _XipAnnotationPart
  { /* resources */

    Cursor               cursor;
    XtCallbackList	 activate_CB;
    Pixmap		 icon_text;

    /* private variables */
    GC			 draw_GC;
  } XipAnnotationPart;

/* full instance record declaration */
typedef struct _XipAnnotationRec
  { CorePart		core;
    XmPrimitivePart	primitive;
    XipAnnotationPart	annotation;
  } XipAnnotationRec;

#endif /* _XipAnnotationP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
