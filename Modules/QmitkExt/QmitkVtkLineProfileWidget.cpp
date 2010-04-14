/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-18 21:46:13 +0200 (So, 18 Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkVtkLineProfileWidget.h"

#include "mitkGeometry2D.h"


#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>


#include <vtkQtChartArea.h>
#include <vtkQtChartTableSeriesModel.h>
#include <vtkQtChartStyleManager.h>
#include <vtkQtChartColorStyleGenerator.h>

#include <vtkQtChartMouseSelection.h>
#include <vtkQtChartInteractorSetup.h>
#include <vtkQtChartSeriesSelectionHandler.h>
#include <vtkQtChartAxisLayer.h>
#include <vtkQtChartAxis.h>
#include <vtkQtChartAxisOptions.h>



//#include <iostream>

QmitkVtkLineProfileWidget::QmitkVtkLineProfileWidget( QWidget *parent )
: m_PathMode( PATH_MODE_DIRECT )
{
  m_ChartWidget = new vtkQtChartWidget( this );

  QBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( m_ChartWidget );
  layout->setSpacing( 10 );

  vtkQtChartArea *area = m_ChartWidget->getChartArea();

  // Set up the line chart.
  m_LineChart = new vtkQtLineChart();
  area->insertLayer( area->getAxisLayerIndex(), m_LineChart );

  // Set up the default interactor.
  vtkQtChartMouseSelection *selector =
    vtkQtChartInteractorSetup::createDefault( area );
  vtkQtChartSeriesSelectionHandler *handler =
    new vtkQtChartSeriesSelectionHandler( selector );
  handler->setModeNames( "Bar Chart - Series", "Bar Chart - Bars" );
  handler->setMousePressModifiers( Qt::ControlModifier, Qt::ControlModifier );
  handler->setLayer( m_LineChart );
  selector->addHandler( handler );
  selector->setSelectionMode("Bar Chart - Bars");

  // Hide the x-axis grid.
  vtkQtChartAxisLayer *axisLayer = area->getAxisLayer();
  vtkQtChartAxis *xAxis = axisLayer->getAxis(vtkQtChartAxis::Bottom);
  xAxis->getOptions()->setGridVisible(false);
  xAxis->getOptions()->setPrecision( 1 );
  xAxis->getOptions()->setNotation( vtkQtChartAxisOptions::Standard );
  
  vtkQtChartAxis *yAxis = axisLayer->getAxis(vtkQtChartAxis::Left);
  yAxis->getOptions()->setPrecision( 0 );
  yAxis->getOptions()->setNotation( vtkQtChartAxisOptions::Standard );

  // Set up the model for the bar chart.
  m_ItemModel = new QStandardItemModel( m_LineChart );
  m_ItemModel->setItemPrototype( new QStandardItem() );
  m_ItemModel->setHorizontalHeaderItem( 0, new QStandardItem("Intensity profile") );


  // Initialize parametric path object
  m_ParametricPath = ParametricPathType::New();
}



QmitkVtkLineProfileWidget::~QmitkVtkLineProfileWidget()
{
}


void QmitkVtkLineProfileWidget::SetPathModeToDirectPath()
{
  if ( m_PathMode != PATH_MODE_DIRECT )
  {
    m_PathMode = PATH_MODE_DIRECT;
    this->Modified();
  }
}


void QmitkVtkLineProfileWidget::SetPathModeToPlanarFigure()
{
  if ( m_PathMode != PATH_MODE_PLANARFIGURE )
  {
    m_PathMode = PATH_MODE_PLANARFIGURE;
    this->Modified();
  }
}


void QmitkVtkLineProfileWidget::UpdateItemModelFromPath()
{
  this->ComputePath();

  if ( m_DerivedPath.IsNull() )
  {
    itkExceptionMacro( << "QmitkVtkLineProfileWidget: no path set" );
  }

  // TODO: indices according to mm

  //// Clear the item model
  m_ItemModel->clear();

  MITK_INFO << "Intensity profile (t)";
  MITK_INFO << "Start: " << m_DerivedPath->StartOfInput();
  MITK_INFO << "End:   " << m_DerivedPath->EndOfInput();

  // Get geometry from image
  mitk::Geometry3D *imageGeometry = m_Image->GetGeometry();

  // Fill item model with line profile data
  double distance = 0.0;
  mitk::Point3D currentWorldPoint;

  double t;
  unsigned int i = 0;
  for ( i = 0, t = m_DerivedPath->StartOfInput(); ;++i )
  {
    const PathType::OutputType &continuousIndex = m_DerivedPath->Evaluate( t );
    
    mitk::Point3D indexPoint;
    indexPoint[0] = continuousIndex[0];
    indexPoint[1] = continuousIndex[1];
    indexPoint[2] = continuousIndex[2];

    mitk::Point3D worldPoint;
    imageGeometry->IndexToWorld( indexPoint, worldPoint );

    if ( i == 0 )
    {
      currentWorldPoint = worldPoint;
    }

    distance += currentWorldPoint.EuclideanDistanceTo( worldPoint );
    double intensity = m_Image->GetPixelValueByIndex( indexPoint );

    MITK_INFO << t << "/" << distance << ": " << indexPoint << " (" << intensity << ")";

    m_ItemModel->setVerticalHeaderItem( i, new QStandardItem() );
    m_ItemModel->verticalHeaderItem( i )->setData(
      QVariant( distance ), Qt::DisplayRole );

    m_ItemModel->setItem( i, 0, new QStandardItem() );
    m_ItemModel->item( i, 0 )->setData( intensity, Qt::DisplayRole );

    // Go to next index; when iteration offset reaches zero, iteration is finished
    PathType::OffsetType offset = m_DerivedPath->IncrementInput( t );
    if ( !(offset[0] || offset[1] || offset[2]) )
    {
      break;
    }

    currentWorldPoint = worldPoint;
  }


  vtkQtChartTableSeriesModel *table =
    new vtkQtChartTableSeriesModel( m_ItemModel, m_LineChart );
  m_LineChart->setModel( table );
}


void QmitkVtkLineProfileWidget::ClearItemModel()
{
  m_ItemModel->clear();
}


void QmitkVtkLineProfileWidget::CreatePathFromPlanarFigure()
{
  m_ParametricPath->Initialize();

  if ( m_PlanarFigure.IsNull() )
  {
    itkExceptionMacro( << "QmitkVtkLineProfileWidget: PlanarFigure not set!" );
  }

  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "QmitkVtkLineProfileWidget: Image not set -- needed to calculate path from PlanarFigure!" );
  }

  // Get 2D geometry frame of PlanarFigure
  mitk::Geometry2D *planarFigureGeometry2D = 
    dynamic_cast< mitk::Geometry2D * >( m_PlanarFigure->GetGeometry( 0 ) );
  if ( planarFigureGeometry2D == NULL )
  {
    itkExceptionMacro( << "QmitkVtkLineProfileWidget: PlanarFigure has no valid geometry!" );
  }

  // Get 3D geometry from Image (needed for conversion of point to index)
  mitk::Geometry3D *imageGeometry = m_Image->GetGeometry( 0 );
  if ( imageGeometry == NULL )
  {
    itkExceptionMacro( << "QmitkVtkLineProfileWidget: Image has no valid geometry!" );
  }

  // Get first poly-line of PlanarFigure (other possible poly-lines in PlanarFigure
  // are not supported)
  typedef mitk::PlanarFigure::VertexContainerType VertexContainerType;
  const VertexContainerType *vertexContainer = m_PlanarFigure->GetPolyLine( 0 );

  MITK_INFO << "WorldToIndex:";

  VertexContainerType::ConstIterator it;
  for ( it = vertexContainer->Begin(); it != vertexContainer->End(); ++it )
  {
    // Map PlanarFigure 2D point to 3D point
    mitk::Point3D point3D;
    planarFigureGeometry2D->Map( it->Value(), point3D );

    // Convert world to index coordinates
    mitk::Point3D indexPoint3D;
    imageGeometry->WorldToIndex( point3D, indexPoint3D );

    ParametricPathType::OutputType index;
    index[0] = indexPoint3D[0];
    index[1] = indexPoint3D[1];
    index[2] = indexPoint3D[2];

    MITK_INFO << point3D << " / " << index;

    // Add index to parametric path
    m_ParametricPath->AddVertex( index );
  }

}


void QmitkVtkLineProfileWidget::ComputePath()
{
  switch ( m_PathMode )
  {
  case PATH_MODE_DIRECT:
    {
      m_DerivedPath = m_Path;
      break;
    }

  case PATH_MODE_PLANARFIGURE:
    {
      // Calculate path from PlanarFigure using geometry of specified Image
      this->CreatePathFromPlanarFigure();
      m_DerivedPath = m_ParametricPath;
      break;
    }
  }
}
