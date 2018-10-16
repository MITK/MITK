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

#include "QmitkUSNavigationStepCtUsRegistration.h"
#include "ui_QmitkUSNavigationStepCtUsRegistration.h"

#include "mitkNodeDisplacementFilter.h"
#include "../QmitkUSNavigationMarkerPlacement.h"



#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include "mitkProperties.h"

#include <mitkRenderingManager.h>
#include <mitkStaticIGTHelperFunctions.h>

#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>


QmitkUSNavigationStepCtUsRegistration::QmitkUSNavigationStepCtUsRegistration(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  ui(new Ui::QmitkUSNavigationStepCtUsRegistration)
{
  this->UnsetFloatingImageGeometry();
  this->DefineDataStorageImageFilter();
  this->CreateQtPartControl(this);
}


QmitkUSNavigationStepCtUsRegistration::~QmitkUSNavigationStepCtUsRegistration()
{
  delete ui;
}

bool QmitkUSNavigationStepCtUsRegistration::OnStartStep()
{
  MITK_INFO << "OnStartStep()";
  return true;
}

bool QmitkUSNavigationStepCtUsRegistration::OnStopStep()
{
  MITK_INFO << "OnStopStep()";
  return true;
}

bool QmitkUSNavigationStepCtUsRegistration::OnFinishStep()
{
  MITK_INFO << "OnFinishStep()";
  return true;
}

bool QmitkUSNavigationStepCtUsRegistration::OnActivateStep()
{
  MITK_INFO << "OnActivateStep()";
  ui->floatingImageComboBox->SetDataStorage(this->GetDataStorage());
  ui->fiducialMarkerModelPointSetComboBox->SetDataStorage(this->GetDataStorage());
  ui->devMarkerComboBox->SetDataStorage(this->GetDataStorage());
  return true;
}

bool QmitkUSNavigationStepCtUsRegistration::OnDeactivateStep()
{
  MITK_INFO << "OnDeactivateStep()";
  return true;
}

void QmitkUSNavigationStepCtUsRegistration::OnUpdate()
{
  if (m_NavigationDataSource.IsNull()) { return; }

  m_NavigationDataSource->Update();
}

void QmitkUSNavigationStepCtUsRegistration::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
 
}

QString QmitkUSNavigationStepCtUsRegistration::GetTitle()
{
  return "CT-to-US registration";
}

QmitkUSAbstractNavigationStep::FilterVector QmitkUSNavigationStepCtUsRegistration::GetFilter()
{
  return FilterVector();
}

void QmitkUSNavigationStepCtUsRegistration::OnSetCombinedModality()
{
  mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality = this->GetCombinedModality(false);
  if (combinedModality.IsNotNull())
  {
    m_NavigationDataSource = combinedModality->GetNavigationDataSource();
  }

}

void QmitkUSNavigationStepCtUsRegistration::UnsetFloatingImageGeometry()
{
  m_ImageDimension[0] = 0;
  m_ImageDimension[1] = 0;
  m_ImageDimension[2] = 0;

  m_ImageSpacing[0] = 1;
  m_ImageSpacing[1] = 1;
  m_ImageSpacing[2] = 1;
}

void QmitkUSNavigationStepCtUsRegistration::SetFloatingImageGeometryInformation(mitk::Image * image)
{
  m_ImageDimension[0] = image->GetDimension(0);
  m_ImageDimension[1] = image->GetDimension(1);
  m_ImageDimension[2] = image->GetDimension(2);

  m_ImageSpacing[0] = image->GetGeometry()->GetSpacing()[0];
  m_ImageSpacing[1] = image->GetGeometry()->GetSpacing()[1];
  m_ImageSpacing[2] = image->GetGeometry()->GetSpacing()[2];
}

void QmitkUSNavigationStepCtUsRegistration::DefineDataStorageImageFilter()
{
  m_IsAPointSetPredicate = mitk::TNodePredicateDataType<mitk::PointSet>::New();
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

  auto isSegmentation = mitk::NodePredicateDataType::New("Segment");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(mitk::NodePredicateAnd::New(isImage, mitk::NodePredicateNot::New(isSegmentation)));

  mitk::NodePredicateNot::Pointer isNotAHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));

  m_IsOfTypeImagePredicate = mitk::NodePredicateAnd::New(validImages, isNotAHelperObject);

  mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateNot::Pointer isNotBinaryPredicate = mitk::NodePredicateNot::New(isBinaryPredicate);

  mitk::NodePredicateAnd::Pointer isABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, isBinaryPredicate);
  mitk::NodePredicateAnd::Pointer isNotABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, isNotBinaryPredicate);

  m_IsASegmentationImagePredicate = mitk::NodePredicateOr::New(isABinaryImagePredicate, mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_IsAPatientImagePredicate = mitk::NodePredicateAnd::New(isNotABinaryImagePredicate, mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New()));
}

void QmitkUSNavigationStepCtUsRegistration::CreateQtPartControl(QWidget *parent)
{
  ui->setupUi(parent);
  ui->floatingImageComboBox->SetPredicate(m_IsAPatientImagePredicate);
  ui->fiducialMarkerModelPointSetComboBox->SetPredicate(m_IsAPointSetPredicate);
  ui->devMarkerComboBox->SetPredicate(m_IsAPointSetPredicate);

  // create signal/slot connections
  connect(ui->floatingImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(ui->fiducialMarkerModelPointSetComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnFiducialMarkerModelComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(ui->devMarkerComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnDevComboBoxChanged(const mitk::DataNode*)));
  connect(ui->doRegistrationMarkerToImagePushButton, SIGNAL(clicked()),
    this, SLOT(OnRegisterMarkerToFloatingImageCS()));
}

void QmitkUSNavigationStepCtUsRegistration::OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode* node)
{
  MITK_INFO << "OnFloatingImageComboBoxSelectionChanged()";

  if (m_FloatingImage.IsNotNull())
  {
    //TODO: Define, what will happen if the imageCT is not null...
  }

  if (node == nullptr)
  {
    this->UnsetFloatingImageGeometry();
    m_FloatingImage = nullptr;
    return;
  }

  mitk::DataNode* selectedFloatingImage = ui->floatingImageComboBox->GetSelectedNode();
  if (selectedFloatingImage == nullptr)
  {
    this->UnsetFloatingImageGeometry();
    m_FloatingImage = nullptr;
    return;
  }

  mitk::Image::Pointer floatingImage = dynamic_cast<mitk::Image*>(selectedFloatingImage->GetData());
  if (floatingImage.IsNull())
  {
    MITK_WARN << "Failed to cast selected segmentation node to mitk::Image*";
    this->UnsetFloatingImageGeometry();
    m_FloatingImage = nullptr;
    return;
  }

  m_FloatingImage = floatingImage;
  this->SetFloatingImageGeometryInformation(floatingImage.GetPointer());
}

void QmitkUSNavigationStepCtUsRegistration::OnFiducialMarkerModelComboBoxSelectionChanged(const mitk::DataNode * node)
{
  MITK_INFO << "OnFiducialMarkerModelComboBoxSelectionChanged()";

  if (m_MarkerModelCoordinateSystemPointSet.IsNotNull())
  {
    //TODO: Define, what will happen if the pointSet is not null...
  }

  if (node == nullptr)
  {
    m_MarkerModelCoordinateSystemPointSet = nullptr;
    return;
  }

  mitk::DataNode* selectedPointSet = ui->fiducialMarkerModelPointSetComboBox->GetSelectedNode();
  if (selectedPointSet == nullptr)
  {
    m_MarkerModelCoordinateSystemPointSet = nullptr;
    return;
  }

  mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(selectedPointSet->GetData());
  if (pointSet.IsNull())
  {
    MITK_WARN << "Failed to cast selected pointset node to mitk::PointSet*";
    m_MarkerModelCoordinateSystemPointSet = nullptr;
    return;
  }

  m_MarkerModelCoordinateSystemPointSet = pointSet;
}

void QmitkUSNavigationStepCtUsRegistration::OnDevComboBoxChanged(const mitk::DataNode * node)
{
  if (node == nullptr)
  {
    m_MarkerFloatingImageCoordinateSystemPointSet = nullptr;
    return;
  }

  mitk::DataNode* selectedPointSet = ui->devMarkerComboBox->GetSelectedNode();
  if (selectedPointSet == nullptr)
  {
    m_MarkerFloatingImageCoordinateSystemPointSet = nullptr;
    return;
  }

  mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(selectedPointSet->GetData());
  if (pointSet.IsNull())
  {
    MITK_WARN << "Failed to cast selected pointset node to mitk::PointSet*";
    m_MarkerFloatingImageCoordinateSystemPointSet = nullptr;
    return;
  }

  m_MarkerFloatingImageCoordinateSystemPointSet = pointSet;
}

void QmitkUSNavigationStepCtUsRegistration::OnRegisterMarkerToFloatingImageCS()
{
  //Check for initialization
  if( m_MarkerModelCoordinateSystemPointSet.IsNull() ||
      m_MarkerFloatingImageCoordinateSystemPointSet.IsNull() )
  {
    MITK_WARN << "Fiducial Landmarks are not initialized yet, cannot register";
    return;
  }

  //Retrieve fiducials
  if (m_MarkerFloatingImageCoordinateSystemPointSet->GetSize() != m_MarkerModelCoordinateSystemPointSet->GetSize())
  {
    MITK_WARN << "Not the same number of fiducials, cannot register";
    return;
  }
  else if (m_MarkerFloatingImageCoordinateSystemPointSet->GetSize() < 3)
  {
    MITK_WARN << "Need at least 3 fiducials, cannot register";
    return;
  }

  //############### conversion to vtk data types (we will use the vtk landmark based transform) ##########################
  //convert point sets to vtk poly data
  vtkSmartPointer<vtkPoints> sourcePoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPoints> targetPoints = vtkSmartPointer<vtkPoints>::New();
  for (int i = 0; i<m_MarkerModelCoordinateSystemPointSet->GetSize(); i++)
  {
    double point[3] = { m_MarkerModelCoordinateSystemPointSet->GetPoint(i)[0], 
                        m_MarkerModelCoordinateSystemPointSet->GetPoint(i)[1], 
                        m_MarkerModelCoordinateSystemPointSet->GetPoint(i)[2] };
    sourcePoints->InsertNextPoint(point);

    double point_targets[3] = { m_MarkerFloatingImageCoordinateSystemPointSet->GetPoint(i)[0], 
                                m_MarkerFloatingImageCoordinateSystemPointSet->GetPoint(i)[1], 
                                m_MarkerFloatingImageCoordinateSystemPointSet->GetPoint(i)[2] };
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

  double FRE = mitk::StaticIGTHelperFunctions::ComputeFRE(m_MarkerModelCoordinateSystemPointSet, m_MarkerFloatingImageCoordinateSystemPointSet, transform);
  MITK_INFO << "FRE: " << FRE << " mm";

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
  //################################################################

  //############### object is transformed ##########################
  //save transform
  //this is stored in a member because it is needed for permanent registration later on
  m_TransformMarkerCSToFloatingImageCS = mitk::NavigationData::New(mitkTransform); 
  MITK_INFO << mitkTransform;
                                                            //transform surface/image
                                                            //only move image if we have one. Sometimes, this widget is used just to register point sets without images.
  /*if (m_ImageNode.IsNotNull())
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
  }*/

  //If this option is set, each point will be transformed and the acutal coordinates of the points change.
  /*if (this->m_Controls->m_MoveImagePoints->isChecked())
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
  }*/

  //Do a global reinit
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

}


