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
  XtSetArg( args[n], XipNpic, ipPicGet( "../b.pic", NULL ) ); n++;
  pic = XtCreateManagedWidget( "pic",
                               xipPicWidgetClass,
                               bb,
                               args, n );
/**************/
  XtRealizeWidget( toplevel );
/**************/
  {
    ipPicDescriptor *pic;
    _ipPicTagsElement_t *head;
    ipPicTSV_t *tsv;

    pic = ipPicGetTags( "../b.pic",
                        NULL );
    tsv = ipPicQueryTag( pic, "ANNOTATION" );
    if( tsv != NULL )
      {
        head = tsv->value;

        tsv = _ipPicFindTag( head, "TEXT" )->tsv;
        text = malloc( strlen(tsv->value) );
        strcpy( text, tsv->value );
        printf( "%s\n", text );

        tsv = _ipPicFindTag( head, "POSITION" )->tsv;
        x = ((ipUInt4_t *)(tsv->value))[0];
        y = ((ipUInt4_t *)(tsv->value))[1];
        printf( "%i %i\n", x, y );
      }

    ipPicFree( pic );
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
