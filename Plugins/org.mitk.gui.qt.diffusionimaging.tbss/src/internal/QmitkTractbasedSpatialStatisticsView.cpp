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

// Qmitk
#include "QmitkTractbasedSpatialStatisticsView.h"

#include "mitkDataNodeObject.h"
#include <itkCastImageFilter.h>

// Qt
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>

#include <qwt_plot_picker.h>

#include <mitkTractAnalyzer.h>
#include <mitkTbssImporter.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarFigureInteractor.h>


#include "vtkPoints.h"
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
#include <mitkWorkbenchUtil.h>


const std::string QmitkTractbasedSpatialStatisticsView::VIEW_ID = "org.mitk.views.tractbasedspatialstatistics";

using namespace berry;


QmitkTractbasedSpatialStatisticsView::QmitkTractbasedSpatialStatisticsView()
: QmitkAbstractView()
, m_Controls( 0 )
, m_Activated(false)
{

}

QmitkTractbasedSpatialStatisticsView::~QmitkTractbasedSpatialStatisticsView()
{
}


void QmitkTractbasedSpatialStatisticsView::PerformChange()
{
  m_Controls->m_RoiPlotWidget->ModifyPlot(m_Controls->m_Segments->value(), m_Controls->m_Average->isChecked());
}

void QmitkTractbasedSpatialStatisticsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  //datamanager selection changed
  if (!this->IsActivated())
    return;


  // Check which datatypes are selected in the datamanager and enable/disable widgets accordingly

  bool foundTbssRoi = false;
  bool foundTbss = false;
  bool found3dImage = false;
  bool found4dImage = false;
  bool foundFiberBundle = false;
  bool foundStartRoi = false;
  bool foundEndRoi = false;

  mitk::TbssRoiImage* roiImage = nullptr;
  mitk::TbssImage* image = nullptr;
  mitk::Image* img = nullptr;
  mitk::FiberBundle* fib = nullptr;
  mitk::DataNode* start = nullptr;
  mitk::DataNode* end = nullptr;

  m_CurrentStartRoi = nullptr;
  m_CurrentEndRoi = nullptr;



  for (mitk::DataNode::Pointer node: nodes)
  {
    // only look at interesting types
    // check for valid data
    mitk::BaseData* nodeData = node->GetData();
    if( nodeData )
    {
      if(QString("TbssRoiImage").compare(nodeData->GetNameOfClass())==0)
      {
        foundTbssRoi = true;
        roiImage = static_cast<mitk::TbssRoiImage*>(nodeData);
      }
      else if (QString("TbssImage").compare(nodeData->GetNameOfClass())==0)
      {
        foundTbss = true;
        image = static_cast<mitk::TbssImage*>(nodeData);
      }
      else if(QString("Image").compare(nodeData->GetNameOfClass())==0)
      {
        img = static_cast<mitk::Image*>(nodeData);
        if(img->GetDimension() == 3)
        {
          found3dImage = true;
        }
        else if(img->GetDimension() == 4)
        {
          found4dImage = true;
        }
      }

      else if (QString("FiberBundle").compare(nodeData->GetNameOfClass())==0)
      {
        foundFiberBundle = true;
        fib = static_cast<mitk::FiberBundle*>(nodeData);
        this->m_CurrentFiberNode = node;
      }


      if(QString("PlanarCircle").compare(nodeData->GetNameOfClass())==0)
      {
        if(!foundStartRoi)
        {
          start = node;
          this->m_CurrentStartRoi = node;
          foundStartRoi =  true;
        }
        else
        {
          end = node;
          this->m_CurrentEndRoi = node;
          foundEndRoi = true;
        }
      }
    }
  }


  this->m_Controls->m_CreateRoi->setEnabled(found3dImage);
  this->m_Controls->m_ImportFsl->setEnabled(found4dImage);



  if(foundTbss && foundTbssRoi)
  {
    this->Plot(image, roiImage);
  }

  else if(found3dImage && foundFiberBundle && foundStartRoi && foundEndRoi)
  {
    this->PlotFiberBundle(fib, img, start, end);
  }

  else if(found3dImage && foundFiberBundle)
  {
    this->PlotFiberBundle(fib, img);
  }

  else if(foundTbss && foundStartRoi && foundEndRoi && foundFiberBundle)
  {
    this->PlotFiber4D(image, fib, start, end);
  }

  if(found3dImage)
  {
    this->InitPointsets();
  }

  this->m_Controls->m_Cut->setEnabled(foundFiberBundle && foundStartRoi && foundEndRoi);
  this->m_Controls->m_SegmentLabel->setEnabled(foundFiberBundle && foundStartRoi && foundEndRoi && (found3dImage || foundTbss));
  this->m_Controls->m_Segments->setEnabled(foundFiberBundle && foundStartRoi && foundEndRoi && (found3dImage || foundTbss));
  this->m_Controls->m_Average->setEnabled(foundFiberBundle && foundStartRoi && foundEndRoi && found3dImage);

}

void QmitkTractbasedSpatialStatisticsView::InitPointsets()
{
  // Check if PointSetStart exsits, if not create it.
  m_P1 = this->GetDataStorage()->GetNamedNode("PointSetNode");

  if (m_PointSetNode)
  {
    //m_PointSetNode = dynamic_cast<mitk::PointSet*>(m_P1->GetData());
    return;
  }

  if ((!m_P1) || (!m_PointSetNode))
  {
    // create new ones
    m_PointSetNode = mitk::PointSet::New();
    m_P1 = mitk::DataNode::New();
    m_P1->SetData( m_PointSetNode );
    m_P1->SetProperty( "name", mitk::StringProperty::New( "PointSet" ) );
    m_P1->SetProperty( "opacity", mitk::FloatProperty::New( 1 ) );
    m_P1->SetProperty( "helper object", mitk::BoolProperty::New(true) ); // CHANGE if wanted
    m_P1->SetProperty( "pointsize", mitk::FloatProperty::New( 0.1 ) );
    m_P1->SetColor( 1.0, 0.0, 0.0 );
    this->GetDataStorage()->Add(m_P1);
    m_Controls->m_PointWidget->SetPointSetNode(m_P1);
    auto renderWindowPart = this->GetRenderWindowPart(mitk::WorkbenchUtil::IRenderWindowPartStrategy::OPEN);
    auto axialSnc = renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController();
    auto sagittalSnc = renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController();
    auto coronalSnc = renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController();
    m_Controls->m_PointWidget->AddSliceNavigationController(axialSnc);
    m_Controls->m_PointWidget->AddSliceNavigationController(sagittalSnc);
    m_Controls->m_PointWidget->AddSliceNavigationController(coronalSnc);
  }
}

void QmitkTractbasedSpatialStatisticsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkTractbasedSpatialStatisticsViewControls;
    m_Controls->setupUi( parent );
    this->CreateConnections();
  }


  // Table for the FSL TBSS import
  m_GroupModel = new QmitkTbssTableModel();
  m_Controls->m_GroupInfo->setModel(m_GroupModel);


}

void QmitkTractbasedSpatialStatisticsView::SetFocus()
{
  m_Controls->m_AddGroup->setFocus();
}

void QmitkTractbasedSpatialStatisticsView::Activated()
{
  m_Activated = true;
}

void QmitkTractbasedSpatialStatisticsView::Deactivated()
{
  m_Activated = false;
}

bool QmitkTractbasedSpatialStatisticsView::IsActivated() const
{
  return m_Activated;
}

void QmitkTractbasedSpatialStatisticsView::Visible()
{
}

void QmitkTractbasedSpatialStatisticsView::Hidden()
{
}

void QmitkTractbasedSpatialStatisticsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_CreateRoi), SIGNAL(clicked()), this, SLOT(CreateRoi()) );
    connect( (QObject*)(m_Controls->m_ImportFsl), SIGNAL(clicked()), this, SLOT(TbssImport()) );
    connect( (QObject*)(m_Controls->m_AddGroup), SIGNAL(clicked()), this, SLOT(AddGroup()) );
    connect( (QObject*)(m_Controls->m_RemoveGroup), SIGNAL(clicked()), this, SLOT(RemoveGroup()) );
    connect( (QObject*)(m_Controls->m_Clipboard), SIGNAL(clicked()), this, SLOT(CopyToClipboard()) );
    connect( m_Controls->m_RoiPlotWidget->m_PlotPicker, SIGNAL(selected(const QPointF&)), SLOT(Clicked(const QPointF&) ) );
    connect( m_Controls->m_RoiPlotWidget->m_PlotPicker, SIGNAL(moved(const QPointF&)), SLOT(Clicked(const QPointF&) ) );
    connect( (QObject*)(m_Controls->m_Cut), SIGNAL(clicked()), this, SLOT(Cut()) );
    connect( (QObject*)(m_Controls->m_Average), SIGNAL(stateChanged(int)), this, SLOT(PerformChange()) );
    connect( (QObject*)(m_Controls->m_Segments), SIGNAL(valueChanged(int)), this, SLOT(PerformChange()) );

  }
}


void QmitkTractbasedSpatialStatisticsView::CopyToClipboard()
{



  if(m_Controls->m_RoiPlotWidget->IsPlottingFiber())
  {
    // Working with fiber bundles
    std::vector <std::vector<double> > profiles = m_Controls->m_RoiPlotWidget->GetIndividualProfiles();

    QString clipboardText;
    for (std::vector<std::vector<double> >::iterator it = profiles.begin(); it
                                                           != profiles.end(); ++it)
    {
      for (std::vector<double>::iterator it2 = (*it).begin(); it2 !=
            (*it).end(); ++it2)
      {
        clipboardText.append(QString("%1 \t").arg(*it2));
      }
      clipboardText.append(QString("\n"));
    }



    if(m_Controls->m_Average->isChecked())
    {
      std::vector<double> averages = m_Controls->m_RoiPlotWidget->GetAverageProfile();
      clipboardText.append(QString("\nAverage\n"));

      for (std::vector<double>::iterator it2 = averages.begin(); it2 !=
            averages.end(); ++it2)
      {
        clipboardText.append(QString("%1 \t").arg(*it2));

      }
    }

    QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);

  }

  else{

    // Working with TBSS Data
    if(m_Controls->m_Average->isChecked())
    {
      std::vector<std::vector<double> > vals = m_Controls->m_RoiPlotWidget->GetVals();
      QString clipboardText;
      for (std::vector<std::vector<double> >::iterator it = vals.begin(); it
                                                             != vals.end(); ++it)
      {
        for (std::vector<double>::iterator it2 = (*it).begin(); it2 !=
              (*it).end(); ++it2)
        {
          clipboardText.append(QString("%1 \t").arg(*it2));

          double d = *it2;
          std::cout << d <<std::endl;
        }
        clipboardText.append(QString("\n"));
      }

      QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);
    }
    else
    {
      std::vector<std::vector<double> > vals = m_Controls->m_RoiPlotWidget->GetIndividualProfiles();
      QString clipboardText;
      for (std::vector<std::vector<double> >::iterator it = vals.begin(); it
                                                             != vals.end(); ++it)
      {
        for (std::vector<double>::iterator it2 = (*it).begin(); it2 !=
              (*it).end(); ++it2)
        {
          clipboardText.append(QString("%1 \t").arg(*it2));

          double d = *it2;
          std::cout << d <<std::endl;
        }
        clipboardText.append(QString("\n"));
      }
      QApplication::clipboard()->setText(clipboardText, QClipboard::Clipboard);
    }

  }


}

void QmitkTractbasedSpatialStatisticsView::RemoveGroup()
{

  QTableView *temp = static_cast<QTableView*>(m_Controls->m_GroupInfo);

  QItemSelectionModel *selectionModel = temp->selectionModel();

  QModelIndexList indices = selectionModel->selectedRows();

  QModelIndex index;

  foreach(index, indices)
  {
    int row = index.row();
    m_GroupModel->removeRows(row, 1, QModelIndex());
  }

}



void QmitkTractbasedSpatialStatisticsView::AddGroup()
{
  QString group("Group");
  int number = 0;
  QPair<QString, int> pair(group, number);
  QList< QPair<QString, int> >list = m_GroupModel->getList();


  if(!list.contains(pair))
  {
    m_GroupModel->insertRows(0, 1, QModelIndex());

    QModelIndex index = m_GroupModel->index(0, 0, QModelIndex());
    m_GroupModel->setData(index, group, Qt::EditRole);
    index = m_GroupModel->index(0, 1, QModelIndex());
    m_GroupModel->setData(index, number, Qt::EditRole);

  }
}


void QmitkTractbasedSpatialStatisticsView::TbssImport()
{

  // Read groups from the interface
  mitk::TbssImporter::Pointer importer = mitk::TbssImporter::New();

  QList< QPair<QString, int> >list = m_GroupModel->getList();


  if(list.size() == 0)
  {
    QMessageBox msgBox;
    msgBox.setText("No study group information has been set yet.");
    msgBox.exec();
    return;
  }


  std::vector < std::pair<std::string, int> > groups;
  for(int i=0; i<list.size(); i++)
  {
    QPair<QString, int> pair = list.at(i);
    std::string s = pair.first.toStdString();
    int n = pair.second;

    std::pair<std::string, int> p;
    p.first = s;
    p.second = n;
    groups.push_back(p);
  }

  importer->SetGroupInfo(groups);

  std::string minfo = m_Controls->m_MeasurementInfo->text().toStdString();
  importer->SetMeasurementInfo(minfo);


  std::string name = "";

  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

  for (auto node: nodes)
  {
    if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
    {
      mitk::Image* img = static_cast<mitk::Image*>(node->GetData());
      if(img->GetDimension() == 4)
      {
        importer->SetImportVolume(img);
        name = node->GetName();
      }
    }
  }


  mitk::TbssImage::Pointer tbssImage;

  tbssImage = importer->Import();
  name += "_tbss";
  AddTbssToDataStorage(tbssImage, name);


}


void QmitkTractbasedSpatialStatisticsView::AddTbssToDataStorage(mitk::Image* image, std::string name)
{
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( image );
  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  levWinProp->SetLevelWindow( levelwindow );

  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty( "name", mitk::StringProperty::New(name) );
  result->SetData( image );
  result->SetProperty( "levelwindow", levWinProp );


  // add new image to data storage and set as active to ease further processing
  GetDataStorage()->Add( result );

  // show the results
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkTractbasedSpatialStatisticsView::Clicked(const QPointF& pos)
{
  int index = (int)pos.x();

  if(m_Roi.size() > 0 && m_CurrentGeometry != nullptr && !m_Controls->m_RoiPlotWidget->IsPlottingFiber() )
  {

    index = std::min( (int)m_Roi.size()-1, std::max(0, index) );
    itk::Index<3> ix = m_Roi.at(index);

    mitk::Vector3D i;
    i[0] = ix[0];
    i[1] = ix[1];
    i[2] = ix[2];

    mitk::Vector3D w;

    m_CurrentGeometry->IndexToWorld(i, w);

    mitk::Point3D origin = m_CurrentGeometry->GetOrigin();

    mitk::Point3D p;
    p[0] = w[0] + origin[0];
    p[1] = w[1] + origin[1];
    p[2] = w[2] + origin[2];

    this->GetRenderWindowPart()->SetSelectedPosition(p);
    m_Controls->m_RoiPlotWidget->drawBar(index);
  }

  else if(m_Controls->m_RoiPlotWidget->IsPlottingFiber() )
  {

    mitk::Point3D point = m_Controls->m_RoiPlotWidget->GetPositionInWorld(index);
    this->GetRenderWindowPart()->SetSelectedPosition(point);


  }


}

void QmitkTractbasedSpatialStatisticsView::Cut()
{
  mitk::BaseData* fibData = m_CurrentFiberNode->GetData();
  mitk::FiberBundle* fib = static_cast<mitk::FiberBundle*>(fibData);

  mitk::PlaneGeometry* startGeometry2D = const_cast<mitk::PlaneGeometry*>(dynamic_cast<mitk::PlanarFigure*>(m_CurrentStartRoi->GetData())->GetPlaneGeometry());
  mitk::PlaneGeometry* endGeometry2D = const_cast<mitk::PlaneGeometry*>(dynamic_cast<mitk::PlanarFigure*>(m_CurrentEndRoi->GetData())->GetPlaneGeometry());

  mitk::Point3D startCenter = dynamic_cast<mitk::PlanarFigure*>(m_CurrentStartRoi->GetData())->GetWorldControlPoint(0); //center Point of start roi
  mitk::Point3D endCenter = dynamic_cast<mitk::PlanarFigure*>(m_CurrentEndRoi->GetData())->GetWorldControlPoint(0); //center Point of end roi

  mitk::FiberBundle::Pointer inStart = fib->ExtractFiberSubset(m_CurrentStartRoi, nullptr);
  mitk::FiberBundle::Pointer inBoth = inStart->ExtractFiberSubset(m_CurrentEndRoi, nullptr);

  int num = inBoth->GetNumFibers();

  vtkSmartPointer<vtkPolyData> fiberPolyData = inBoth->GetFiberPolyData();
  vtkCellArray* lines = fiberPolyData->GetLines();
  lines->InitTraversal();


  // initialize new vtk polydata
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();

  int pointIndex=0;


  // find start and endpoint
  for( int fiberID( 0 ); fiberID < num; fiberID++ )
  {
    vtkIdType   numPointsInCell(0);
    vtkIdType*  pointsInCell(nullptr);
    lines->GetNextCell ( numPointsInCell, pointsInCell );

    int startId = 0;
    int endId = 0;

    float minDistStart = std::numeric_limits<float>::max();
    float minDistEnd = std::numeric_limits<float>::max();


    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    int lineIndex=0;


    for( int pointInCellID( 0 ); pointInCellID < numPointsInCell ; pointInCellID++)
    {
      double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );

      mitk::Point3D point;
      point[0] = p[0];
      point[1] = p[1];
      point[2] = p[2];

      float distanceToStart = point.EuclideanDistanceTo(startCenter);
      float distanceToEnd = point.EuclideanDistanceTo(endCenter);

      if(distanceToStart < minDistStart)
      {
        minDistStart = distanceToStart;
        startId = pointInCellID;
      }

      if(distanceToEnd < minDistEnd)
      {
        minDistEnd = distanceToEnd;
        endId = pointInCellID;
      }



    }



    /* We found the start and end points of of the part that should be plottet for
       the current fiber. now we need to plot them. If the endId is smaller than the startId the plot order
       must be reversed*/

    if(startId < endId)
    {

      double *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );

      mitk::Vector3D p0;
      p0[0] = p[0];
      p0[1] = p[1];
      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ startId+1 ] );

      mitk::Vector3D p1;
      p1[0] = p[0];
      p1[1] = p[1];
      p1[2] = p[2];


      // Check if p and p2 are both on the same side of the plane
      mitk::Vector3D normal = startGeometry2D->GetNormal();

      mitk::Point3D pStart;
      pStart[0] = p0[0];
      pStart[1] = p0[1];
      pStart[2] = p0[2];

      bool startOnPositive = startGeometry2D->IsAbove(pStart);

      mitk::Point3D pSecond;
      pSecond[0] = p1[0];
      pSecond[1] = p1[1];
      pSecond[2] = p1[2];

      bool secondOnPositive = startGeometry2D->IsAbove(pSecond);


      // Calculate intersection with the plane

      mitk::Vector3D onPlane;
      onPlane[0] = startCenter[0];
      onPlane[1] = startCenter[1];
      onPlane[2] = startCenter[2];


      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* startId and startId+1 lie on the same side of the plane, so we need
           need startId-1 to calculate the intersection with the planar figure*/
        p = fiberPolyData->GetPoint( pointsInCell[ startId-1 ] );
        p1[0] = p[0];
        p1[1] = p[1];
        p1[2] = p[2];
      }


      double d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      mitk::Vector3D newPoint = (p0-p1);

      newPoint[0] = d*newPoint[0] + p0[0];
      newPoint[1] = d*newPoint[1] + p0[1];
      newPoint[2] = d*newPoint[2] + p0[2];


      double insertPoint[3];
      insertPoint[0] = newPoint[0];
      insertPoint[1] = newPoint[1];
      insertPoint[2] = newPoint[2];


      // First insert the intersection with the start roi
      points->InsertNextPoint(insertPoint);
      polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
      lineIndex++;
      pointIndex++;

      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* StartId and startId+1 lie on the same side of the plane
           so startId is also part of the ROI*/

        double *start = fiberPolyData->GetPoint( pointsInCell[startId] );
        points->InsertNextPoint(start);
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;


      }

      // Insert the rest up and to including endId-1
      for( int pointInCellID( startId+1 ); pointInCellID < endId ; pointInCellID++)
      {
        // create new polyline for new polydata
        double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
        points->InsertNextPoint(p);


        // add point to line
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;

      }



      /* endId must be included if endId and endId-1 lie on the same side of the
         plane defined by endRoi*/


      p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
      p0[0] = p[0];      p0[1] = p[1];      p0[2] = p[2];


      p = fiberPolyData->GetPoint( pointsInCell[ endId-1 ] );
      p1[0] = p[0];      p1[1] = p[1];      p1[2] = p[2];


      mitk::Point3D pLast;
      pLast[0] = p0[0];      pLast[1] = p0[1];      pLast[2] = p0[2];

      mitk::Point3D pBeforeLast;
      pBeforeLast[0] = p1[0];      pBeforeLast[1] = p1[1];      pBeforeLast[2] = p1[2];

      bool lastOnPositive = endGeometry2D->IsAbove(pLast);
      bool secondLastOnPositive = endGeometry2D->IsAbove(pBeforeLast);
      normal = endGeometry2D->GetNormal();


      onPlane[0] = endCenter[0];
      onPlane[1] = endCenter[1];
      onPlane[2] = endCenter[2];



      if(! (lastOnPositive ^ secondLastOnPositive) )
      {
        /* endId and endId-1 lie on the same side of the plane, so we need
           need endId+1 to calculate the intersection with the planar figure.
           this should exist since we know that the fiber crosses the planar figure
           endId is part of the roi so can also be included here*/
        p = fiberPolyData->GetPoint( pointsInCell[ endId+1 ] );
        p1[0] = p[0];
        p1[1] = p[1];
        p1[2] = p[2];


        double *end = fiberPolyData->GetPoint( pointsInCell[endId] );
        points->InsertNextPoint(end);
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;
      }

      d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      newPoint = (p0-p1);

      newPoint[0] = d*newPoint[0] + p0[0];
      newPoint[1] = d*newPoint[1] + p0[1];
      newPoint[2] = d*newPoint[2] + p0[2];

      insertPoint[0] = newPoint[0];
      insertPoint[1] = newPoint[1];
      insertPoint[2] = newPoint[2];

      //Insert the Last Point (intersection with the end roi)

      points->InsertNextPoint(insertPoint);
      polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
      lineIndex++;
      pointIndex++;


    }

    // Need to reverse walking order
    else{
      double *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );

      mitk::Vector3D p0;
      p0[0] = p[0];
      p0[1] = p[1];
      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ startId-1 ] );

      mitk::Vector3D p1;
      p1[0] = p[0];
      p1[1] = p[1];
      p1[2] = p[2];


      // Check if p and p2 are both on the same side of the plane
      mitk::Vector3D normal = startGeometry2D->GetNormal();

      mitk::Point3D pStart;
      pStart[0] = p0[0];
      pStart[1] = p0[1];
      pStart[2] = p0[2];

      bool startOnPositive = startGeometry2D->IsAbove(pStart);

      mitk::Point3D pSecond;
      pSecond[0] = p1[0];
      pSecond[1] = p1[1];
      pSecond[2] = p1[2];

      bool secondOnPositive = startGeometry2D->IsAbove(pSecond);


      // Calculate intersection with the plane

      mitk::Vector3D onPlane;
      onPlane[0] = startCenter[0];
      onPlane[1] = startCenter[1];
      onPlane[2] = startCenter[2];


      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* startId and startId-1 lie on the same side of the plane, so we need
           need startId+1 to calculate the intersection with the planar figure*/
        p = fiberPolyData->GetPoint( pointsInCell[ startId+1 ] );
        p1[0] = p[0];
        p1[1] = p[1];
        p1[2] = p[2];
      }


      double d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      mitk::Vector3D newPoint = (p0-p1);

      newPoint[0] = d*newPoint[0] + p0[0];
      newPoint[1] = d*newPoint[1] + p0[1];
      newPoint[2] = d*newPoint[2] + p0[2];


      double insertPoint[3];
      insertPoint[0] = newPoint[0];
      insertPoint[1] = newPoint[1];
      insertPoint[2] = newPoint[2];


      // First insert the intersection with the start roi
      points->InsertNextPoint(insertPoint);
      polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
      lineIndex++;
      pointIndex++;

      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* startId and startId-1 lie on the same side of the plane
           so endId is also part of the ROI*/

        double *start = fiberPolyData->GetPoint( pointsInCell[startId] );
        points->InsertNextPoint(start);
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;


      }

      // Insert the rest up and to including endId-1
      for( int pointInCellID( startId-1 ); pointInCellID > endId ; pointInCellID--)
      {
        // create new polyline for new polydata
        double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
        points->InsertNextPoint(p);


        // add point to line
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;

      }



      /* startId must be included if startId and startId+ lie on the same side of the
         plane defined by endRoi*/


      p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
      p0[0] = p[0];
      p0[1] = p[1];
      p0[2] = p[2];


      p = fiberPolyData->GetPoint( pointsInCell[ endId+1 ] );
      p1[0] = p[0];
      p1[1] = p[1];
      p1[2] = p[2];


      mitk::Point3D pLast;
      pLast[0] = p0[0];
      pLast[1] = p0[1];
      pLast[2] = p0[2];

      bool lastOnPositive = endGeometry2D->IsAbove(pLast);

      mitk::Point3D pBeforeLast;
      pBeforeLast[0] = p1[0];
      pBeforeLast[1] = p1[1];
      pBeforeLast[2] = p1[2];

      bool secondLastOnPositive = endGeometry2D->IsAbove(pBeforeLast);

      onPlane[0] = endCenter[0];
      onPlane[1] = endCenter[1];
      onPlane[2] = endCenter[2];



      if(! (lastOnPositive ^ secondLastOnPositive) )
      {
        /* endId and endId+1 lie on the same side of the plane, so we need
           need endId-1 to calculate the intersection with the planar figure.
           this should exist since we know that the fiber crosses the planar figure*/
        p = fiberPolyData->GetPoint( pointsInCell[ endId-1 ] );
        p1[0] = p[0];
        p1[1] = p[1];
        p1[2] = p[2];


        /* endId and endId+1 lie on the same side of the plane
           so startId is also part of the ROI*/

        double *end = fiberPolyData->GetPoint( pointsInCell[endId] );
        points->InsertNextPoint(end);
        polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
        lineIndex++;
        pointIndex++;
      }

      d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      newPoint = (p0-p1);

      newPoint[0] = d*newPoint[0] + p0[0];
      newPoint[1] = d*newPoint[1] + p0[1];
      newPoint[2] = d*newPoint[2] + p0[2];


      insertPoint[0] = newPoint[0];
      insertPoint[1] = newPoint[1];
      insertPoint[2] = newPoint[2];



      //Insert the Last Point (intersection with the end roi)
      points->InsertNextPoint(insertPoint);
      polyLine->GetPointIds()->InsertId(lineIndex,pointIndex);
      lineIndex++;
      pointIndex++;



    }


    // add polyline to vtk cell array
     cells->InsertNextCell(polyLine);

  }


  // Add the points to the dataset
  polyData->SetPoints(points);

  // Add the lines to the dataset
  polyData->SetLines(cells);

  mitk::FiberBundle::Pointer cutBundle = mitk::FiberBundle::New(polyData);


  mitk::DataNode::Pointer cutNode = mitk::DataNode::New();
  cutNode->SetData(cutBundle);
  std::string name = "fiberCut";
  cutNode->SetName(name);
  GetDataStorage()->Add(cutNode);



}


void QmitkTractbasedSpatialStatisticsView::CreateRoi()
{
  bool ok;
  double threshold = QInputDialog::getDouble(m_Controls->m_CreateRoi, tr("Set an FA threshold"),
                                                      tr("Threshold:"), 0.2, 0.0, 1.0, 2, &ok);

  if(!ok)
    return;

  mitk::Image::Pointer image;

  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

  for (auto node: nodes)
  {
    if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
    {
        mitk::Image* img = static_cast<mitk::Image*>(node->GetData());
        if(img->GetDimension() == 3)
        {
          image = img;
        }
    }
  }

  if(image.IsNull())
  {
    return;
  }

  mitk::TractAnalyzer analyzer;
  analyzer.SetInputImage(image);
  analyzer.SetThreshold(threshold);

  m_PointSetNode = this->m_Controls->m_PointWidget->GetPointSet();

  // Set Pointset to analyzer
  analyzer.SetPointSet(m_PointSetNode);

  // Run Analyzer
  try
  {
    analyzer.MakeRoi();
  }
  catch (const mitk::Exception& e)
  {
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(e.what()));
    msgBox.exec();
  }

  // Obtain tbss roi image from analyzer
  mitk::TbssRoiImage::Pointer tbssRoi = analyzer.GetRoiImage();

  tbssRoi->SetStructure(m_Controls->m_Structure->text().toStdString());

  // get path description and set to interface
  std::string pathDescription = analyzer.GetPathDescription();
  m_Controls->m_PathTextEdit->setPlainText(QString(pathDescription.c_str()));


  // Add roi image to datastorage
  AddTbssToDataStorage(tbssRoi, m_Controls->m_RoiName->text().toStdString());




}

void QmitkTractbasedSpatialStatisticsView::PlotFiber4D(mitk::TbssImage* image,
                                                            mitk::FiberBundle* fib,
                                                            mitk::DataNode* startRoi,
                                                            mitk::DataNode* endRoi)
{


  if(m_Controls->m_TabWidget->currentWidget() == m_Controls->m_MeasureTAB)
  {
    m_CurrentGeometry = image->GetGeometry();

    m_Controls->m_RoiPlotWidget->SetGroups(image->GetGroupInfo());
    m_Controls->m_RoiPlotWidget->SetProjections(image->GetImage());
    m_Controls->m_RoiPlotWidget->SetMeasure( image->GetMeasurementInfo() );
    m_Controls->m_RoiPlotWidget->PlotFiber4D(image, fib, startRoi, endRoi, m_Controls->m_Segments->value());
  }


}

void QmitkTractbasedSpatialStatisticsView:: PlotFiberBundle(mitk::FiberBundle *fib, mitk::Image* img,
                                                           mitk::DataNode* startRoi, mitk::DataNode* endRoi)
{
  bool avg = m_Controls->m_Average->isChecked();
  int segments = m_Controls->m_Segments->value();
  m_Controls->m_RoiPlotWidget->PlotFiberBetweenRois(fib, img, startRoi ,endRoi, avg, segments);
  m_Controls->m_RoiPlotWidget->SetPlottingFiber(true);
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}


void QmitkTractbasedSpatialStatisticsView::Plot(mitk::TbssImage* image, mitk::TbssRoiImage* roiImage)
{
  if(m_Controls->m_TabWidget->currentWidget() == m_Controls->m_MeasureTAB)
  {

    std::vector< itk::Index<3> > roi = roiImage->GetRoi();
    m_Roi = roi;
    m_CurrentGeometry = image->GetGeometry();


    std::string structure = roiImage->GetStructure();

    m_Controls->m_RoiPlotWidget->SetGroups(image->GetGroupInfo());
    m_Controls->m_RoiPlotWidget->SetProjections(image->GetImage());
    m_Controls->m_RoiPlotWidget->SetRoi(roi);
    m_Controls->m_RoiPlotWidget->SetStructure(structure);
    m_Controls->m_RoiPlotWidget->SetMeasure( image->GetMeasurementInfo() );
    m_Controls->m_RoiPlotWidget->DrawProfiles();
  }

  m_Controls->m_RoiPlotWidget->SetPlottingFiber(false);

}
