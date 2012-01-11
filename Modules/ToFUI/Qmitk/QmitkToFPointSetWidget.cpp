/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <QmitkToFPointSetWidget.h>

#include <QmitkTextOverlay.h>

#include <mitkGlobalInteraction.h>
#include <mitkVtkLayerController.h>

const std::string QmitkToFPointSetWidget::VIEW_ID = "org.mitk.views.qmitktofpointsetwidget";

QmitkToFPointSetWidget::QmitkToFPointSetWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
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
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFPointSetWidget::~QmitkToFPointSetWidget()
{
  if (m_MeasurementPointSet2D.IsNotNull())
  {
    m_MeasurementPointSet2D->RemoveObserver(m_MeasurementPointSetChangedObserverTag);
  }
  if (m_PointSet2D.IsNotNull())
  {
    m_PointSet2D->RemoveObserver(m_PointSetChangedObserverTag);
  }
  if (m_MultiWidget)
  {
    if (m_ForegroundRenderer1&&m_RenderWindow1)
    {
      if (mitk::VtkLayerController::GetInstance(m_RenderWindow1))
      {
        mitk::VtkLayerController::GetInstance(m_RenderWindow1)->RemoveRenderer(m_ForegroundRenderer1);
      }
    }
    if (m_ForegroundRenderer2&&m_RenderWindow2)
    {
      if (mitk::VtkLayerController::GetInstance(m_RenderWindow2))
      {
        mitk::VtkLayerController::GetInstance(m_RenderWindow2)->RemoveRenderer(m_ForegroundRenderer2);
      }
    }
    if (m_ForegroundRenderer3&&m_RenderWindow3)
    {
      if (mitk::VtkLayerController::GetInstance(m_RenderWindow3))
      {
        mitk::VtkLayerController::GetInstance(m_RenderWindow3)->RemoveRenderer(m_ForegroundRenderer3);
      }
    }
  }
  if (mitk::RenderingManager::GetInstance())
  {
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
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

void QmitkToFPointSetWidget::InitializeWidget(QmitkStdMultiWidget* stdMultiWidget, mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer distanceImage)
{
  // initialize members
  m_MultiWidget = stdMultiWidget;
  m_DistanceImage = distanceImage;
  if ((stdMultiWidget!=NULL)&&(dataStorage.IsNotNull()))
  {
    // enable buttons
    m_Controls->pointSetButton->setEnabled(true);
    m_Controls->measureButton->setEnabled(true);
    // initialize overlays
    this->m_VtkTextActor = vtkTextActor::New();
    this->m_VtkTextActor->SetInput("Choose measurement points with SHIFT+Click");
    int windowHeight = m_MultiWidget->mitkWidget1->GetRenderer()->GetSizeY();
    this->m_VtkTextActor->SetDisplayPosition(10,windowHeight-30);
    this->m_VtkTextActor->GetTextProperty()->SetFontSize(16);
//    this->m_VtkTextActor->GetTextProperty()->SetColor(1,0,0);
    this->m_VtkTextActor->GetTextProperty()->BoldOn();
    this->m_VtkTextActor->SetVisibility(0);
    this->m_ForegroundRenderer1 = vtkRenderer::New();
    this->m_ForegroundRenderer1->AddActor(m_VtkTextActor);
    m_RenderWindow1 = m_MultiWidget->mitkWidget1->GetRenderWindow();
    mitk::VtkLayerController::GetInstance(m_MultiWidget->mitkWidget1->GetRenderWindow())->InsertForegroundRenderer(m_ForegroundRenderer1,true);
    this->m_ForegroundRenderer2 = vtkRenderer::New();
    this->m_ForegroundRenderer2->AddActor(m_VtkTextActor);
    m_RenderWindow2 = m_MultiWidget->mitkWidget2->GetRenderWindow();
    mitk::VtkLayerController::GetInstance(m_RenderWindow2)->InsertForegroundRenderer(m_ForegroundRenderer2,true);
    this->m_ForegroundRenderer3 = vtkRenderer::New();
    this->m_ForegroundRenderer3->AddActor(m_VtkTextActor);
    m_RenderWindow3 = m_MultiWidget->mitkWidget3->GetRenderWindow();
    mitk::VtkLayerController::GetInstance(m_RenderWindow3)->InsertForegroundRenderer(m_ForegroundRenderer3,true);
    // initialize 2D measurement point set
    m_MeasurementPointSet2D = mitk::PointSet::New();
    mitk::DataNode::Pointer measurementNode2D = mitk::DataNode::New();
    measurementNode2D->SetName("Measurement PointSet 2D");
    measurementNode2D->SetBoolProperty("helper object",true);
    measurementNode2D->SetBoolProperty("show contour",true);
    measurementNode2D->SetVisibility(false,stdMultiWidget->mitkWidget4->GetRenderer());
    measurementNode2D->SetData(m_MeasurementPointSet2D);
    dataStorage->Add(measurementNode2D);
    m_MeasurementPointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor",measurementNode2D,2);
    // create observer for m_MeasurementPointSet2D
    itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer measurementPointSetChangedCommand;
    measurementPointSetChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
    measurementPointSetChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::MeasurementPointSetChanged);
    m_MeasurementPointSetChangedObserverTag = m_MeasurementPointSet2D->AddObserver(itk::ModifiedEvent(), measurementPointSetChangedCommand);
    // initialize 3D measurement PointSet
    m_MeasurementPointSet3DNode = mitk::DataNode::New();
    m_MeasurementPointSet3DNode->SetName("Measurement PointSet 3D");
    m_MeasurementPointSet3DNode->SetBoolProperty("helper object",true);
    m_MeasurementPointSet3DNode->SetBoolProperty("show contour",true);
    m_MeasurementPointSet3DNode->SetFloatProperty("pointsize",5.0f);
    mitk::PointSet::Pointer measurementPointSet3D = mitk::PointSet::New();
    m_MeasurementPointSet3DNode->SetData(measurementPointSet3D);
    dataStorage->Add(m_MeasurementPointSet3DNode);
    // initialize PointSets
    if(!dataStorage->Exists(dataStorage->GetNamedNode("ToF PointSet 2D")))
    {
      m_PointSet2D = mitk::PointSet::New();
      mitk::DataNode::Pointer pointSet2DNode = mitk::DataNode::New();
      pointSet2DNode->SetName("ToF PointSet 2D");
      pointSet2DNode->SetVisibility(false,stdMultiWidget->mitkWidget4->GetRenderer());
      pointSet2DNode->SetData(m_PointSet2D);
      dataStorage->Add(pointSet2DNode);
      m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor",pointSet2DNode);
    }
    // create observer for m_MeasurementPointSet2D
    itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer pointSetChangedCommand;
    pointSetChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
    pointSetChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::PointSetChanged);
    m_PointSetChangedObserverTag = m_PointSet2D->AddObserver(itk::ModifiedEvent(), pointSetChangedCommand);
    // initialize 3D point set
    if(!dataStorage->Exists(dataStorage->GetNamedNode("ToF PointSet 3D")))
    {
      m_PointSet3DNode = mitk::DataNode::New();
      m_PointSet3DNode->SetName("ToF PointSet 3D");
      m_PointSet3DNode->SetFloatProperty("pointsize",5.0f);
      mitk::PointSet::Pointer pointSet3D = mitk::PointSet::New();
      m_PointSet3DNode->SetData(pointSet3D);
      dataStorage->Add(m_PointSet3DNode);
    }
  }
}

void QmitkToFPointSetWidget::SetCameraIntrinsics(mitk::CameraIntrinsics::Pointer cameraIntrinsics)
{
  m_CameraIntrinsics = cameraIntrinsics;
}

void QmitkToFPointSetWidget::OnMeasurement()
{
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
  int windowHeight = m_MultiWidget->mitkWidget1->GetRenderer()->GetSizeY();
  this->m_VtkTextActor->SetDisplayPosition(10,windowHeight-30);
  // check if points are inside the image range
  int imageSizeX = m_DistanceImage->GetDimensions()[0];
  int imageSizeY = m_DistanceImage->GetDimensions()[1];
  mitk::Point3D point1 = m_MeasurementPointSet2D->GetPoint(0);
  mitk::Point3D point2 = m_MeasurementPointSet2D->GetPoint(1);
  if (m_MeasurementPointSet2D->GetSize()>0)
  {
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
