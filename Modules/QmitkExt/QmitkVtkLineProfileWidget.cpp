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

#include "QmitkVtkLineProfileWidget.h"

#include "mitkGeometry2D.h"


#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>
#include <QBrush>


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
#include <vtkQtBarChartOptions.h>
#include <vtkQtChartSeriesOptions.h>
#include <vtkQtChartColors.h>
#include <vtkConfigure.h>

#if ((VTK_MAJOR_VERSION<=5) && (VTK_MINOR_VERSION<=4) )
#include <vtkQtChartPenBrushGenerator.h>
#endif

#if ((VTK_MAJOR_VERSION>=5) && (VTK_MINOR_VERSION>=6) )
#include <vtkQtChartPenGenerator.h>
#include <vtkQtChartBasicStyleManager.h>
#endif

//#include <iostream>

QmitkVtkLineProfileWidget::QmitkVtkLineProfileWidget( QWidget * parent )
  : QDialog(parent), m_PathMode( PATH_MODE_DIRECT )
{
  m_ChartWidget = new vtkQtChartWidget( this );

  QBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( m_ChartWidget );
  layout->setSpacing( 10 );

  vtkQtChartArea *area = m_ChartWidget->getChartArea();

  // Set up the line chart.
  m_LineChart = new vtkQtLineChart();
  area->insertLayer( area->getAxisLayerIndex(), m_LineChart );
  //m_BarChart->getOptions()->setBarGroupFraction(10);

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

#if ((VTK_MAJOR_VERSION<=5) && (VTK_MINOR_VERSION<=4) )
  vtkQtChartStyleManager *styleManager = area->getStyleManager();
  vtkQtChartPenBrushGenerator *pen = new vtkQtChartPenBrushGenerator();
  pen->setPen(0,QPen(Qt::SolidLine));
  pen->addPens(vtkQtChartColors::WildFlower);
  styleManager->setGenerator(pen);
#endif

#if ((VTK_MAJOR_VERSION>=5) && (VTK_MINOR_VERSION>=6) )

  vtkQtChartBasicStyleManager *styleManager =
      qobject_cast<vtkQtChartBasicStyleManager *>(area->getStyleManager());

  vtkQtChartPenGenerator *pen = new vtkQtChartPenGenerator();
  pen->setPen(0,QPen(Qt::SolidLine));
  pen->addPens(vtkQtChartColors::WildFlower);
  styleManager->setGenerator("Pen",pen);

#endif



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
  }
}


void QmitkVtkLineProfileWidget::SetPathModeToPlanarFigure()
{
  if ( m_PathMode != PATH_MODE_PLANARFIGURE )
  {
    m_PathMode = PATH_MODE_PLANARFIGURE;
  }
}


void QmitkVtkLineProfileWidget::UpdateItemModelFromPath()
{
  this->ComputePath();

  if ( m_DerivedPath.IsNull() )
  {
    throw std::invalid_argument("QmitkVtkLineProfileWidget: no path set");
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
  int t_tmp = 0;
  QStandardItemModel *tmp_ItemModel = new QStandardItemModel();
  vtkQtChartTableSeriesModel *table;
  vtkQtChartArea* area = m_ChartWidget->getChartArea();
  for(unsigned int j = 0; j < m_VectorLineCharts.size(); j++)
  {
      area->removeLayer(m_VectorLineCharts[j]);
      m_VectorLineCharts[j]->getModel()->deleteLater();
      m_VectorLineCharts[j]->deleteLater();
  }
  m_VectorLineCharts.clear();

  int k = 0;
  for ( i = 0, t = m_DerivedPath->StartOfInput(); ;++i )
  {
    const PathType::OutputType &continuousIndex = m_DerivedPath->Evaluate( t );

    mitk::Point3D worldPoint;
    imageGeometry->IndexToWorld( continuousIndex, worldPoint );

    if ( i == 0 )
    {
      currentWorldPoint = worldPoint;
    }

    distance += currentWorldPoint.EuclideanDistanceTo( worldPoint );

    mitk::Index3D indexPoint;
    imageGeometry->WorldToIndex( worldPoint, indexPoint );
    double intensity = m_Image->GetPixelValueByIndex( indexPoint );

    MITK_INFO << t << "/" << distance << ": " << indexPoint << " (" << intensity << ")";

    m_ItemModel->setVerticalHeaderItem( i, new QStandardItem() );
    m_ItemModel->verticalHeaderItem( i )->setData(
      QVariant( distance ), Qt::DisplayRole );

    m_ItemModel->setItem( i, 0, new QStandardItem() );
    m_ItemModel->item( i, 0 )->setData( intensity, Qt::DisplayRole );

    tmp_ItemModel->setVerticalHeaderItem( k, new QStandardItem() );
    tmp_ItemModel->verticalHeaderItem( k )->setData(
      QVariant( distance ), Qt::DisplayRole );

    tmp_ItemModel->setItem( k, 0, new QStandardItem() );
    tmp_ItemModel->item( k, 0 )->setData( intensity, Qt::DisplayRole );

    if ((int)t > t_tmp){
        t_tmp = (int)t;

        vtkQtLineChart *tmp_LineChart = new vtkQtLineChart();
        table = new vtkQtChartTableSeriesModel( tmp_ItemModel, tmp_LineChart );
        tmp_LineChart->setModel( table );

        m_VectorLineCharts.push_back(tmp_LineChart);

        tmp_ItemModel = new QStandardItemModel();

        k = 0;

        tmp_ItemModel->setVerticalHeaderItem( k, new QStandardItem() );
        tmp_ItemModel->verticalHeaderItem( k )->setData(
            QVariant( distance ), Qt::DisplayRole );

        tmp_ItemModel->setItem( k, 0, new QStandardItem() );
        tmp_ItemModel->item( k, 0 )->setData( intensity, Qt::DisplayRole );
    }
    k++;

    // Go to next index; when iteration offset reaches zero, iteration is finished
    PathType::OffsetType offset = m_DerivedPath->IncrementInput( t );
    if ( !(offset[0] || offset[1] || offset[2]) )
    {
      break;
    }

    currentWorldPoint = worldPoint;
  }
  for(unsigned int j = 0; j < m_VectorLineCharts.size() ; j++)
  {
/*    int styleIndex = styleManager->getStyleIndex(m_LineChart, m_LineChart->getSeriesOptions(0));

    vtkQtChartStylePen *stylePen = qobject_cast<vtkQtChartStylePen *>(
      styleManager->getGenerator("Pen"));
    stylePen->getStylePen(styleIndex).setStyle(Qt::SolidLine);*/
    area->insertLayer(area->getAxisLayerIndex() + j +1, m_VectorLineCharts[j]);
  }

  table =
    new vtkQtChartTableSeriesModel( m_ItemModel, m_LineChart );
  //m_LineChart->setModel( table );
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
    throw std::invalid_argument("QmitkVtkLineProfileWidget: PlanarFigure not set!" );
  }

  if ( m_Image.IsNull() )
  {
    throw std::invalid_argument("QmitkVtkLineProfileWidget: Image not set -- needed to calculate path from PlanarFigure!" );
  }

  // Get 2D geometry frame of PlanarFigure
  mitk::Geometry2D *planarFigureGeometry2D =
    dynamic_cast< mitk::Geometry2D * >( m_PlanarFigure->GetGeometry( 0 ) );
  if ( planarFigureGeometry2D == NULL )
  {
    throw std::invalid_argument("QmitkVtkLineProfileWidget: PlanarFigure has no valid geometry!" );
  }

  // Get 3D geometry from Image (needed for conversion of point to index)
  mitk::Geometry3D *imageGeometry = m_Image->GetGeometry( 0 );
  if ( imageGeometry == NULL )
  {
    throw std::invalid_argument("QmitkVtkLineProfileWidget: Image has no valid geometry!" );
  }

  // Get first poly-line of PlanarFigure (other possible poly-lines in PlanarFigure
  // are not supported)
  typedef mitk::PlanarFigure::PolyLineType VertexContainerType;
  const VertexContainerType vertexContainer = m_PlanarFigure->GetPolyLine( 0 );

  MITK_INFO << "WorldToIndex:";

  VertexContainerType::const_iterator it;
  for ( it = vertexContainer.begin(); it != vertexContainer.end(); ++it )
  {
    // Map PlanarFigure 2D point to 3D point
    mitk::Point3D point3D;
    planarFigureGeometry2D->Map( it->Point, point3D );

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

void QmitkVtkLineProfileWidget::SetImage( mitk::Image* image )
{
  m_Image = image;
}

void QmitkVtkLineProfileWidget::SetPath( const PathType* path )
{
  m_Path = path;
}

void QmitkVtkLineProfileWidget::SetPlanarFigure( const mitk::PlanarFigure* planarFigure )
{
  m_PlanarFigure = planarFigure;
}

void QmitkVtkLineProfileWidget::SetPathMode( unsigned int pathMode )
{
  m_PathMode = pathMode;
}

unsigned int QmitkVtkLineProfileWidget::GetPathMode()
{
  return m_PathMode;
}
