
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
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
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
