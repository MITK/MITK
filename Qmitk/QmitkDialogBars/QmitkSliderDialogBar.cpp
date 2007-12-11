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

#include "QmitkSliderDialogBar.h"
#include "QmitkSliderNavigator.h"
#include "QmitkStepperAdapter.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSliderDialogBar.xpm"

#include <qaction.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qgrid.h>

QmitkSliderDialogBar
::QmitkSliderDialogBar( 
  QObject *parent, const char *name, QmitkStdMultiWidget *multiWidget, 
  mitk::DataTreeIteratorBase *dataIt )
: QmitkDialogBar( "Slicer", parent, name, multiWidget, dataIt )
{
}

QmitkSliderDialogBar
::~QmitkSliderDialogBar()
{
}


QString 
QmitkSliderDialogBar
::GetFunctionalityName()
{
  return "SliderDialogBar";
}


QAction *
QmitkSliderDialogBar
::CreateAction( QObject *parent )
{
  QAction* action;
  action = new QAction( 
    tr( "Slicer" ), 
    QPixmap((const char**)QmitkSliderDialogBar_xpm), 
    tr( "Slicer" ), 
    0, 
    parent, 
    "QmitkSliderDialogBar" );

  return action;
}

QWidget *
QmitkSliderDialogBar
::CreateDialogBar( QWidget *parent )
{
  QGrid *grid = new QGrid( 2, Qt::Horizontal, parent );

  new QLabel( "Transversal", grid );
  QmitkSliderNavigator *sliderNavigator1 = new QmitkSliderNavigator( grid );

  new QLabel( "Sagittal", grid );
  QmitkSliderNavigator *sliderNavigator2 = new QmitkSliderNavigator( grid );

  new QLabel( "Coronal", grid );
  QmitkSliderNavigator *sliderNavigator3 = new QmitkSliderNavigator( grid );

  new QLabel( "Time", grid );
  QmitkSliderNavigator *sliderNavigatorTime = new QmitkSliderNavigator( grid );

  sliderNavigator1->ShowLabels( true );
  sliderNavigator2->ShowLabels( true );
  sliderNavigator3->ShowLabels( true );
  sliderNavigatorTime->ShowLabels( true );

  new QmitkStepperAdapter(
    sliderNavigator1, 
    m_MultiWidget->GetRenderWindow1()->GetController()->GetSlice(),
    "SpatialStepperAdaptor1"
  );

  new QmitkStepperAdapter(
    sliderNavigator2, 
    m_MultiWidget->GetRenderWindow2()->GetController()->GetSlice(),
    "SpatialStepperAdaptor2"
  );

  new QmitkStepperAdapter(
    sliderNavigator3, 
    m_MultiWidget->GetRenderWindow3()->GetController()->GetSlice(),
    "SpatialStepperAdaptor3"
  );

  new QmitkStepperAdapter(
    sliderNavigatorTime, 
    m_MultiWidget->GetTimeNavigationController()->GetTime(),
    "TimeStepperAdaptor"
  );

  return grid;
}
