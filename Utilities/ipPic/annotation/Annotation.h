
/*
 * Annotation.h
 *---------------------------------------------------------------
 * DESCRIPTION
 *   public header for the annotation widget
 *
 * Author
 *   a.schroeter
 *
 *---------------------------------------------------------------
 *  COPYRIGHT (c) 1993 by DKFZ (Dept. MBI) Heidelberg, FRG
 */

#ifndef  _XipAnnotationWidget_h
#define  _XipAnnotationWidget_h

extern WidgetClass xipAnnotationWidgetClass;

typedef struct _XipAnnotationClassRec *XipAnnotationWidgetClass;
typedef struct _XipAnnotationRec      *XipAnnotationWidget;

#ifndef XipIsAnnotation
#define XipIsAnnotation(w) XtIsSubclass(w, xipAnnotationWidgetClass)
#endif

/*
** resource strings 
*/
#define XipNactivateCallback	XmNactivateCallback

#define XipCActivateCallback	XmCActivateCallback

/*
** new data types
*/


#endif /* _XipAnnotationWidget_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
