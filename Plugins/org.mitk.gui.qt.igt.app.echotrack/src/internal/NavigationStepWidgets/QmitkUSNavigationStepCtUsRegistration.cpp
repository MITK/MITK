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
#include <mitkImage.h>
#include <mitkImageCast.h>

#include <mitkRenderingManager.h>
#include <mitkStaticIGTHelperFunctions.h>

#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>

#include <itkThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkFlatStructuringElement.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkXorImageFilter.h>
#include <itkWhiteTopHatImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkGeometryUtilities.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkImage.h>
#include <mitkImagePixelReadAccessor.h>

// Declare typedefs:
typedef itk::Image<int, 3>  ImageType;
typedef itk::BinaryThresholdImageFilter <ImageType, ImageType> BinaryThresholdImageFilterType;
typedef itk::BinaryBallStructuringElement<ImageType::PixelType, ImageType::ImageDimension> StructuringElementType;
typedef itk::BinaryMorphologicalOpeningImageFilter <ImageType, ImageType, StructuringElementType> BinaryMorphologicalOpeningImageFilterType;
typedef itk::XorImageFilter <ImageType> XorImageFilterType;
typedef itk::WhiteTopHatImageFilter<ImageType, ImageType, StructuringElementType> WhiteTopHatImageFilterType;
typedef itk::GradientMagnitudeImageFilter<ImageType, ImageType> GradientMagnitudeImageFilterType;
typedef itk::LaplacianRecursiveGaussianImageFilter<ImageType, ImageType> LaplacianRecursiveGaussianImageFilterType;
typedef itk::VotingBinaryIterativeHoleFillingImageFilter<ImageType> VotingBinaryIterativeHoleFillingImageFilterType;
typedef itk::BinaryImageToShapeLabelMapFilter<ImageType> BinaryImageToShapeLabelMapFilterType;



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

double QmitkUSNavigationStepCtUsRegistration::GetVoxelVolume()
{
  if (m_FloatingImage.IsNull())
  {
    return 0.0;
  }

  return m_ImageSpacing[0] * m_ImageSpacing[1] * m_ImageSpacing[2];
}

double QmitkUSNavigationStepCtUsRegistration::GetFiducialVolume(double radius)
{
  return 1.333333333 * 3.141592 * (radius * radius * radius);
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
  connect(ui->filterTestsPushButton, SIGNAL(clicked()),
    this, SLOT(OnFilterFloatingImage()));
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
  //create mitk affine transform 3D and save it to the class member
  m_TransformMarkerCSToFloatingImageCS = mitk::AffineTransform3D::New();
  m_TransformMarkerCSToFloatingImageCS->SetMatrix(rotationDouble);
  m_TransformMarkerCSToFloatingImageCS->SetOffset(translationDouble);
  MITK_INFO << m_TransformMarkerCSToFloatingImageCS;
  //################################################################

  //############### object is transformed ##########################
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

void QmitkUSNavigationStepCtUsRegistration::OnFilterFloatingImage()
{
  if (m_FloatingImage.IsNull())
  {
    return;
  }
  
  /* Evtl. für später (da FlatStructuringElement genauer ist als BinaryBallStructuringElement:
  typedef itk::FlatStructuringElement<3> FlatStructuringElementType;
  FlatStructuringElementType::RadiusType radiusType;
  radiusType.Fill(10);
  FlatStructuringElementType ballType = FlatStructuringElementType::Ball(radiusType, true);
  */
  
  //Initialize threshold filters
  itk::ThresholdImageFilter<itk::Image<int,3>>::Pointer thresholdFilter = itk::ThresholdImageFilter<itk::Image<int, 3>>::New();
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetLower(500);
  thresholdFilter->SetUpper(3200);

  /*
  itk::ThresholdImageFilter<itk::Image<int, 3>>::Pointer thresholdFilter2 = itk::ThresholdImageFilter<itk::Image<int, 3>>::New();
  thresholdFilter2->SetOutsideValue(0);
  thresholdFilter2->SetLower(350);
  thresholdFilter2->SetUpper(10000);*/

  
  //Initialize binary threshold filter 1
  BinaryThresholdImageFilterType::Pointer binaryThresholdFilter1 = BinaryThresholdImageFilterType::New();
  binaryThresholdFilter1->SetOutsideValue(0);
  binaryThresholdFilter1->SetInsideValue(1);
  binaryThresholdFilter1->SetLowerThreshold(350);
  binaryThresholdFilter1->SetUpperThreshold(10000);

  /*
  BinaryThresholdImageFilterType::Pointer binaryThresholdFilter2 = BinaryThresholdImageFilterType::New();
  binaryThresholdFilter2->SetOutsideValue(0);
  binaryThresholdFilter2->SetInsideValue(1);
  binaryThresholdFilter2->SetLowerThreshold(500);
  binaryThresholdFilter2->SetUpperThreshold(3200);*/

  
  //Initialize structuring element, which is in this case a ball of radius X
  StructuringElementType structuringElement;
  //radius is in pixel|voxel size as direct neighbourhood members
  structuringElement.SetRadius(1); 
  structuringElement.CreateStructuringElement();
  
  
  //Initialize binary morphological opening filter
  BinaryMorphologicalOpeningImageFilterType::Pointer binaryOpeningFilter = BinaryMorphologicalOpeningImageFilterType::New();
  binaryOpeningFilter->SetKernel(structuringElement);
  binaryOpeningFilter->SetForegroundValue(1);
  binaryOpeningFilter->SetBackgroundValue(0);

  /*
  //Initialize white top hat image filter
  WhiteTopHatImageFilterType::Pointer whiteTopHatFilter = WhiteTopHatImageFilterType::New();
  whiteTopHatFilter->SetKernel(structuringElement);*/

  /*
  //Initialize Xor image filter
  XorImageFilterType::Pointer xorFilter = XorImageFilterType::New();*/

  //Initialize GradientMagnitudeImageFilterType
  GradientMagnitudeImageFilterType::Pointer gradientFilter = GradientMagnitudeImageFilterType::New();
  GradientMagnitudeImageFilterType::Pointer gradientFilter2 = GradientMagnitudeImageFilterType::New();
  LaplacianRecursiveGaussianImageFilterType::Pointer laplacianFilter = LaplacianRecursiveGaussianImageFilterType::New();
  LaplacianRecursiveGaussianImageFilterType::Pointer laplacianFilter2 = LaplacianRecursiveGaussianImageFilterType::New();

  //Initialize binary hole filling filter 
  VotingBinaryIterativeHoleFillingImageFilterType::Pointer holeFillingFilter = VotingBinaryIterativeHoleFillingImageFilterType::New();
  VotingBinaryIterativeHoleFillingImageFilterType::InputSizeType radius;
  radius.Fill(1);
  holeFillingFilter->SetRadius(radius);
  holeFillingFilter->SetBackgroundValue(0);
  holeFillingFilter->SetForegroundValue(1);
  holeFillingFilter->SetMaximumNumberOfIterations(5);


  //Initialize binary image to shape label map filter
  BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
  binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);

  itk::Image<int,3>::Pointer itkImage1 = itk::Image<int, 3>::New();
  mitk::CastToItkImage(m_FloatingImage, itkImage1);
  itk::Image<int, 3>::Pointer itkImage2 = itk::Image<int, 3>::New();
  mitk::CastToItkImage(m_FloatingImage, itkImage2);

  thresholdFilter->SetInput(itkImage1);
  //thresholdFilter->Update();
  //binaryOpeningFilter->SetInput(thresholdFilter->GetOutput());
  //binaryOpeningFilter->Update();
  
  //binaryThresholdFilter1->SetInput(itkImage1);
  //binaryOpeningFilter->SetInput(binaryThresholdFilter1->GetOutput());
  //binaryOpeningFilter->Update();
  
  //whiteTopHatFilter->SetInput(binaryThresholdFilter1->GetOutput());
  //whiteTopHatFilter->Update();

  /*
  //itkImage2 is filtered only by the binary threshold filter
  binaryThresholdFilter2->SetInput(itkImage2);
  binaryThresholdFilter2->Update();

  //Set binary threshold filtered image and binary opening filtered image as inputs to Xor-filter
  xorFilter->SetInput1(binaryThresholdFilter2->GetOutput());
  xorFilter->SetInput2(binaryOpeningFilter->GetOutput());
  xorFilter->Update();*/

  laplacianFilter->SetInput(thresholdFilter->GetOutput());
  laplacianFilter2->SetInput(laplacianFilter->GetOutput());
  binaryThresholdFilter1->SetInput(laplacianFilter2->GetOutput());
  holeFillingFilter->SetInput(binaryThresholdFilter1->GetOutput());
  binaryImageToShapeLabelMapFilter->SetInput(holeFillingFilter->GetOutput());
  binaryImageToShapeLabelMapFilter->Update();

  BinaryImageToShapeLabelMapFilterType::OutputImageType::Pointer labelMap = binaryImageToShapeLabelMapFilter->GetOutput();
  // The output of this filter is an itk::LabelMap, which contains itk::LabelObject's
  MITK_INFO << "There are " << labelMap->GetNumberOfLabelObjects() << " objects.";

  double voxelVolume = this->GetVoxelVolume();
  double fiducialVolume5mm = this->GetFiducialVolume(2.5);
  double fiducialVolume3mm = this->GetFiducialVolume(1.5);
  MITK_INFO << "Voxel Volume = " << voxelVolume << "; Fiducial Volume 5mm = " << fiducialVolume5mm << "; Fiducial Volume 3mm = " << fiducialVolume3mm;
  unsigned int numberOfPixels5mm = ceil(fiducialVolume5mm / voxelVolume);
  unsigned int numberOfPixels3mm = ceil(fiducialVolume3mm / voxelVolume);

  MITK_INFO << "Number of pixels 5mm Fiducial = " << numberOfPixels5mm;
  MITK_INFO << "Number of pixels 3mm Fiducial = " << numberOfPixels3mm;

  itk::Image<int, 3>::Pointer binaryImage = itk::Image<int, 3>::New();
  binaryImage = holeFillingFilter->GetOutput();


  // Loop over each region
  for (int i = labelMap->GetNumberOfLabelObjects() - 1; i >= 0; --i)
  {
    MITK_INFO << "Number " << i;
    // Get the ith region
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelMap->GetNthLabelObject(i);
    MITK_INFO << "Object " << i << " contains " << labelObject->Size() << " pixel";

    //TODO: Threshold-Wert evtl. experimentell besser abstimmen,
    //      um zu verhindern, dass durch Threshold wahre Fiducial-Kandidaten elimiert werden.
    if (labelObject->Size() < numberOfPixels3mm * 0.8)
    {
      for (unsigned int pixelId = 0; pixelId < labelObject->Size(); pixelId++)
      {
        binaryImage->SetPixel(labelObject->GetIndex(pixelId), 0);
      }
      labelMap->RemoveLabelObject(labelObject);
    }
      
  }

  MITK_INFO << "After Removing Objects:";

  for (int i = labelMap->GetNumberOfLabelObjects() - 1; i >= 0; --i)
  {
    // Get the ith region
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelMap->GetNthLabelObject(i);
    MITK_INFO << "Object " << i << " contains " << labelObject->Size() << " pixel";

    mitk::Point3D centroid;
    centroid.CastFrom(labelObject->GetCentroid());
    m_CentroidsOfFiducialCandidates.push_back(centroid);
  }
  //evtl. for later: itk::LabelMapOverlayImageFilter

  mitk::CastToMitkImage(binaryImage, m_FloatingImage);

  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->SetPoint(counter, m_CentroidsOfFiducialCandidates.at(counter)); //Alternativ:  InsertPoint()
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(pointSet);
    node->SetName("PointSet");
    node->SetFloatProperty("pointsize", 5.0);
    this->GetDataStorage()->Add(node);
  }



}


