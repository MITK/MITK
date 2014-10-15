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

#include "QmitkToFPointSetWidget.h"

#include <mitkGlobalInteraction.h>
#include <mitkVtkLayerController.h>

#include <vtkTextProperty.h>


const std::string QmitkToFPointSetWidget::VIEW_ID = "org.mitk.views.qmitktofpointsetwidget";

QmitkToFPointSetWidget::QmitkToFPointSetWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
, m_DataStorage(NULL)
, m_DistanceImage(NULL)
, m_CameraIntrinsics(NULL)
, m_VtkTextActor(NULL)
, m_ForegroundRenderer1(NULL)
, m_ForegroundRenderer2(NULL)
, m_ForegroundRenderer3(NULL)
, m_RenderWindow1(NULL)
, m_RenderWindow2(NULL)
, m_RenderWindow3(NULL)
, m_MeasurementPointSet2D(NULL)
, m_MeasurementPointSet3DNode(NULL)
, m_PointSet2D(NULL)
, m_PointSet3DNode(NULL)
, m_PointSetInteractor(NULL)
, m_MeasurementPointSetInteractor(NULL)
, m_MeasurementPointSetChangedObserverTag(0)
, m_PointSetChangedObserverTag(0)
, m_WindowHeight(0)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFPointSetWidget::~QmitkToFPointSetWidget()
{
  this->CleanUpWidget();
}

void QmitkToFPointSetWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFPointSetWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();
  }
}

void QmitkToFPointSetWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->measureButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnMeasurement()) );
    connect( (QObject*)(m_Controls->pointSetButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnPointSet()) );
  }
}

void QmitkToFPointSetWidget::InitializeWidget(QHash<QString, QmitkRenderWindow*> renderWindowHashMap, mitk::DataStorage::Pointer dataStorage, mitk::CameraIntrinsics::Pointer cameraIntrinsics)
{
  // initialize members
  m_CameraIntrinsics = cameraIntrinsics;
  m_DataStorage = dataStorage;
  //  m_RenderWindowPart = renderWindowPart;
  m_RenderWindow1 = renderWindowHashMap.value("axial")->GetRenderWindow();
  m_RenderWindow2 = renderWindowHashMap.value("sagittal")->GetRenderWindow();
  m_RenderWindow3 = renderWindowHashMap.value("coronal")->GetRenderWindow();
  m_RenderWindow4 = renderWindowHashMap.value("3d")->GetRenderWindow();
  if ((m_RenderWindow1 != NULL) && (m_RenderWindow2 != NULL) && (m_RenderWindow3 != NULL) && (m_RenderWindow4 != NULL) && (dataStorage.IsNotNull()))
  {
    // enable buttons
    m_Controls->pointSetButton->setEnabled(true);
    m_Controls->measureButton->setEnabled(true);
    // initialize overlays
    this->m_VtkTextActor = vtkSmartPointer<vtkTextActor>::New();
    this->m_VtkTextActor->SetInput("Choose measurement points with SHIFT+Click");
    m_WindowHeight = renderWindowHashMap.value("axial")->GetRenderer()->GetSizeY();
    this->m_VtkTextActor->SetDisplayPosition(10,m_WindowHeight-30);
    this->m_VtkTextActor->GetTextProperty()->SetFontSize(16);
    //    this->m_VtkTextActor->GetTextProperty()->SetColor(1,0,0);
    this->m_VtkTextActor->GetTextProperty()->BoldOn();
    this->m_VtkTextActor->SetVisibility(0);
    if (m_ForegroundRenderer1==NULL)
    {
      this->m_ForegroundRenderer1 = vtkSmartPointer<vtkRenderer>::New();
      this->m_ForegroundRenderer1->AddActor(m_VtkTextActor);
      mitk::VtkLayerController::GetInstance(m_RenderWindow1)->InsertForegroundRenderer(m_ForegroundRenderer1,true);
    }
    if (m_ForegroundRenderer2==NULL)
    {
      this->m_ForegroundRenderer2 = vtkSmartPointer<vtkRenderer>::New();
      this->m_ForegroundRenderer2->AddActor(m_VtkTextActor);
      mitk::VtkLayerController::GetInstance(m_RenderWindow2)->InsertForegroundRenderer(m_ForegroundRenderer2,true);
    }
    if (m_ForegroundRenderer3==NULL)
    {
      this->m_ForegroundRenderer3 =vtkSmartPointer<vtkRenderer>::New();
      this->m_ForegroundRenderer3->AddActor(m_VtkTextActor);
      mitk::VtkLayerController::GetInstance(m_RenderWindow3)->InsertForegroundRenderer(m_ForegroundRenderer3,true);
    }
    mitk::DataNode::Pointer measurementPointSet2DNode = dataStorage->GetNamedNode("Measurement PointSet 2D") ;
    if(dataStorage->Exists(measurementPointSet2DNode))
    {
      dataStorage->Remove(measurementPointSet2DNode);
    }
    // initialize 2D measurement point set
    m_MeasurementPointSet2D = mitk::PointSet::New();
    measurementPointSet2DNode = mitk::DataNode::New();
    measurementPointSet2DNode->SetName("Measurement PointSet 2D");
    measurementPointSet2DNode->SetBoolProperty("helper object",true);
    measurementPointSet2DNode->SetBoolProperty("show contour",true);
    measurementPointSet2DNode->SetVisibility(false, renderWindowHashMap.value("3d")->GetRenderer());
    measurementPointSet2DNode->SetData(m_MeasurementPointSet2D);
    dataStorage->Add(measurementPointSet2DNode);
    m_MeasurementPointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor",measurementPointSet2DNode,2);
    // create observer for m_MeasurementPointSet2D
    itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer measurementPointSetChangedCommand;
    measurementPointSetChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
    measurementPointSetChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::MeasurementPointSetChanged);
    m_MeasurementPointSetChangedObserverTag = m_MeasurementPointSet2D->AddObserver(itk::ModifiedEvent(), measurementPointSetChangedCommand);
    // initialize 3D measurement PointSet
    m_MeasurementPointSet3DNode = dataStorage->GetNamedNode("Measurement PointSet 3D");
    if(dataStorage->Exists(m_MeasurementPointSet3DNode))
    {
      dataStorage->Remove(m_MeasurementPointSet3DNode);
    }
    m_MeasurementPointSet3DNode = mitk::DataNode::New();
    m_MeasurementPointSet3DNode->SetName("Measurement PointSet 3D");
    m_MeasurementPointSet3DNode->SetBoolProperty("helper object",true);
    m_MeasurementPointSet3DNode->SetBoolProperty("show contour",true);
    m_MeasurementPointSet3DNode->SetFloatProperty("pointsize",5.0f);
    mitk::PointSet::Pointer measurementPointSet3D = mitk::PointSet::New();
    m_MeasurementPointSet3DNode->SetData(measurementPointSet3D);
    dataStorage->Add(m_MeasurementPointSet3DNode);

    // initialize PointSets
    mitk::DataNode::Pointer pointSet2DNode = dataStorage->GetNamedNode("ToF PointSet 2D") ;
    if(dataStorage->Exists(pointSet2DNode))
    {
      dataStorage->Remove(pointSet2DNode);
    }
    m_PointSet2D = mitk::PointSet::New();
    pointSet2DNode = mitk::DataNode::New();
    pointSet2DNode->SetName("ToF PointSet 2D");
    pointSet2DNode->SetVisibility(false, renderWindowHashMap.value("3d")->GetRenderer());
    pointSet2DNode->SetData(m_PointSet2D);
    dataStorage->Add(pointSet2DNode);
    m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor",pointSet2DNode);

    // create observer for m_MeasurementPointSet2D
    itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer pointSetChangedCommand;
    pointSetChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
    pointSetChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::PointSetChanged);
    m_PointSetChangedObserverTag = m_PointSet2D->AddObserver(itk::ModifiedEvent(), pointSetChangedCommand);
    // initialize 3D point set
    mitk::DataNode::Pointer pointSet3DNode = dataStorage->GetNamedNode("ToF PointSet 3D");
    if(dataStorage->Exists(pointSet3DNode))
    {
      dataStorage->Remove(pointSet3DNode);
    }
    m_PointSet3DNode = mitk::DataNode::New();
    m_PointSet3DNode->SetName("ToF PointSet 3D");
    m_PointSet3DNode->SetFloatProperty("pointsize",5.0f);
    mitk::PointSet::Pointer pointSet3D = mitk::PointSet::New();
    m_PointSet3DNode->SetData(pointSet3D);
    dataStorage->Add(m_PointSet3DNode);
  }
}

void QmitkToFPointSetWidget::CleanUpWidget()
{
  // toggle button state
  if (m_Controls->measureButton->isChecked())
  {
    m_Controls->measureButton->setChecked(false);
    this->OnMeasurement();
  }
  if (m_Controls->pointSetButton->isChecked())
  {
    m_Controls->pointSetButton->setChecked(false);
    this->OnPointSet();
  }
  // remove observer
  if (m_MeasurementPointSet2D.IsNotNull())
  {
    m_MeasurementPointSet2D->RemoveObserver(m_MeasurementPointSetChangedObserverTag);
  }
  if (m_PointSet2D.IsNotNull())
  {
    m_PointSet2D->RemoveObserver(m_PointSetChangedObserverTag);
  }
//  if (m_DistanceImage.IsNotNull())
//  {
//    m_DistanceImage->RemoveObserver(m_DistanceImageChangedObserverTag);
//  }
  // remove foreground renderer
  if (m_ForegroundRenderer1&&m_RenderWindow1)
  {
    if (mitk::VtkLayerController::GetInstance(m_RenderWindow1))
    {
      mitk::VtkLayerController::GetInstance(m_RenderWindow1)->RemoveRenderer(m_ForegroundRenderer1);
    }
    m_ForegroundRenderer1 = NULL;
  }
  if (m_ForegroundRenderer2&&m_RenderWindow2)
  {
    if (mitk::VtkLayerController::GetInstance(m_RenderWindow2))
    {
      mitk::VtkLayerController::GetInstance(m_RenderWindow2)->RemoveRenderer(m_ForegroundRenderer2);
    }
    m_ForegroundRenderer2 = NULL;
  }
  if (m_ForegroundRenderer3&&m_RenderWindow3)
  {
    if (mitk::VtkLayerController::GetInstance(m_RenderWindow3))
    {
      mitk::VtkLayerController::GetInstance(m_RenderWindow3)->RemoveRenderer(m_ForegroundRenderer3);
    }
    m_ForegroundRenderer3 = NULL;
  }
  if (mitk::RenderingManager::GetInstance())
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkToFPointSetWidget::SetDistanceImage(mitk::Image::Pointer distanceImage)
{
//  // remove existing observer
//  if (m_DistanceImage.IsNotNull())
//  {
//    m_DistanceImage->RemoveObserver(m_DistanceImageChangedObserverTag);
//  }
  m_DistanceImage = distanceImage;
//  // create observer for m_DistanceImage
//  itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer distanceImageChangedCommand;
//  distanceImageChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
//  distanceImageChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::MeasurementPointSetChanged);
//  m_DistanceImageChangedObserverTag = m_DistanceImage->AddObserver(itk::ModifiedEvent(), distanceImageChangedCommand);
}

void QmitkToFPointSetWidget::SetCameraIntrinsics(mitk::CameraIntrinsics::Pointer cameraIntrinsics)
{
  m_CameraIntrinsics = cameraIntrinsics;
}

void QmitkToFPointSetWidget::OnMeasurement()
{
  // always show 2D PointSet in foreground
  mitk::DataNode::Pointer pointSetNode = m_DataStorage->GetNamedNode("Measurement PointSet 2D");
  if (pointSetNode.IsNotNull())
  {
    pointSetNode->SetIntProperty("layer",100);
  }
  if (m_Controls->measureButton->isChecked())
  {
    // disable point set interaction
    if (m_Controls->pointSetButton->isChecked())
    {
      m_Controls->pointSetButton->setChecked(false);
      // remove interactor
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
    }
    // show overlays
    m_VtkTextActor->SetVisibility(1);
    this->m_VtkTextActor->SetInput("Choose measurement points with SHIFT+Click");
    // enable interactor
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_MeasurementPointSetInteractor);
    // initial update of measurement
    this->MeasurementPointSetChanged();
  }
  else
  {
    // hide overlays
    m_VtkTextActor->SetVisibility(0);
    // disable interactor
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_MeasurementPointSetInteractor);
  }
}

void QmitkToFPointSetWidget::OnPointSet()
{
  // always show 2D PointSet in foreground
  mitk::DataNode::Pointer pointSetNode = m_DataStorage->GetNamedNode("ToF PointSet 2D");
  if (pointSetNode.IsNotNull())
  {
    pointSetNode->SetIntProperty("layer",100);
  }
  if (m_Controls->pointSetButton->isChecked())
  {
    // disable measurement
    if (m_Controls->measureButton->isChecked())
    {
      m_Controls->measureButton->setChecked(false);
      // remove interactor
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_MeasurementPointSetInteractor);
    }
    // show overlays
    m_VtkTextActor->SetVisibility(1);
    this->m_VtkTextActor->SetInput("Choose points with SHIFT+Click");
    // enable interactor
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
    // initial update of PointSet
    this->PointSetChanged();
  }
  else
  {
    // hide overlays
    m_VtkTextActor->SetVisibility(0);
    // disable interactor
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }
}

void QmitkToFPointSetWidget::MeasurementPointSetChanged()
{
  // replace text actor
  this->m_VtkTextActor->SetDisplayPosition(10,m_WindowHeight-30);
  if (m_MeasurementPointSet2D->GetSize()==2)
  {
    // check if points are inside the image range
    int imageSizeX = m_DistanceImage->GetDimensions()[0];
    int imageSizeY = m_DistanceImage->GetDimensions()[1];
    mitk::Point3D point1 = m_MeasurementPointSet2D->GetPoint(0);
    mitk::Point3D point2 = m_MeasurementPointSet2D->GetPoint(1);
    if ((point1[0]>=0.0f)&&(point1[0]<imageSizeX)&&(point1[1]>=0)&&(point1[1]<imageSizeY)&&
        (point2[0]>=0.0f)&&(point2[0]<imageSizeX)&&(point2[1]>=0)&&(point2[1]<imageSizeY))
    {
      // create PointSet filter
      mitk::ToFDistanceImageToPointSetFilter::Pointer toFDistanceImageToPointSetFilter = mitk::ToFDistanceImageToPointSetFilter::New();
      if (m_CameraIntrinsics.IsNotNull())
      {
        toFDistanceImageToPointSetFilter->SetCameraIntrinsics(m_CameraIntrinsics);
      }
      toFDistanceImageToPointSetFilter->SetInput(m_DistanceImage);
      toFDistanceImageToPointSetFilter->SetSubset(m_MeasurementPointSet2D);
      toFDistanceImageToPointSetFilter->Update();
      mitk::PointSet::Pointer measurementPointSet3D = toFDistanceImageToPointSetFilter->GetOutput();
      m_MeasurementPointSet3DNode->SetData(measurementPointSet3D);

      // calculate distance between points
      if (measurementPointSet3D->GetSize()==2)
      {
        mitk::Point3D point1 = measurementPointSet3D->GetPoint(0);
        mitk::Point3D point2 = measurementPointSet3D->GetPoint(1);
        float distance = point1.EuclideanDistanceTo(point2);
        std::stringstream stream;
        stream<<distance<<" mm";
        this->m_VtkTextActor->SetInput(stream.str().c_str());
      }
      else
      {
        this->m_VtkTextActor->SetInput("Choose measurement points with SHIFT+Click");
      }
    }
    else
    {
      this->m_VtkTextActor->SetInput("Measurement outside image range.");
    }
  }
  else
  {
    // initialize 3D pointset empty
    mitk::PointSet::Pointer pointSet3D = mitk::PointSet::New();
    m_MeasurementPointSet3DNode->SetData(pointSet3D);
  }
}

void QmitkToFPointSetWidget::PointSetChanged()
{
  if (m_DistanceImage.IsNotNull())
  {
  int imageSizeX = m_DistanceImage->GetDimensions()[0];
  int imageSizeY = m_DistanceImage->GetDimensions()[1];
  int pointSetValid = 1;
  for (int i=0; i<m_PointSet2D->GetSize(); i++)
  {
    mitk::Point3D currentPoint = m_PointSet2D->GetPoint(i);
    if ((currentPoint[0]>=0.0f)&&(currentPoint[0]<imageSizeX)&&(currentPoint[1]>=0)&&(currentPoint[1]<imageSizeY))
    {
      pointSetValid*=1;
    }
    else
    {
      pointSetValid*=0;
    }
  }
  if (m_PointSet2D->GetSize()>0)
  {
    if (pointSetValid)
    {
      // create PointSet filter
      mitk::ToFDistanceImageToPointSetFilter::Pointer toFDistanceImageToPointSetFilter = mitk::ToFDistanceImageToPointSetFilter::New();
      if (m_CameraIntrinsics.IsNotNull())
      {
        toFDistanceImageToPointSetFilter->SetCameraIntrinsics(m_CameraIntrinsics);
      }
      toFDistanceImageToPointSetFilter->SetInput(m_DistanceImage);
      toFDistanceImageToPointSetFilter->SetSubset(m_PointSet2D);
      toFDistanceImageToPointSetFilter->Update();
      mitk::PointSet::Pointer pointSet3D = toFDistanceImageToPointSetFilter->GetOutput();
      m_PointSet3DNode->SetData(pointSet3D);
      this->m_VtkTextActor->SetInput("Choose points with SHIFT+Click");    }
    else
    {
      this->m_VtkTextActor->SetInput("Point set outside image range.");
    }
  }
  else
  {
    // initialize 3D pointset empty
    mitk::PointSet::Pointer pointSet3D = mitk::PointSet::New();
    m_PointSet3DNode->SetData(pointSet3D);
  }
  }
}
