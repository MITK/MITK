/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFiducialRegistrationWidget.h"

#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkPoints.h>
#include <vtkLandmarkTransform.h>
#include <mitkStaticIGTHelperFunctions.h>

#include <mitkRenderingManager.h>

#define FRW_LOG MITK_INFO("Fiducial Registration Widget")
#define FRW_WARN MITK_WARN("Fiducial Registration Widget")
#define FRW_DEBUG MITK_DEBUG("Fiducial Registration Widget")

/* VIEW MANAGEMENT */
QmitkFiducialRegistrationWidget::QmitkFiducialRegistrationWidget(QWidget* parent)
: QWidget(parent), m_Controls(nullptr),m_MultiWidget(nullptr), m_ImageFiducialsNode(nullptr), m_TrackerFiducialsNode(nullptr)
{
  CreateQtPartControl(this);
}


QmitkFiducialRegistrationWidget::~QmitkFiducialRegistrationWidget()
{
  m_Controls = nullptr;

  //clean up data nodes
  if (m_DataStorage.IsNotNull())
  {
    if (m_ImageFiducialsNode.IsNotNull()) m_DataStorage->Remove(m_ImageFiducialsNode);
    if (m_TrackerFiducialsNode.IsNotNull()) m_DataStorage->Remove(m_TrackerFiducialsNode);
  }
}


void QmitkFiducialRegistrationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkFiducialRegistrationWidget;
    m_Controls->setupUi(parent);

    // hide additional image fiducial button
    m_Controls->m_AddImageFiducialBtn->setHidden(true);

    m_Controls->m_spaceHolderGroupBox->setStyleSheet("QGroupBox {border: 0px transparent;}");
    m_Controls->m_spaceHolderGroupBox2->setStyleSheet("QGroupBox {border: 0px transparent;}");

    this->CreateConnections();
  }
}


void QmitkFiducialRegistrationWidget::CreateConnections()
{
  connect( (QObject*)(m_Controls->m_AddTrackingFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedTrackingFiducial()) );
  connect((QObject*)(m_Controls->m_AddTrackingFiducialBtn), SIGNAL(clicked()), this, SLOT(AddTrackerPoint()));
  connect( (QObject*)(m_Controls->m_AddImageFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedImageFiducial()) );
  connect( (QObject*)(m_Controls->m_RegisterFiducialsBtn), SIGNAL(clicked()), this, SIGNAL(PerformFiducialRegistration()) );
  connect((QObject*)(m_Controls->m_RegisterFiducialsBtn), SIGNAL(clicked()), this, SLOT(Register()));
  connect( (QObject*)(m_Controls->m_UseICPRegistration), SIGNAL(toggled(bool)), this, SIGNAL(FindFiducialCorrespondences(bool)) );

  //unselects the edit button of the other widget if one is selected
  connect( (QObject*)(m_Controls->m_RegistrationImagePoints), SIGNAL(EditPointSets(bool)), this, SLOT(DisableEditButtonRegistrationTrackingPoints(bool)));
  connect( (QObject*)(m_Controls->m_RegistrationTrackingPoints), SIGNAL(EditPointSets(bool)), this, SLOT(DisableEditButtonRegistrationImagePoints(bool)));
}

void QmitkFiducialRegistrationWidget::DisableEditButtonRegistrationImagePoints(bool activated)
{
if (activated) m_Controls->m_RegistrationImagePoints->UnselectEditButton();
}
void QmitkFiducialRegistrationWidget::DisableEditButtonRegistrationTrackingPoints(bool activated)
{
if (activated) m_Controls->m_RegistrationTrackingPoints->UnselectEditButton();
}

void QmitkFiducialRegistrationWidget::SetWidgetAppearanceMode(WidgetAppearanceMode widgetMode)
{
  if (widgetMode==LANDMARKMODE)
  {
    this->HideContinousRegistrationRadioButton(true);
    this->HideStaticRegistrationRadioButton(true);
    this->HideFiducialRegistrationGroupBox();
    this->HideUseICPRegistrationCheckbox(true);
    this->HideImageFiducialButton(false);
    this->m_Controls->sourceLandmarksGroupBox->setTitle("Target/Reference landmarks");
    this->m_Controls->targetLandmarksGroupBox->setTitle("Source Landmarks");
    this->m_Controls->m_AddImageFiducialBtn->setText("Add target landmark");
    this->m_Controls->m_AddTrackingFiducialBtn->setText("Add source landmark");
  }
  else if (widgetMode==FIDUCIALMODE)
  {
    this->HideContinousRegistrationRadioButton(false);
    this->HideStaticRegistrationRadioButton(false);
    this->HideFiducialRegistrationGroupBox();
    this->HideUseICPRegistrationCheckbox(false);
    this->HideImageFiducialButton(true);
    this->m_Controls->sourceLandmarksGroupBox->setTitle("Image fiducials");
    this->m_Controls->targetLandmarksGroupBox->setTitle("OR fiducials");
    this->m_Controls->m_AddImageFiducialBtn->setText("Add image fiducial");
    this->m_Controls->m_AddTrackingFiducialBtn->setText("Add current instrument position");
  }
}

void QmitkFiducialRegistrationWidget::SetQualityDisplayText( QString text )
{
  if (text == nullptr)
    return;
  m_Controls->m_RegistrationQualityDisplay->setText(text); // set text on the QLabel
}

bool QmitkFiducialRegistrationWidget::UseICPIsChecked()
{
  if(m_Controls->m_UseICPRegistration->isChecked())
    return true;
  else
    return false;
}

void QmitkFiducialRegistrationWidget::SetImageFiducialsNode( mitk::DataNode::Pointer imageFiducialsNode )
{
  if(imageFiducialsNode.IsNull())
  {
    FRW_WARN<< "tracker fiducial node is nullptr";
    return;
  }
  m_Controls->m_RegistrationImagePoints->SetPointSetNode(imageFiducialsNode); // pass node to pointListWidget
  if(m_MultiWidget == nullptr)
  {
    MITK_DEBUG<< "MultiWidget is nullptr";
    return;
  }
  m_Controls->m_RegistrationImagePoints->SetMultiWidget(m_MultiWidget); // pass multiWidget to pointListWidget
}

void QmitkFiducialRegistrationWidget::SetTrackerFiducialsNode( mitk::DataNode::Pointer trackerFiducialsNode )
{
  if(trackerFiducialsNode.IsNull())
  {
    FRW_WARN<< "tracker fiducial node is nullptr";
    return;
  }
  m_Controls->m_RegistrationTrackingPoints->SetPointSetNode(trackerFiducialsNode); // pass node to pointListWidget
  if(m_MultiWidget == nullptr)
  {
    MITK_DEBUG<< "MultiWidget is nullptr";
    return;
  }
  m_Controls->m_RegistrationTrackingPoints->SetMultiWidget(m_MultiWidget); // pass multiWidget to pointListWidget
}

void QmitkFiducialRegistrationWidget::SetMultiWidget( QmitkAbstractMultiWidget* multiWidget )
{
  m_MultiWidget=multiWidget;
}

void QmitkFiducialRegistrationWidget::AddSliceNavigationController(mitk::SliceNavigationController* snc)
{
  m_Controls->m_RegistrationTrackingPoints->AddSliceNavigationController(snc);
  m_Controls->m_RegistrationImagePoints->AddSliceNavigationController(snc);
}

mitk::DataNode::Pointer QmitkFiducialRegistrationWidget::GetImageFiducialsNode()
{
  return m_ImageFiducialsNode;
}

mitk::DataNode::Pointer QmitkFiducialRegistrationWidget::GetTrackerFiducialsNode()
{
  return m_TrackerFiducialsNode;
}

void QmitkFiducialRegistrationWidget::HideStaticRegistrationRadioButton( bool on )
{
  m_Controls->m_rbStaticRegistration->setHidden(on);
  HideFiducialRegistrationGroupBox();
}

void QmitkFiducialRegistrationWidget::HideContinousRegistrationRadioButton( bool on )
{
  m_Controls->m_rbContinousRegistration->setHidden(on);
  HideFiducialRegistrationGroupBox();
}

void QmitkFiducialRegistrationWidget::HideFiducialRegistrationGroupBox()
{
  if (m_Controls->m_rbStaticRegistration->isHidden() && m_Controls->m_rbContinousRegistration->isHidden())
  {
    m_Controls->m_gbFiducialRegistration->setHidden(true);
  }
  else
  {
    m_Controls->m_gbFiducialRegistration->setHidden(false);
  }
}

void QmitkFiducialRegistrationWidget::HideUseICPRegistrationCheckbox( bool on )
{
  m_Controls->m_UseICPRegistration->setHidden(on);
}

void QmitkFiducialRegistrationWidget::HideImageFiducialButton( bool on )
{
  m_Controls->m_AddImageFiducialBtn->setHidden(on);
  AdjustButtonSpacing();

}

void QmitkFiducialRegistrationWidget::HideTrackingFiducialButton( bool on )
{
  m_Controls->m_AddTrackingFiducialBtn->setHidden(on);
  AdjustButtonSpacing();
}

void QmitkFiducialRegistrationWidget::AdjustButtonSpacing()
{
  if (m_Controls->m_AddImageFiducialBtn->isHidden() && m_Controls->m_AddTrackingFiducialBtn->isHidden())
  {
    m_Controls->m_spaceHolderGroupBox->setHidden(true);
    m_Controls->m_spaceHolderGroupBox2->setHidden(true);
  }
  else
  {
    m_Controls->m_spaceHolderGroupBox->setHidden(false);
    m_Controls->m_spaceHolderGroupBox2->setHidden(false);
  }
}

void QmitkFiducialRegistrationWidget::SetSourceLandmarkName(QString sourceLandmarkName)
{
  QString groupBoxTitle = sourceLandmarkName;
  groupBoxTitle.append(" Landmarks");
  m_Controls->sourceLandmarksGroupBox->setTitle(groupBoxTitle);
  QString buttonText = "Add ";
  buttonText.append(sourceLandmarkName);
  buttonText.append(" Landmark");
  m_Controls->m_AddImageFiducialBtn->setText(buttonText);
}

void QmitkFiducialRegistrationWidget::SetTargetLandmarkName(QString targetLandmarkName)
{
  QString groupBoxTitle = targetLandmarkName;
  groupBoxTitle.append(" Landmarks");
  m_Controls->targetLandmarksGroupBox->setTitle(groupBoxTitle);
  QString buttonText = "Add ";
  buttonText.append(targetLandmarkName);
  buttonText.append(" Landmark");
  m_Controls->m_AddTrackingFiducialBtn->setText(buttonText);
}

void QmitkFiducialRegistrationWidget::setImageNode(mitk::DataNode::Pointer i)
{
  m_ImageNode = i;
}

void QmitkFiducialRegistrationWidget::setTrackerNavigationData(mitk::NavigationData::Pointer t)
{
  m_TrackerNavigationData = t;
}

void QmitkFiducialRegistrationWidget::setDataStorage(mitk::DataStorage::Pointer d)
{
  m_DataStorage = d;

  mitk::DataNode::Pointer ImageFiducialsNode = mitk::DataNode::New();
  mitk::PointSet::Pointer imagePointSet = mitk::PointSet::New();
  ImageFiducialsNode->SetData(imagePointSet);
  ImageFiducialsNode->SetName("Image Point Set");
  m_DataStorage->Add(ImageFiducialsNode);
  this->SetImageFiducialsNode(ImageFiducialsNode);
  m_ImageFiducialsNode = ImageFiducialsNode;

  mitk::DataNode::Pointer TrackerFiducialsNode = mitk::DataNode::New();
  mitk::PointSet::Pointer trackerPointSet = mitk::PointSet::New();
  TrackerFiducialsNode->SetData(trackerPointSet);
  TrackerFiducialsNode->SetName("Tracker Point Set");
  m_DataStorage->Add(TrackerFiducialsNode);
  this->SetTrackerFiducialsNode(TrackerFiducialsNode);
  m_TrackerFiducialsNode = TrackerFiducialsNode;
}

void QmitkFiducialRegistrationWidget::AddTrackerPoint()
{
  if (m_DataStorage.IsNull()) { return ; } //here the widget should simply do nothing (for backward compatibility)
  else if (m_TrackerNavigationData.IsNull() || m_TrackerFiducialsNode.IsNull()) { MITK_WARN << "Tracker node not correctly initialized"; return; }
  mitk::PointSet::Pointer ps = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsNode->GetData());
  ps->InsertPoint(ps->GetSize(), m_TrackerNavigationData->GetPosition());
}

bool QmitkFiducialRegistrationWidget::CheckRegistrationInitialization()
{
  if (m_DataStorage.IsNull()) { return false; } //here the widget should simply do nothing (for backward compatibility)
  else if ( m_ImageFiducialsNode.IsNull() ||
      m_TrackerFiducialsNode.IsNull()
      ) {MITK_WARN << "Registration not correctly initialized"; return false;}
  else {return true;}
}

void QmitkFiducialRegistrationWidget::Register()
{
  //Check for initialization
  if (!CheckRegistrationInitialization()) return;

  /* retrieve fiducials */
  mitk::PointSet::Pointer imageFiducials = dynamic_cast<mitk::PointSet*>(m_ImageFiducialsNode->GetData());
  mitk::PointSet::Pointer trackerFiducials = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsNode->GetData());
  if (trackerFiducials->GetSize() != imageFiducials->GetSize())
  {
    MITK_WARN << "Not the same number of fiducials, cannot register";
    return;
  }
  else if (trackerFiducials->GetSize() < 3)
  {
    MITK_WARN << "Need at least 3 fiducials, cannot register";
    return;
  }

  //############### conversion to vtk data types (we will use the vtk landmark based transform) ##########################
  //convert point sets to vtk poly data
  vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
  for (int i = 0; i<imageFiducials->GetSize(); i++)
  {
    double point[3] = { imageFiducials->GetPoint(i)[0], imageFiducials->GetPoint(i)[1], imageFiducials->GetPoint(i)[2] };
    sourcePoints->InsertNextPoint(point);
    double point_targets[3] = { trackerFiducials->GetPoint(i)[0], trackerFiducials->GetPoint(i)[1], trackerFiducials->GetPoint(i)[2] };
    targetPoints->InsertNextPoint(point_targets);
  }

  //########################### here, the actual transform is computed ##########################
  //compute transform
  vtkSmartPointer<vtkLandmarkTransform> transform = vtkSmartPointer<vtkLandmarkTransform>::New();
  transform->SetSourceLandmarks(sourcePoints);
  transform->SetTargetLandmarks(targetPoints);
  transform->SetModeToRigidBody();
  transform->Modified();
  transform->Update();
  //compute FRE of transform

  double FRE = mitk::StaticIGTHelperFunctions::ComputeFRE(imageFiducials, trackerFiducials, transform);
  this->SetQualityDisplayText("FRE: " + QString::number(FRE) + " mm");

  //#############################################################################################

  //############### conversion back to itk/mitk data types ##########################
  //convert from vtk to itk data types
  itk::Matrix<float, 3, 3> rotationFloat = itk::Matrix<float, 3, 3>();
  itk::Vector<float, 3> translationFloat = itk::Vector<float, 3>();
  itk::Matrix<double, 3, 3> rotationDouble = itk::Matrix<double, 3, 3>();
  itk::Vector<double, 3> translationDouble = itk::Vector<double, 3>();

  vtkSmartPointer<vtkMatrix4x4> m = transform->GetMatrix();
  for (int k = 0; k<3; k++) for (int l = 0; l<3; l++)
  {
    rotationFloat[k][l] = m->GetElement(k, l);
    rotationDouble[k][l] = m->GetElement(k, l);

  }
  for (int k = 0; k<3; k++)
  {
    translationFloat[k] = m->GetElement(k, 3);
    translationDouble[k] = m->GetElement(k, 3);
  }
  //create affine transform 3D surface
  mitk::AffineTransform3D::Pointer mitkTransform = mitk::AffineTransform3D::New();
  mitkTransform->SetMatrix(rotationDouble);
  mitkTransform->SetOffset(translationDouble);
  //#############################################################################################

  //############### object is transformed ##########################
  //save transform
  m_T_ObjectReg = mitk::NavigationData::New(mitkTransform); // this is stored in a member because it is needed for permanent registration later on

  //transform surface/image
  //only move image if we have one. Sometimes, this widget is used just to register point sets without images.
  if (m_ImageNode.IsNotNull())
  {
    //first we have to store the original ct image transform to compose it with the new transform later
    mitk::AffineTransform3D::Pointer imageTransform = m_ImageNode->GetData()->GetGeometry()->GetIndexToWorldTransform();
    imageTransform->Compose(mitkTransform);
    mitk::AffineTransform3D::Pointer newImageTransform = mitk::AffineTransform3D::New(); //create new image transform... setting the composed directly leads to an error
    itk::Matrix<mitk::ScalarType, 3, 3> rotationFloatNew = imageTransform->GetMatrix();
    itk::Vector<mitk::ScalarType, 3> translationFloatNew = imageTransform->GetOffset();
    newImageTransform->SetMatrix(rotationFloatNew);
    newImageTransform->SetOffset(translationFloatNew);
    m_ImageNode->GetData()->GetGeometry()->SetIndexToWorldTransform(newImageTransform);
  }

  //If this option is set, each point will be transformed and the acutal coordinates of the points change.
  if (this->m_Controls->m_MoveImagePoints->isChecked())
  {
    mitk::PointSet* pointSet_orig = dynamic_cast<mitk::PointSet*>(m_ImageFiducialsNode->GetData());
    mitk::PointSet::Pointer pointSet_moved = mitk::PointSet::New();

    for (int i = 0; i < pointSet_orig->GetSize(); i++)
    {
      pointSet_moved->InsertPoint(mitkTransform->TransformPoint(pointSet_orig->GetPoint(i)));
    }

    pointSet_orig->Clear();
    for (int i = 0; i < pointSet_moved->GetSize(); i++)
      pointSet_orig->InsertPoint(pointSet_moved->GetPoint(i));
  }

  //Do a global reinit
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
}
