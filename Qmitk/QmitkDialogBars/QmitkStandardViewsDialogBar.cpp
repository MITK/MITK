/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkStandardViewsDialogBar.h"
#include "QmitkStdMultiWidget.h"

#include "QmitkStandardViews.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkStandardViewsDialogBar.xpm"

#include <qaction.h>
#include <qlabel.h>
#include <qvbox.h>

QmitkStandardViewsDialogBar
::QmitkStandardViewsDialogBar( 
  QObject *parent, const char *name, QmitkStdMultiWidget *multiWidget, 
  mitk::DataTreeIteratorBase *dataIt )
: QmitkDialogBar( "Standard views", parent, name, multiWidget, dataIt )
{
}

QmitkStandardViewsDialogBar
::~QmitkStandardViewsDialogBar()
{
}


QString 
QmitkStandardViewsDialogBar
::GetFunctionalityName()
{
  return "StandardViewsDialogBar";
}


QAction *
QmitkStandardViewsDialogBar
::CreateAction( QObject *parent )
{
  QAction* action;
  action = new QAction( 
    tr( "Standard views" ), 
    QPixmap((const char**)QmitkStandardViewsDialogBar_xpm), 
    tr( "Standard views" ), 
    0, 
    parent, 
    "QmitkStandardViewsDialogBar" );

  return action;
}

QWidget *
QmitkStandardViewsDialogBar
::CreateDialogBar( QWidget *parent )
{
  QVBox* layout = new QVBox( parent );

  QmitkStandardViews* m_StandardViews = new QmitkStandardViews( layout, "StandardViewsDialogBar" );
  //m_StandardViews->setMinimumSize( QSize( 0, 100 ) );
    
  if (m_MultiWidget)
  {
    m_StandardViews->SetCameraControllerFromRenderWindow( m_MultiWidget->mitkWidget4->GetRenderWindow() );
  }
  else
  {
    std::cerr << "No StdMultiWidget known to QmitkStandardViewsDialogBar, widget will not work. File " __FILE__ ", l. " << __LINE__ << std::endl;
  }

  return layout;
}
