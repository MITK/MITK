/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkNavigationDataPlayerView.h"
#include "QmitkStdMultiWidget.h"

//mitk
#include <mitkCone.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// Qt
#include <QMessageBox>

// vtk
#include <vtkConeSource.h>

const std::string QmitkNavigationDataPlayerView::VIEW_ID = "org.mitk.views.navigationdataplayer";

QmitkNavigationDataPlayerView::QmitkNavigationDataPlayerView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
  , m_Trajectory( NULL )
  , m_TrajectoryIndex( -1 )
  , m_ReloadData( true )
  , m_ShowTrajectory( false )
  , m_SplineMapper( NULL )
  , m_PointSetMapper( NULL )
{
  m_TrajectoryPointSet = mitk::PointSet::New(); // PointSet for trajectory points
}

QmitkNavigationDataPlayerView::~QmitkNavigationDataPlayerView()
{
}

void QmitkNavigationDataPlayerView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkNavigationDataPlayerViewControls;
    m_Controls->setupUi( parent );

    this->CreateBundleWidgets( parent );
    this->CreateConnections();
  }
}

void QmitkNavigationDataPlayerView::CreateBundleWidgets(QWidget* parent)
{
  //m_PlayerWidget = new QmitkIGTPlayerWidget( parent );   // this bundle's ND player widget
}

void QmitkNavigationDataPlayerView::CreateConnections()
{
  //connect( m_PlayerWidget, SIGNAL(SignalPlayingStarted()), this, SLOT(OnCreatePlaybackVisualization()) );
}

void QmitkNavigationDataPlayerView::OnPlayingStarted()
{
}

mitk::Color QmitkNavigationDataPlayerView::GetColorCircleColor(int index)
{
  mitk::Color result;

  mitk::ColorSequenceCycleH colorCycle;

  for(int i=0; i <= index; ++i)
  {
    result = colorCycle.GetNextColor();
  }

  return result;
}