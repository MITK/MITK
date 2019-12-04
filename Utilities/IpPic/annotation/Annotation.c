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
 * Annotation.c
 *---------------------------------------------------------------
 * DESCRIPTION
 *   the annotation widget methods
 *
 * Author
 *   a.schroeter
 *
 *---------------------------------------------------------------
 *
 */
#ifndef lint
  static char *what = { "@(#)XipAnnotationWidget\tGerman Cancer Research Center (DKFZ)\t1993/09/17" };
#endif


#include <X11/StringDefs.h>
#include <X11/cursorfont.h>

#include "AnnotationP.h"
#include "icon_text.xbm"

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

static void    Initialize();
static void    Realize();
static void    Redisplay();
static void    Resize();
static XtGeometryResult QueryGeometry();
static void    Destroy();
static Boolean SetValues();

static void    activate();

static void getDrawGC();


static XtResource resources[] =
{
  { XipNactivateCallback, XtCCallback, XtRCallback,
    sizeof(XtPointer),
    XtOffset (XipAnnotationWidget, annotation.activate_CB),
    XtRCallback, NULL },

  /* change parent class defaults */
  { XmNtopShadowColor, XmCTopShadowColor, XmRPixel,
    sizeof(Pixel),
    XtOffset(XipAnnotationWidget, primitive.top_shadow_color),
    XtRString, "black" },

  { XmNbottomShadowColor, XmCBottomShadowColor, XmRPixel,
    sizeof(Pixel),
    XtOffset(XipAnnotationWidget, primitive.bottom_shadow_color),
    XtRString, "black" },

  { XmNbackground, XmCBackground, XmRPixel,
    sizeof(Pixel),
    XtOffset(XipAnnotationWidget, core.background_pixel),
    XtRString, "Yellow" },

  { XmNtraversalOn, XmCTraversalOn, XmRBoolean,
    sizeof(Boolean),
    XtOffset (XipAnnotationWidget, primitive.traversal_on),
    XmRImmediate, (XtPointer)False },
};

static char defaultTranslations[] =
" <Btn1Down>:   activate()\n\
";

static XtActionsRec actions[] =
{
  { "activate",		(XtActionProc)activate },
};

XipAnnotationClassRec xipAnnotationClassRec =
{
  { /* core_class fields */
    (WidgetClass)&xmPrimitiveClassRec,	/* superclass            */
    "XipAnnotation",			/* class_name            */
    sizeof(XipAnnotationRec),		/* widget_size           */
    NULL,				/* class_initialize      */
    NULL,				/* class_part_initialize */
    False,				/* class_inited          */
    Initialize,				/* initialize            */
    NULL,				/* initialize_hook       */
    Realize,				/* realize               */
    /*XtInheritRealize,			/* realize               */
    actions,				/* actions               */
    XtNumber(actions),			/* num_actions           */
    resources,				/* resources             */
    XtNumber(resources),		/* num_resources         */
    NULLQUARK,				/* xrm_class             */
    True,				/* compress_motion       */
    False,				/* compress_exposure     */
    True,				/* compress_enterleave   */
    False,				/* visible_interest      */
    Destroy,				/* destroy               */
    Resize,				/* resize                */
    Redisplay,				/* expose                */
    SetValues,				/* set_values            */
    NULL,				/* set_values_hook       */
    XtInheritSetValuesAlmost,		/* set_values_almost     */
    NULL,				/* get_values_hook       */
    NULL,				/* accept_focus          */
    XtVersion,				/* version               */
    NULL,				/* callback private      */
    defaultTranslations,		/* tm_table              */
    QueryGeometry,			/* query_geometry        */
    XtInheritDisplayAccelerator,	/* display_accelerator   */
    NULL,				/* extension             */
  },
  { /* primitive_class fields */
    (XtWidgetProc)_XtInherit,		/* border highlight	*/
    (XtWidgetProc)_XtInherit,		/* border unhiglight	*/
    XtInheritTranslations,              /* translations		*/
    NULL,				/* arm_and_activate	*/
    NULL,				/* syn resources	*/
    0,					/* num_sysresources	*/
    NULL,				/* extensions		*/
  },
  { /* annotation_class fields */
    0,					/* extension		*/
  },
};

WidgetClass xipAnnotationWidgetClass = (WidgetClass) &xipAnnotationClassRec;


static void Initialize( request, new, args, num_args )
  XipAnnotationWidget request, new;
  ArgList args;
  Cardinal *num_args;
{
  /*printf( "init\n" );*/

  getDrawGC( new );

  new->annotation.cursor = XCreateFontCursor( XtDisplay(new),
                                              XC_hand1 );

  new->annotation.icon_text = 0;

  if( request->core.width == 0 )
    new->core.width = 25;
  if( request->core.height == 0 )
    new->core.height = 30;
}

static void getDrawGC( w )
  XipAnnotationWidget w;
{
  XGCValues values;
  XtGCMask  valueMask;

  values.foreground = w->primitive.foreground;
  values.background = w->core.background_pixel;
  valueMask = GCForeground | GCBackground;

  w->annotation.draw_GC = XtGetGC ( (Widget)w, valueMask, &values );
}

static void Realize( w, valueMask, attributes )
  XipAnnotationWidget w;
  Mask *valueMask;
  XSetWindowAttributes *attributes;
{
  /*(*xipAnnotationClassRec.core_class.superclass->core_class.realize)
    (w, valueMask, attributes);*/

  XtCreateWindow( (Widget)w,
                  InputOutput,
                  CopyFromParent,
                  *valueMask,
                  attributes);

#ifdef SHAPE
  {
    GC shape_gc;
    Pixmap shape_mask;
    XGCValues xgcv;

    shape_mask = XCreatePixmap( XtDisplay(w),
                                XtWindow(w),
                                w->core.width,
                                w->core.height,
                                1 );
    shape_gc = XCreateGC( XtDisplay(w),
                          shape_mask,
                          0,
                          &xgcv);

    XSetForeground( XtDisplay(w),
                    shape_gc,
                    0);
    XFillRectangle( XtDisplay(w),
                    shape_mask,
                    shape_gc,
                    0, 0,
                    w->core.width, w->core.height );
    XSetForeground( XtDisplay(w),
                    shape_gc,
                    1);
    XFillRectangle( XtDisplay(w),
                    shape_mask,
                    shape_gc,
                    1, 1,
                    w->core.width-4, w->core.height-4 );
    XSetForeground( XtDisplay(w),
                    shape_gc,
                    0);
    {
      int i;

      for( i = 0; i < w->core.width/6; i++ )
        XDrawLine( XtDisplay(w),
                   shape_mask,
                   shape_gc,
                   w->core.width-4-i, w->core.height-4,
                   w->core.width-4, w->core.height-4-i );

      XDrawLine( XtDisplay(w),
                 shape_mask,
                 shape_gc,
                 w->core.width-4-i, w->core.height-4-i,
                 w->core.width-4-i, w->core.height-4 );
      XDrawLine( XtDisplay(w),
                 shape_mask,
                 shape_gc,
                 w->core.width-4-i, w->core.height-4-i,
                 w->core.width-4, w->core.height-4-i );
    }
    XShapeCombineMask( XtDisplay(w),
                       XtWindow(w),
                       ShapeClip,
                       0, 0,
                       shape_mask,
                       ShapeSet);

    XSetForeground( XtDisplay(w),
                    shape_gc,
                    0);
    XFillRectangle( XtDisplay(w),
                    shape_mask,
                    shape_gc,
                    0, 0,
                    w->core.width, w->core.height );
    XSetForeground( XtDisplay(w),
                    shape_gc,
                    1);
    XFillRectangle( XtDisplay(w),
                    shape_mask,
                    shape_gc,
                    0, 0,
                    w->core.width-2, w->core.height-2 );
    XFillRectangle( XtDisplay(w),
                    shape_mask,
                    shape_gc,
                    2, 2,
                    w->core.width-2, w->core.height-2 );
    XSetForeground( XtDisplay(w),
                    shape_gc,
                    0);
    {
      int i;

      for( i = 0; i < w->core.width/6; i++ )
        XDrawLine( XtDisplay(w),
                   shape_mask,
                   shape_gc,
                   w->core.width-1-i, w->core.height-1,
                   w->core.width-1, w->core.height-1-i );
    }
    XShapeCombineMask( XtDisplay(w),
                       XtWindow(w),
                       ShapeBounding,
                       0, 0,
                       shape_mask,
                       ShapeSet);

    XFreePixmap( XtDisplay(w),
                 shape_mask );
  }
#endif

  XDefineCursor( XtDisplay(w),
                 XtWindow(w),
                 w->annotation.cursor );
}

static XtGeometryResult QueryGeometry( w, proposed, answer )
  XipAnnotationWidget w;
  XtWidgetGeometry *proposed;
  XtWidgetGeometry *answer;
{

  answer->width = (Dimension)256;
  answer->height = (Dimension)120;
  answer->request_mode = CWWidth || CWHeight;

  return( XtGeometryAlmost );
}

static void Destroy( w )
  XipAnnotationWidget w;
{
  XtReleaseGC( (Widget)w, w->annotation.draw_GC );

  XtRemoveAllCallbacks ( (Widget)w, XipNactivateCallback );
}

static void Resize( w )
  XipAnnotationWidget w;
{
  /*if( w->core.width > w->annotation.image->width
      || w->core.height > w->annotation.image->height )
    XtWarning( "XipAnnotationWidget resized" );*/

}


static void Redisplay( w, event, region)
  XipAnnotationWidget w;
  XExposeEvent *event;
  Region region;
{
  if( w->core.visible )
    {
      if( w->annotation.icon_text == 0 )
        w->annotation.icon_text = XCreateBitmapFromData( XtDisplay(w),
                                                         XtWindow(w),
                                                         icon_text_bits,
                                                         icon_text_width,
                                                         icon_text_height );

      XCopyPlane( XtDisplay(w),
                  w->annotation.icon_text,
                  XtWindow(w),
                  w->annotation.draw_GC,
                  0, 0,
                  icon_text_width, icon_text_height,
                  0, 0,
                  1 );
      /*XDrawLine( XtDisplay(w),
                 XtWindow(w),
                 w->annotation.draw_GC,
                 0, 0,
                 200, 200 );*/
    }
}

static Boolean SetValues( current, request, new, args, num_args )
     XipAnnotationWidget current, request, new;
     ArgList args;
     Cardinal *num_args;
{
  Cardinal i;

  Boolean    redraw = False;

  /*printf( "setv\n" );*/

  return( redraw );
}

static void activate( w, event )
  XipAnnotationWidget w;
  XEvent *event;
{
  if( event->xbutton.x < w->core.width
      && event->xbutton.y < w->core.height )
    XtCallCallbacks( (Widget)w,
                     XipNactivateCallback,
                     (XtPointer) event );
}
