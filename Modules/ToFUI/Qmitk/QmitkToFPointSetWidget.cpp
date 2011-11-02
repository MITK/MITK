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

const std::string QmitkToFPointSetWidget::VIEW_ID = "org.mitk.views.qmitktofpointsetwidget";

QmitkToFPointSetWidget::QmitkToFPointSetWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
, m_CameraIntrinsics(NULL)
, m_OverlayController(NULL)
, m_MeasurementPropertyList(NULL)
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
  delete m_OverlayController;
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
  m_DistanceImage = distanceImage;
  if ((stdMultiWidget!=NULL)&&(dataStorage.IsNotNull()))
  {
    // enable buttons
    m_Controls->pointSetButton->setEnabled(true);
    m_Controls->measureButton->setEnabled(true);
    // initialize overlays
    m_MeasurementPropertyList = mitk::PropertyList::New();
    m_MeasurementPropertyList->SetStringProperty("overlay.text.distance","");
    m_MeasurementPropertyList->SetIntProperty("overlay.fontSize",16);
    QmitkTextOverlay* textOverlay1 = new QmitkTextOverlay("overlay.text.distance");
    m_OverlayController = new QmitkOverlayController(stdMultiWidget->mitkWidget1,m_MeasurementPropertyList);
    m_OverlayController->AddOverlay(textOverlay1);
    m_OverlayController->SetOverlayVisibility(false);
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
    m_PointSet2D = mitk::PointSet::New();
    mitk::DataNode::Pointer pointSet2DNode = mitk::DataNode::New();
    pointSet2DNode->SetName("ToF PointSet 2D");
    pointSet2DNode->SetVisibility(false,stdMultiWidget->mitkWidget4->GetRenderer());
    pointSet2DNode->SetData(m_PointSet2D);
    dataStorage->Add(pointSet2DNode);
    m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor",pointSet2DNode);
    // create observer for m_MeasurementPointSet2D
    itk::SimpleMemberCommand<QmitkToFPointSetWidget>::Pointer pointSetChangedCommand;
    pointSetChangedCommand = itk::SimpleMemberCommand<QmitkToFPointSetWidget>::New();
    pointSetChangedCommand->SetCallbackFunction(this, &QmitkToFPointSetWidget::PointSetChanged);
    m_PointSetChangedObserverTag = m_PointSet2D->AddObserver(itk::ModifiedEvent(), pointSetChangedCommand);
    // initialize 3D point set
    m_PointSet3DNode = mitk::DataNode::New();
    m_PointSet3DNode->SetName("ToF PointSet 3D");
    m_PointSet3DNode->SetFloatProperty("pointsize",5.0f);
    mitk::PointSet::Pointer pointSet3D = mitk::PointSet::New();
    m_PointSet3DNode->SetData(pointSet3D);
    dataStorage->Add(m_PointSet3DNode);
  }
}

void QmitkToFPointSetWidget::SetCameraIntrinsics(mitk::CameraIntrinsics::Pointer cameraIntrinsics)
{
  m_CameraIntrinsics = cameraIntrinsics;
}

void QmitkToFPointSetWidget::OnMeasurement()
{
  // initial update of measurement
  this->MeasurementPointSetChanged();
  if (m_Controls->measureButton->isChecked())
  {
    // uncheck point set button
    m_Controls->pointSetButton->setChecked(false);
    // show overlays
    m_OverlayController->SetOverlayVisibility(true);
    // enable interactor
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_MeasurementPointSetInteractor);
  }
  else
  {
    // hide overlays
    m_OverlayController->SetOverlayVisibility(false);
    // disable interactor
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_MeasurementPointSetInteractor);
  }
}

void QmitkToFPointSetWidget::OnPointSet()
{
  // initial update of PointSet
  this->PointSetChanged();
  if (m_Controls->pointSetButton->isChecked())
  {
    // uncheck point set button
    m_Controls->measureButton->setChecked(false);
    // hide overlays
    m_OverlayController->SetOverlayVisibility(false);
    // enable interactor
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
  }
  else
  {
    // disable interactor
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }
}

void QmitkToFPointSetWidget::MeasurementPointSetChanged()
{
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
        m_MeasurementPropertyList->SetStringProperty("overlay.text.distance",stream.str().c_str());
      }
    }
    else
    {
      m_MeasurementPropertyList->SetStringProperty("overlay.text.distance","Measurement outside image range.");
    }
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
      m_OverlayController->SetOverlayVisibility(false);
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
    }
    else
    {
      m_OverlayController->SetOverlayVisibility(true);
      m_MeasurementPropertyList->SetStringProperty("overlay.text.distance","PointSet outside image range.");
    }
  }
}
