/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkHelpBrowser.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qstylesheet.h>

#include "back.xpm"
#include "forward.xpm"
#include "home.xpm"

#include <ctype.h>

QmitkHelpBrowser::QmitkHelpBrowser( const QString& home_, const QString& _path,
                                    QWidget* parent, const char *name )
: QMainWindow( parent, name, WDestructiveClose ),
  pathCombo( 0 )
{
    browser = new QTextBrowser( this );

    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( sourceChanged(const QString& ) ),
       this, SLOT( sourceChanged( const QString&) ) );

    browser->setCurrentFont( QFont("SansSerif", 12) );
    browser->zoomIn();
    browser->zoomIn();
    
    QStyleSheet* style = browser->styleSheet();
    QColor lightBlue(24, 65, 173);
    QColor linkBlue(26, 65, 157);
    if (style)
    {
      QStyleSheetItem* styleItem;
      styleItem = style->item("h1"); 
        styleItem->setAlignment( Qt::AlignCenter );
        styleItem->setColor( lightBlue );
      styleItem = style->item("h2"); 
        styleItem->setColor( lightBlue );
      styleItem = style->item("h1"); 
        styleItem->setColor( lightBlue );
      styleItem = style->item("div"); 
        styleItem->setLogicalFontSizeStep( styleItem->logicalFontSizeStep() - 2 );
      styleItem = style->item("a"); 
        styleItem->setColor( linkBlue );
     }
    
    setCentralWidget( browser );

    if ( !home_.isEmpty() )
      browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
       statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&Print"), this, SLOT( print() ), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ), CTRL+Key_Q );

    // The same three icons are used twice each.
    QIconSet icon_back( QPixmap( (const char**)back_xpm ) );
    QIconSet icon_forward( QPixmap( (const char**)forward_xpm ) );
    QIconSet icon_home( QPixmap( (const char**)home_xpm ) );

    QPopupMenu* go = new QPopupMenu( this );
    backwardId = go->insertItem( icon_back,
         tr("&Backward"), browser, SLOT( backward() ),
         CTRL+Key_Left );
    forwardId = go->insertItem( icon_forward,
        tr("&Forward"), browser, SLOT( forward() ),
        CTRL+Key_Right );
    go->insertItem( icon_home, tr("&Home"), browser, SLOT( home() ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
       this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
       this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = new QToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( icon_back, tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_forward, tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_home, tr("Home"), "", browser, SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new QComboBox( TRUE, toolbar );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
       this, SLOT( pathSelected( const QString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( TRUE );
    setDockEnabled( DockLeft, FALSE );
    setDockEnabled( DockRight, FALSE );

    pathCombo->insertItem( home_ );
    browser->setFocus();

}


void QmitkHelpBrowser::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void QmitkHelpBrowser::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}

void QmitkHelpBrowser::sourceChanged( const QString& url )
{
    if ( browser->documentTitle().isNull() )
  setCaption( "MITK Help - " + url );
    else
  setCaption( "MITK Help - " + browser->documentTitle() ) ;

    if ( !url.isEmpty() && pathCombo ) {
  bool exists = FALSE;
  int i;
  for ( i = 0; i < pathCombo->count(); ++i ) {
      if ( pathCombo->text( i ) == url ) {
    exists = TRUE;
    break;
      }
  }
  if ( !exists ) {
      pathCombo->insertItem( url, 0 );
      pathCombo->setCurrentItem( 0 );
  } else
      pathCombo->setCurrentItem( i );
    }
}

QmitkHelpBrowser::~QmitkHelpBrowser()
{
}

void QmitkHelpBrowser::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) ) {
  QPainter p( &printer );
  if( !p.isActive() ) // starting printing failed
      return;
  QPaintDeviceMetrics metrics(p.device());
  int dpiy = metrics.logicalDpiY();
  int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
  QRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
  QSimpleRichText richText( browser->text(),
          QFont(),
          browser->context(),
          browser->styleSheet(),
          browser->mimeSourceFactory(),
          view.height() );
  richText.setWidth( &p, view.width() );
  int page = 1;
  do {
      richText.draw( &p, margin, margin, view, colorGroup() );
      view.moveBy( 0, view.height() );
      p.translate( 0 , -view.height() );
      p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
      view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
      if ( view.top() - margin >= richText.height() )
    break;
      printer.newPage();
      page++;
  } while (TRUE);
    }
#endif
}

void QmitkHelpBrowser::pathSelected( const QString &_path )
{
    browser->setSource( _path );
}

