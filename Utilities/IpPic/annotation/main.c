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

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/Frame.h>

#include <mitkIpPic.h>

#include <pic/Pic.h>

#include "Annotation.h"

/*#include <X11/Editres.h>*/

static String fallback_resources[] = {
#include "ad.h"
NULL
};


static int n;
static Arg args[20];

void activate( Widget w, XtPointer data, XtPointer call_data );

extern void _XEditResCheckMessages();

Widget button1;
Widget button2;

Position x, y;
char* text;

main( int argc, char *argv[] )
{
  XtAppContext app_context;
  Widget toplevel, bb, pic, annotation;
  /*Widget button;*/


  toplevel = XtVaAppInitialize( &app_context,
                                "Test",
                                NULL, 0,
                                &argc, argv,
                                fallback_resources,
                                NULL );

  XtAddEventHandler( toplevel,
                     (EventMask)0,
                     True,_XEditResCheckMessages,
                     NULL );

  n = 0;
  XtSetArg( args[n], XmNheight, 400 ); n++;
  XtSetArg( args[n], XmNwidth, 400 ); n++;

  XtSetValues( toplevel, args, n );

  bb = XmCreateDrawingArea( toplevel, "bb", args, n );
  XtManageChild( bb );


  n = 0;
  XtSetArg( args[n], XmNx, 200 ); n++;
  XtSetArg( args[n], XmNy, 100 ); n++;
  annotation = XtCreateManagedWidget( "annotation",
                                      xipAnnotationWidgetClass,
                                      bb,
                                      args, n );
  XtAddCallback( annotation, XipNactivateCallback, activate, NULL );

  n = 0;
  XtSetArg( args[n], XmNx, 0 ); n++;
  XtSetArg( args[n], XmNy, 0 ); n++;
  XtSetArg( args[n], XipNquantisation, True ); n++;
  XtSetArg( args[n], XipNpic, mitkIpPicGet( "../b.pic", NULL ) ); n++;
  pic = XtCreateManagedWidget( "pic",
                               xipPicWidgetClass,
                               bb,
                               args, n );
/**************/
  XtRealizeWidget( toplevel );
/**************/
  {
    mitkIpPicDescriptor *pic;
    _mitkIpPicTagsElement_t *head;
    mitkIpPicTSV_t *tsv;

    pic = mitkIpPicGetTags( "../b.pic",
                        NULL );
    tsv = mitkIpPicQueryTag( pic, "ANNOTATION" );
    if( tsv != NULL )
      {
        head = tsv->value;

        tsv = _mitkIpPicFindTag( head, "TEXT" )->tsv;
        text = malloc( strlen(tsv->value) );
        strcpy( text, tsv->value );
        printf( "%s\n", text );

        tsv = _mitkIpPicFindTag( head, "POSITION" )->tsv;
        x = ((mitkIpUInt4_t *)(tsv->value))[0];
        y = ((mitkIpUInt4_t *)(tsv->value))[1];
        printf( "%i %i\n", x, y );
      }

    mitkIpPicFree( pic );
  }
/**************/
  XtVaSetValues( annotation,
                 XmNx, x,
                 XmNy, y,
                 NULL );
/**************/
  XtAppMainLoop(app_context);
}

void activate( Widget w, XtPointer data, XtPointer call_data )
{
  static Widget box = 0;

  /*printf( "activated\n" );*/

  if( box == 0 )
    box = XmCreateInformationDialog( w, "info", NULL, 0 );

  XtVaSetValues( box,
                 XmNnoResize, True,
                 XtVaTypedArg,
                   XmNdialogTitle, XmRString, "HELP", sizeof( char *),
                 XtVaTypedArg,
                   XmNmessageString, XmRString, text, sizeof( char *),
                 XtVaTypedArg,
                   XmNokLabelString, XmRString, " Close ", sizeof( char *),
                 NULL );

  XtUnmanageChild( XmMessageBoxGetChild( box, XmDIALOG_CANCEL_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( box, XmDIALOG_HELP_BUTTON ) );

  XtManageChild( box );
}
