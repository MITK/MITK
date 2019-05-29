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

#include <QMessageBox>

#include "mitkNodeDisplacementFilter.h"
#include "../Filter/mitkFloatingImageToUltrasoundRegistrationFilter.h"
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

#include <itkImageRegionIterator.h>
#include <itkGeometryUtilities.h>
#include <mitkImagePixelReadAccessor.h>

#include <itkZeroCrossingImageFilter.h>
#include <itkSimpleContourExtractorImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>

static const int NUMBER_FIDUCIALS_NEEDED = 8;

QmitkUSNavigationStepCtUsRegistration::QmitkUSNavigationStepCtUsRegistration(QWidget *parent) :
  QmitkUSAbstractNavigationStep(parent),
  ui(new Ui::QmitkUSNavigationStepCtUsRegistration),
  m_PerformingGroundTruthProtocolEvaluation(false),
  m_FloatingImageToUltrasoundRegistrationFilter(nullptr),
  m_FreezeCombinedModality(false)
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
  ui->ctImagesToChooseComboBox->SetDataStorage(this->GetDataStorage());
  ui->segmentationComboBox->SetDataStorage(this->GetDataStorage());
  ui->selectedSurfaceComboBox->SetDataStorage(this->GetDataStorage());
  ui->pointSetComboBox->SetDataStorage(this->GetDataStorage());
  m_FloatingImageToUltrasoundRegistrationFilter =
    mitk::FloatingImageToUltrasoundRegistrationFilter::New();
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
  m_FloatingImageToUltrasoundRegistrationFilter->Update();
}

void QmitkUSNavigationStepCtUsRegistration::OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode)
{
  Q_UNUSED(settingsNode);
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

  MITK_INFO << "ImageSpacing = " << m_ImageSpacing;
  return m_ImageSpacing[0] * m_ImageSpacing[1] * m_ImageSpacing[2];
}

double QmitkUSNavigationStepCtUsRegistration::GetFiducialVolume(double radius)
{
  return 1.333333333 * 3.141592 * (radius * radius * radius);
}

bool QmitkUSNavigationStepCtUsRegistration::FilterFloatingImage()
{
  if (m_FloatingImage.IsNull())
  {
    return false;
  }

  ImageType::Pointer itkImage1 = ImageType::New();
  mitk::CastToItkImage(m_FloatingImage, itkImage1);

  this->InitializeImageFilters();

  m_ThresholdFilter->SetInput(itkImage1);
  m_LaplacianFilter1->SetInput(m_ThresholdFilter->GetOutput());
  m_LaplacianFilter2->SetInput(m_LaplacianFilter1->GetOutput());
  m_BinaryThresholdFilter->SetInput(m_LaplacianFilter2->GetOutput());
  m_HoleFillingFilter->SetInput(m_BinaryThresholdFilter->GetOutput());
  m_BinaryImageToShapeLabelMapFilter->SetInput(m_HoleFillingFilter->GetOutput());
  m_BinaryImageToShapeLabelMapFilter->Update();

  ImageType::Pointer binaryImage = ImageType::New();
  binaryImage = m_HoleFillingFilter->GetOutput();

  this->EliminateTooSmallLabeledObjects(binaryImage);
  //mitk::CastToMitkImage(binaryImage, m_FloatingImage);
  return true;
}

void QmitkUSNavigationStepCtUsRegistration::InitializeImageFilters()
{
  //Initialize threshold filters
  m_ThresholdFilter = itk::ThresholdImageFilter<ImageType>::New();
  m_ThresholdFilter->SetOutsideValue(0);
  m_ThresholdFilter->SetLower(500);
  m_ThresholdFilter->SetUpper(3200);

  //Initialize binary threshold filter 1
  m_BinaryThresholdFilter = BinaryThresholdImageFilterType::New();
  m_BinaryThresholdFilter->SetOutsideValue(0);
  m_BinaryThresholdFilter->SetInsideValue(1);
  m_BinaryThresholdFilter->SetLowerThreshold(350);
  m_BinaryThresholdFilter->SetUpperThreshold(10000);

  //Initialize laplacian recursive gaussian image filter
  m_LaplacianFilter1 = LaplacianRecursiveGaussianImageFilterType::New();
  m_LaplacianFilter2 = LaplacianRecursiveGaussianImageFilterType::New();

  //Initialize binary hole filling filter
  m_HoleFillingFilter = VotingBinaryIterativeHoleFillingImageFilterType::New();
  VotingBinaryIterativeHoleFillingImageFilterType::InputSizeType radius;
  radius.Fill(1);
  m_HoleFillingFilter->SetRadius(radius);
  m_HoleFillingFilter->SetBackgroundValue(0);
  m_HoleFillingFilter->SetForegroundValue(1);
  m_HoleFillingFilter->SetMaximumNumberOfIterations(5);

  //Initialize binary image to shape label map filter
  m_BinaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
  m_BinaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
}

double QmitkUSNavigationStepCtUsRegistration::GetCharacteristicDistanceAWithUpperMargin()
{
  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
    case 0:
      return 12.07;

    // case 1 is equal to fiducial marker configuration B (15mm distance)
    case 1:
      return 18.105;

    // case 2 is equal to fiducial marker configuration C (20mm distance)
    case 2:
      return 24.14;
  }
  return 0.0;
}

double QmitkUSNavigationStepCtUsRegistration::GetCharacteristicDistanceBWithLowerMargin()
{
  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
  case 0:
    return 12.07;

    // case 1 is equal to fiducial marker configuration B (15mm distance)
  case 1:
    return 18.105;

    // case 2 is equal to fiducial marker configuration C (20mm distance)
  case 2:
    return 24.14;
  }
  return 0.0;
}

double QmitkUSNavigationStepCtUsRegistration::GetCharacteristicDistanceBWithUpperMargin()
{
  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
  case 0:
    return 15.73;

    // case 1 is equal to fiducial marker configuration B (15mm distance)
  case 1:
    return 23.595;

    // case 2 is equal to fiducial marker configuration C (20mm distance)
  case 2:
    return 31.46;
  }
  return 0.0;
}

double QmitkUSNavigationStepCtUsRegistration::GetMinimalFiducialConfigurationDistance()
{
  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
  case 0:
    return 10.0;

    // case 1 is equal to fiducial marker configuration B (15mm distance)
  case 1:
    return 15.0;

    // case 2 is equal to fiducial marker configuration C (20mm distance)
  case 2:
    return 20.0;
  }
  return 0.0;
}

void QmitkUSNavigationStepCtUsRegistration::CreateMarkerModelCoordinateSystemPointSet()
{
  if (m_MarkerModelCoordinateSystemPointSet.IsNull())
  {
    m_MarkerModelCoordinateSystemPointSet = mitk::PointSet::New();
  }
  else
  {
    m_MarkerModelCoordinateSystemPointSet->Clear();
  }

  mitk::Point3D fiducial1;
  mitk::Point3D fiducial2;
  mitk::Point3D fiducial3;
  mitk::Point3D fiducial4;
  mitk::Point3D fiducial5;
  mitk::Point3D fiducial6;
  mitk::Point3D fiducial7;
  mitk::Point3D fiducial8;



  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
  case 0:
    fiducial1[0] = 0;
    fiducial1[1] = 0;
    fiducial1[2] = 0;

    fiducial2[0] = 0;
    fiducial2[1] = 10;
    fiducial2[2] = 0;

    fiducial3[0] = 10;
    fiducial3[1] = 0;
    fiducial3[2] = 0;

    fiducial4[0] = 20;
    fiducial4[1] = 20;
    fiducial4[2] = 0;

    fiducial5[0] = 0;
    fiducial5[1] = 20;
    fiducial5[2] = 10;

    fiducial6[0] = 10;
    fiducial6[1] = 20;
    fiducial6[2] = 10;

    fiducial7[0] = 20;
    fiducial7[1] = 10;
    fiducial7[2] = 10;

    fiducial8[0] = 20;
    fiducial8[1] = 0;
    fiducial8[2] = 10;
    break;
    // case 1 is equal to fiducial marker configuration B (15mm distance)
  case 1:
    fiducial1[0] = 0;
    fiducial1[1] = 0;
    fiducial1[2] = 0;

    fiducial2[0] = 0;
    fiducial2[1] = 15;
    fiducial2[2] = 0;

    fiducial3[0] = 15;
    fiducial3[1] = 0;
    fiducial3[2] = 0;

    fiducial4[0] = 30;
    fiducial4[1] = 30;
    fiducial4[2] = 0;

    fiducial5[0] = 0;
    fiducial5[1] = 30;
    fiducial5[2] = 15;

    fiducial6[0] = 15;
    fiducial6[1] = 30;
    fiducial6[2] = 15;

    fiducial7[0] = 30;
    fiducial7[1] = 15;
    fiducial7[2] = 15;

    fiducial8[0] = 30;
    fiducial8[1] = 0;
    fiducial8[2] = 15;
    break;
    // case 2 is equal to fiducial marker configuration C (20mm distance)
  case 2:
    fiducial1[0] = 0;
    fiducial1[1] = 0;
    fiducial1[2] = 0;

    fiducial2[0] = 0;
    fiducial2[1] = 20;
    fiducial2[2] = 0;

    fiducial3[0] = 20;
    fiducial3[1] = 0;
    fiducial3[2] = 0;

    fiducial4[0] = 40;
    fiducial4[1] = 40;
    fiducial4[2] = 0;

    fiducial5[0] = 0;
    fiducial5[1] = 40;
    fiducial5[2] = 20;

    fiducial6[0] = 20;
    fiducial6[1] = 40;
    fiducial6[2] = 20;

    fiducial7[0] = 40;
    fiducial7[1] = 20;
    fiducial7[2] = 20;

    fiducial8[0] = 40;
    fiducial8[1] = 0;
    fiducial8[2] = 20;
    break;
  }

  m_MarkerModelCoordinateSystemPointSet->InsertPoint(0, fiducial1);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(1, fiducial2);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(2, fiducial3);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(3, fiducial4);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(4, fiducial5);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(5, fiducial6);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(6, fiducial7);
  m_MarkerModelCoordinateSystemPointSet->InsertPoint(7, fiducial8);

  /*mitk::DataNode::Pointer node = this->GetDataStorage()->GetNamedNode("Marker Model Coordinate System Point Set");
  if (node == nullptr)
  {
    node = mitk::DataNode::New();
    node->SetName("Marker Model Coordinate System Point Set");
    node->SetData(m_MarkerModelCoordinateSystemPointSet);
    this->GetDataStorage()->Add(node);
  }
  else
  {
    node->SetData(m_MarkerModelCoordinateSystemPointSet);
    this->GetDataStorage()->Modified();
  }*/
}

void QmitkUSNavigationStepCtUsRegistration::InitializePointsToTransformForGroundTruthProtocol()
{

  m_PointsToTransformGroundTruthProtocol.clear();

  mitk::Point3D point0mm;
  mitk::Point3D point20mm;
  mitk::Point3D point40mm;
  mitk::Point3D point60mm;
  mitk::Point3D point80mm;
  mitk::Point3D point100mm;

  point0mm[0] = 0.0;
  point0mm[1] = 0.0;
  point0mm[2] = 0.0;

  point20mm[0] = 0.0;
  point20mm[1] = 0.0;
  point20mm[2] = 0.0;

  point40mm[0] = 0.0;
  point40mm[1] = 0.0;
  point40mm[2] = 0.0;

  point60mm[0] = 0.0;
  point60mm[1] = 0.0;
  point60mm[2] = 0.0;

  point80mm[0] = 0.0;
  point80mm[1] = 0.0;
  point80mm[2] = 0.0;

  point100mm[0] = 0.0;
  point100mm[1] = 0.0;
  point100mm[2] = 0.0;

  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(0, point0mm));
  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(20, point20mm));
  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(40, point40mm));
  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(60, point60mm));
  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(80, point80mm));
  m_PointsToTransformGroundTruthProtocol.insert(std::pair<int, mitk::Point3D>(100, point100mm));
}

void QmitkUSNavigationStepCtUsRegistration::CreatePointsToTransformForGroundTruthProtocol()
{
  this->InitializePointsToTransformForGroundTruthProtocol();

  switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
  {
    // case 0 is equal to fiducial marker configuration A (10mm distance)
  case 0:
    MITK_WARN << "For this marker configuration (10mm) there does not exist a point to transform.";
    break;
    // case 1 is equal to fiducial marker configuration B (15mm distance)
  case 1:
    m_PointsToTransformGroundTruthProtocol.at(0)[0] = 130;  // = 30mm to end of clipping plate + 100 mm to middle axis of measurement plate
    m_PointsToTransformGroundTruthProtocol.at(0)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(0)[2] = -7;  // = 5mm distance to clipping plate + 2mm to base

    m_PointsToTransformGroundTruthProtocol.at(20)[0] = 130;
    m_PointsToTransformGroundTruthProtocol.at(20)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(20)[2] = -27;  // = 5mm distance to clipping plate + 2mm to base + 20mm depth

    m_PointsToTransformGroundTruthProtocol.at(40)[0] = 130;
    m_PointsToTransformGroundTruthProtocol.at(40)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(40)[2] = -47;  // = 5mm distance to clipping plate + 2mm to base + 40mm depth

    m_PointsToTransformGroundTruthProtocol.at(60)[0] = 130;
    m_PointsToTransformGroundTruthProtocol.at(60)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(60)[2] = -67;  // = 5mm distance to clipping plate + 2mm to base + 60mm depth

    m_PointsToTransformGroundTruthProtocol.at(80)[0] = 130;
    m_PointsToTransformGroundTruthProtocol.at(80)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(80)[2] = -87;  // = 5mm distance to clipping plate + 2mm to base + 80mm depth

    m_PointsToTransformGroundTruthProtocol.at(100)[0] = 130;
    m_PointsToTransformGroundTruthProtocol.at(100)[1] = 15;
    m_PointsToTransformGroundTruthProtocol.at(100)[2] = -107;  // = 5mm distance to clipping plate + 2mm to base + 100mm depth

    break;
    // case 2 is equal to fiducial marker configuration C (20mm distance)
  case 2:
    m_PointsToTransformGroundTruthProtocol.at(0)[0] = 135;  // = 20 + 15mm to end of clipping plate + 100 mm to middle axis of measurement plate
    m_PointsToTransformGroundTruthProtocol.at(0)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(0)[2] = -9;  // = 7mm distance to clipping plate + 2mm to base

    m_PointsToTransformGroundTruthProtocol.at(20)[0] = 135;
    m_PointsToTransformGroundTruthProtocol.at(20)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(20)[2] = -29;  // = 7mm distance to clipping plate + 2mm to base + 20mm depth

    m_PointsToTransformGroundTruthProtocol.at(40)[0] = 135;
    m_PointsToTransformGroundTruthProtocol.at(40)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(40)[2] = -49;  // = 7mm distance to clipping plate + 2mm to base + 40mm depth

    m_PointsToTransformGroundTruthProtocol.at(60)[0] = 135;
    m_PointsToTransformGroundTruthProtocol.at(60)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(60)[2] = -69;  // = 7mm distance to clipping plate + 2mm to base + 60mm depth

    m_PointsToTransformGroundTruthProtocol.at(80)[0] = 135;
    m_PointsToTransformGroundTruthProtocol.at(80)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(80)[2] = -89;  // = 7mm distance to clipping plate + 2mm to base + 80mm depth

    m_PointsToTransformGroundTruthProtocol.at(100)[0] = 135;
    m_PointsToTransformGroundTruthProtocol.at(100)[1] = 20;
    m_PointsToTransformGroundTruthProtocol.at(100)[2] = -109;  // = 7mm distance to clipping plate + 2mm to base + 100mm depth
    break;
  }
}

void QmitkUSNavigationStepCtUsRegistration::TransformPointsGroundTruthProtocol()
{
  if (m_GroundTruthProtocolTransformedPoints.find(0) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(0)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(0, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(0)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(0)));
  }

  if (m_GroundTruthProtocolTransformedPoints.find(20) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(20)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(20, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(20)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(20)));
  }

  if (m_GroundTruthProtocolTransformedPoints.find(40) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(40)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(40, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(40)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(40)));
  }

  if (m_GroundTruthProtocolTransformedPoints.find(60) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(60)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(60, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(60)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(60)));
  }

  if (m_GroundTruthProtocolTransformedPoints.find(80) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(80)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(80, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(80)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(80)));
  }

  if (m_GroundTruthProtocolTransformedPoints.find(100) == m_GroundTruthProtocolTransformedPoints.end())
  {
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSet->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(100)));
    m_GroundTruthProtocolTransformedPoints.insert(std::pair<int, mitk::PointSet::Pointer>(100, pointSet));
  }
  else
  {
    m_GroundTruthProtocolTransformedPoints.at(100)->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(m_PointsToTransformGroundTruthProtocol.at(100)));
  }

}

void QmitkUSNavigationStepCtUsRegistration::AddTransformedPointsToDataStorage()
{
  if (m_GroundTruthProtocolTransformedPoints.find(0) == m_GroundTruthProtocolTransformedPoints.end() ||
      m_GroundTruthProtocolTransformedPoints.find(20) == m_GroundTruthProtocolTransformedPoints.end() ||
      m_GroundTruthProtocolTransformedPoints.find(40) == m_GroundTruthProtocolTransformedPoints.end() ||
      m_GroundTruthProtocolTransformedPoints.find(60) == m_GroundTruthProtocolTransformedPoints.end() ||
      m_GroundTruthProtocolTransformedPoints.find(80) == m_GroundTruthProtocolTransformedPoints.end() ||
      m_GroundTruthProtocolTransformedPoints.find(100) == m_GroundTruthProtocolTransformedPoints.end())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot add transformed Points to DataStorage because they do not exist.\
      Stopping evaluation the protocol.");
    msgBox.exec();
    return;
  }

  std::string nameNode0mm = "GroundTruthProt-Depth0mm";
  std::string nameNode20mm = "GroundTruthProt-Depth20mm";
  std::string nameNode40mm = "GroundTruthProt-Depth40mm";
  std::string nameNode60mm = "GroundTruthProt-Depth60mm";
  std::string nameNode80mm = "GroundTruthProt-Depth80mm";
  std::string nameNode100mm = "GroundTruthProt-Depth100mm";

  //Add transformed points of depth 0mm to the data storage
  mitk::DataNode::Pointer node0mm = this->GetDataStorage()->GetNamedNode(nameNode0mm);
  if (node0mm.IsNull())
  {
    node0mm = mitk::DataNode::New();
    node0mm->SetName(nameNode0mm);
    node0mm->SetData(m_GroundTruthProtocolTransformedPoints.at(0));
    this->GetDataStorage()->Add(node0mm);
  }
  else
  {
    node0mm->SetData(m_GroundTruthProtocolTransformedPoints.at(0));
    this->GetDataStorage()->Modified();
  }

  if(ui->protocolEvaluationTypeComboBox->currentText().compare("PLANE") == 0 )
  {
    //Add transformed points of depth 20mm to the data storage
    mitk::DataNode::Pointer node20mm = this->GetDataStorage()->GetNamedNode(nameNode20mm);
    if (node20mm.IsNull())
    {
      node20mm = mitk::DataNode::New();
      node20mm->SetName(nameNode20mm);
      node20mm->SetData(m_GroundTruthProtocolTransformedPoints.at(20));
      this->GetDataStorage()->Add(node20mm);
    }
    else
    {
      node20mm->SetData(m_GroundTruthProtocolTransformedPoints.at(20));
      this->GetDataStorage()->Modified();
    }

    //Add transformed points of depth 40mm to the data storage
    mitk::DataNode::Pointer node40mm = this->GetDataStorage()->GetNamedNode(nameNode40mm);
    if (node40mm.IsNull())
    {
      node40mm = mitk::DataNode::New();
      node40mm->SetName(nameNode40mm);
      node40mm->SetData(m_GroundTruthProtocolTransformedPoints.at(40));
      this->GetDataStorage()->Add(node40mm);
    }
    else
    {
      node40mm->SetData(m_GroundTruthProtocolTransformedPoints.at(40));
      this->GetDataStorage()->Modified();
    }

    //Add transformed points of depth 60mm to the data storage
    mitk::DataNode::Pointer node60mm = this->GetDataStorage()->GetNamedNode(nameNode60mm);
    if (node60mm.IsNull())
    {
      node60mm = mitk::DataNode::New();
      node60mm->SetName(nameNode60mm);
      node60mm->SetData(m_GroundTruthProtocolTransformedPoints.at(60));
      this->GetDataStorage()->Add(node60mm);
    }
    else
    {
      node60mm->SetData(m_GroundTruthProtocolTransformedPoints.at(60));
      this->GetDataStorage()->Modified();
    }

    //Add transformed points of depth 80mm to the data storage
    mitk::DataNode::Pointer node80mm = this->GetDataStorage()->GetNamedNode(nameNode80mm);
    if (node80mm.IsNull())
    {
      node80mm = mitk::DataNode::New();
      node80mm->SetName(nameNode80mm);
      node80mm->SetData(m_GroundTruthProtocolTransformedPoints.at(80));
      this->GetDataStorage()->Add(node80mm);
    }
    else
    {
      node80mm->SetData(m_GroundTruthProtocolTransformedPoints.at(80));
      this->GetDataStorage()->Modified();
    }

    //Add transformed points of depth 100mm to the data storage
    mitk::DataNode::Pointer node100mm = this->GetDataStorage()->GetNamedNode(nameNode100mm);
    if (node100mm.IsNull())
    {
      node100mm = mitk::DataNode::New();
      node100mm->SetName(nameNode100mm);
      node100mm->SetData(m_GroundTruthProtocolTransformedPoints.at(100));
      this->GetDataStorage()->Add(node100mm);
    }
    else
    {
      node100mm->SetData(m_GroundTruthProtocolTransformedPoints.at(100));
      this->GetDataStorage()->Modified();
    }
  }
  //Do a global reinit
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
}

double QmitkUSNavigationStepCtUsRegistration::CalculateMeanFRE()
{
  double meanFRE = 0.0;
  for (unsigned int counter = 0; counter < m_GroundTruthProtocolFRE.size(); ++counter)
  {
    meanFRE += m_GroundTruthProtocolFRE[counter];
  }

  return meanFRE / m_GroundTruthProtocolFRE.size();
}

double QmitkUSNavigationStepCtUsRegistration::CalculateStandardDeviationOfFRE(double meanFRE)
{
  double variance = 0.0;

  for (unsigned int counter = 0; counter < m_GroundTruthProtocolFRE.size(); ++counter)
  {
    variance += ((meanFRE - m_GroundTruthProtocolFRE[counter]) * (meanFRE - m_GroundTruthProtocolFRE[counter]));
  }
  variance /= m_GroundTruthProtocolFRE.size(); // calculate the empirical variance (n) and not the sampling variance (n-1)

  return sqrt(variance);
}

void QmitkUSNavigationStepCtUsRegistration::CalculateGroundTruthProtocolTRE()
{
  if (m_GroundTruthProtocolTransformedPoints.find(0) == m_GroundTruthProtocolTransformedPoints.end() ||
    m_GroundTruthProtocolTransformedPoints.find(20) == m_GroundTruthProtocolTransformedPoints.end() ||
    m_GroundTruthProtocolTransformedPoints.find(40) == m_GroundTruthProtocolTransformedPoints.end() ||
    m_GroundTruthProtocolTransformedPoints.find(60) == m_GroundTruthProtocolTransformedPoints.end() ||
    m_GroundTruthProtocolTransformedPoints.find(80) == m_GroundTruthProtocolTransformedPoints.end() ||
    m_GroundTruthProtocolTransformedPoints.find(100) == m_GroundTruthProtocolTransformedPoints.end())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot calculate TRE of Ground-Truth-Protocol because points were not transformed.");
    msgBox.exec();
    return;
  }

  // clear the std::map containing possibly data of earlier TRE calculations
  m_GroundTruthProtocolTRE.clear();
  // loop through all existing point sets containing the transformed points
  for (int counter = 0;
       m_GroundTruthProtocolTransformedPoints.find(counter) != m_GroundTruthProtocolTransformedPoints.end();
       counter += 20)
  {
    //calculate the middle point of  the point set
    mitk::PointSet::Pointer pointSet = m_GroundTruthProtocolTransformedPoints.at(counter);
    mitk::Point3D middlePoint;
    middlePoint[0] = 0.0;
    middlePoint[1] = 0.0;
    middlePoint[2] = 0.0;

    for (int position = 0; position < pointSet->GetSize(); ++position)
    {
      middlePoint[0] += pointSet->GetPoint(position)[0];
      middlePoint[1] += pointSet->GetPoint(position)[1];
      middlePoint[2] += pointSet->GetPoint(position)[2];
    }
    middlePoint[0] /= pointSet->GetSize();
    middlePoint[1] /= pointSet->GetSize();
    middlePoint[2] /= pointSet->GetSize();
    MITK_INFO << "Calculated MiddlePoint: " << middlePoint;

    //sum up the euclidean distances between the middle point and each transformed point
    double meanDistance = 0.0;
    for (int position = 0; position < pointSet->GetSize(); ++position)
    {
      meanDistance += middlePoint.SquaredEuclideanDistanceTo(pointSet->GetPoint(position));
      MITK_INFO << "SquaredEuclideanDistance: " << middlePoint.SquaredEuclideanDistanceTo(pointSet->GetPoint(position));
    }

    meanDistance /= pointSet->GetSize(); // this can be interpreted as empirical variance
    // the root of the empirical variance can be interpreted as the protocols registration TRE
    m_GroundTruthProtocolTRE.insert(std::pair<int, double>(counter, sqrt(meanDistance)));
    MITK_INFO << "Ground-Truth-Protocol TRE: " << sqrt(meanDistance);
  }

}

void QmitkUSNavigationStepCtUsRegistration::EliminateTooSmallLabeledObjects(
  ImageType::Pointer binaryImage)
{
  BinaryImageToShapeLabelMapFilterType::OutputImageType::Pointer labelMap =
    m_BinaryImageToShapeLabelMapFilter->GetOutput();
  double voxelVolume = this->GetVoxelVolume();
  double fiducialVolume;
  unsigned int numberOfPixels;

  if (ui->fiducialDiameter3mmRadioButton->isChecked())
  {
    fiducialVolume = this->GetFiducialVolume(1.5);
    numberOfPixels = ceil(fiducialVolume / voxelVolume);
  }
  else
  {
    fiducialVolume = this->GetFiducialVolume(2.5);
    numberOfPixels = ceil(fiducialVolume / voxelVolume);
  }

  MITK_INFO << "Voxel Volume = " << voxelVolume << "; Fiducial Volume = " << fiducialVolume;
  MITK_INFO << "Number of pixels = " << numberOfPixels;

  labelMap = m_BinaryImageToShapeLabelMapFilter->GetOutput();
  // The output of this filter is an itk::LabelMap, which contains itk::LabelObject's
  MITK_INFO << "There are " << labelMap->GetNumberOfLabelObjects() << " objects.";

  // Loop over each region
  for (int i = labelMap->GetNumberOfLabelObjects() - 1; i >= 0; --i)
  {
    // Get the ith region
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelMap->GetNthLabelObject(i);
    MITK_INFO << "Object " << i << " contains " << labelObject->Size() << " pixel";

    //TODO: Threshold-Wert evtl. experimentell besser abstimmen,
    //      um zu verhindern, dass durch Threshold wahre Fiducial-Kandidaten elimiert werden.
    if (labelObject->Size() < numberOfPixels * 0.8)
    {
      for (unsigned int pixelId = 0; pixelId < labelObject->Size(); pixelId++)
      {
        binaryImage->SetPixel(labelObject->GetIndex(pixelId), 0);
      }
      labelMap->RemoveLabelObject(labelObject);
    }
  }
}

bool QmitkUSNavigationStepCtUsRegistration::EliminateFiducialCandidatesByEuclideanDistances()
{
  if (m_CentroidsOfFiducialCandidates.size() < NUMBER_FIDUCIALS_NEEDED)
  {
    return false;
  }

  for (unsigned int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    int amountOfAcceptedFiducials = 0;
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    //Loop through all fiducial candidates and calculate the distance between the chosen fiducial
    // candidate and the other candidates. For each candidate with a right distance between
    // Configuration A: 7.93mm and 31.0mm (10 mm distance between fiducial centers) or
    // Configuration B: 11.895mm and 45.0mm (15 mm distance between fiducial centers) or
    // Configuration C: 15.86mm and 59.0mm (20 mm distance between fiducial centers)
    //
    // increase the amountOfAcceptedFiducials.
    for (unsigned int position = 0; position < m_CentroidsOfFiducialCandidates.size(); ++position)
    {
      if (position == counter)
      {
        continue;
      }
      mitk::Point3D otherCentroid(m_CentroidsOfFiducialCandidates.at(position));
      double distance = fiducialCentroid.EuclideanDistanceTo(otherCentroid);

      switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
      {
        // case 0 is equal to fiducial marker configuration A (10mm distance)
        case 0:
          if (distance > 7.93 && distance < 31.0)
          {
            ++amountOfAcceptedFiducials;
          }
          break;
        // case 1 is equal to fiducial marker configuration B (15mm distance)
        case 1:
          if (distance > 11.895 && distance < 45.0)
          {
            ++amountOfAcceptedFiducials;
          }
          break;
        // case 2 is equal to fiducial marker configuration C (20mm distance)
        case 2:
          if (distance > 15.86 && distance < 59.0)
          {
            ++amountOfAcceptedFiducials;
          }
          break;
      }
    }
    //The amountOfAcceptedFiducials must be at least 7. Otherwise delete the fiducial candidate
    // from the list of candidates.
    if (amountOfAcceptedFiducials < NUMBER_FIDUCIALS_NEEDED - 1)
    {
      MITK_INFO << "Deleting fiducial candidate at position: " <<
        m_CentroidsOfFiducialCandidates.at(counter);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + counter);
      if (m_CentroidsOfFiducialCandidates.size() < NUMBER_FIDUCIALS_NEEDED )
      {
        return false;
      }
      counter = -1;
    }
  }

  //Classify the rested fiducial candidates by its characteristic Euclidean distances
  // between the canidates and remove all candidates with a false distance configuration:
  this->ClassifyFiducialCandidates();
  return true;
}

void QmitkUSNavigationStepCtUsRegistration::ClassifyFiducialCandidates()
{
  MITK_INFO << "ClassifyFiducialCandidates()";
  std::vector<int> fiducialCandidatesToBeRemoved;
  std::vector<std::vector<double>> distanceVectorsFiducials;
  this->CalculateDistancesBetweenFiducials(distanceVectorsFiducials);

  for (unsigned int counter = 0; counter < distanceVectorsFiducials.size(); ++counter)
  {
    int distanceA = 0;      // => 10,00mm distance
    int distanceB = 0;      // => 14,14mm distance
    int distanceC = 0;      // => 17,32mm distance
    int distanceD = 0;      // => 22,36mm distance
    int distanceE = 0;      // => 24,49mm distance
    int distanceF = 0;      // => 28,28mm distance

    std::vector<double> &distances = distanceVectorsFiducials.at(counter);
    for (unsigned int number = 0; number < distances.size(); ++number)
    {
      double &distance = distances.at(number);
      switch (ui->fiducialMarkerConfigurationComboBox->currentIndex())
      {
        // case 0 is equal to fiducial marker configuration A (10mm distance)
        case 0:
          if (distance > 7.93 && distance <= 12.07)
          {
            ++distanceA;
          }
          else if (distance > 12.07 && distance <= 15.73)
          {
            ++distanceB;
          }
          else if (distance > 15.73 && distance <= 19.84)
          {
            ++distanceC;
          }
          else if (distance > 19.84 && distance <= 23.425)
          {
            ++distanceD;
          }
          else if (distance > 23.425 && distance <= 26.385)
          {
            ++distanceE;
          }
          else if (distance > 26.385 && distance <= 31.00)
          {
            ++distanceF;
          }
        break;

        // case 1 is equal to fiducial marker configuration B (15mm distance)
        case 1:
          if (distance > 11.895 && distance <= 18.105)
          {
            ++distanceA;
          }
          else if (distance > 18.105 && distance <= 23.595)
          {
            ++distanceB;
          }
          else if (distance > 23.595 && distance <= 29.76)
          {
            ++distanceC;
          }
          else if (distance > 29.76 && distance <= 35.1375)
          {
            ++distanceD;
            if (distance > 33.54)
            {
              ++distanceE;
            }
          }
          else if (distance > 35.1375 && distance <= 39.5775)
          {
            ++distanceE;
            if (distance < 36.735)
            {
              ++distanceD;
            }
          }
          else if (distance > 39.5775 && distance <= 45.00)
          {
            ++distanceF;
          }
        break;

        // case 2 is equal to fiducial marker configuration C (20mm distance)
        case 2:
          if (distance > 15.86 && distance <= 24.14)
          {
            ++distanceA;
          }
          else if (distance > 24.14 && distance <= 31.46)
          {
            ++distanceB;
          }
          else if (distance > 31.46 && distance <= 39.68)
          {
            ++distanceC;
          }
          else if (distance > 39.68 && distance <= 46.85)
          {
            ++distanceD;
          }
          else if (distance > 46.85 && distance <= 52.77)
          {
            ++distanceE;
          }
          else if (distance > 52.77 && distance <= 59.00)
          {
            ++distanceF;
          }
        break;
      }
    }// End for-loop distances-vector

    //Now, having looped through all distances of one fiducial candidate, check
    // if the combination of different distances is known. The >= is due to the
    // possible occurrence of other fiducial candidates that have an distance equal to
    // one of the distances A - E. However, false fiducial candidates outside
    // the fiducial marker does not have the right distance configuration:
    if (((distanceA >= 2 && distanceD >= 2 && distanceE >= 2 && distanceF >= 1) ||
      (distanceA >= 1 && distanceB >= 2 && distanceC >= 1 && distanceD >= 2 && distanceE >= 1) ||
      (distanceB >= 2 && distanceD >= 4 && distanceF >= 1) ||
      (distanceA >= 1 && distanceB >= 1 && distanceD >= 3 && distanceE >= 1 && distanceF >= 1)) == false)
    {
      MITK_INFO << "Detected fiducial candidate with unknown distance configuration.";
      fiducialCandidatesToBeRemoved.push_back(counter);
    }
  }
  for (int count = fiducialCandidatesToBeRemoved.size() - 1; count >= 0; --count)
  {
    MITK_INFO << "Removing fiducial candidate " << fiducialCandidatesToBeRemoved.at(count);
    m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin()
                                          + fiducialCandidatesToBeRemoved.at(count));
  }
}

void QmitkUSNavigationStepCtUsRegistration::GetCentroidsOfLabeledObjects()
{
  MITK_INFO << "GetCentroidsOfLabeledObjects()";
  BinaryImageToShapeLabelMapFilterType::OutputImageType::Pointer labelMap =
    m_BinaryImageToShapeLabelMapFilter->GetOutput();
  for (int i = labelMap->GetNumberOfLabelObjects() - 1; i >= 0; --i)
  {
    // Get the ith region
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelMap->GetNthLabelObject(i);
    MITK_INFO << "Object " << i << " contains " << labelObject->Size() << " pixel";

    mitk::Vector3D centroid;
    centroid[0] = labelObject->GetCentroid()[0];
    centroid[1] = labelObject->GetCentroid()[1];
    centroid[2] = labelObject->GetCentroid()[2];
    m_CentroidsOfFiducialCandidates.push_back(centroid);
  }
  //evtl. for later: itk::LabelMapOverlayImageFilter
}

void QmitkUSNavigationStepCtUsRegistration::NumerateFiducialMarks()
{
  MITK_INFO << "NumerateFiducialMarks()";
  bool successFiducialNo1;
  bool successFiducialNo4;
  bool successFiducialNo2And3;
  bool successFiducialNo5;
  bool successFiducialNo8;
  bool successFiducialNo6;
  bool successFiducialNo7;

  std::vector<std::vector<double>> distanceVectorsFiducials;
  this->CalculateDistancesBetweenFiducials(distanceVectorsFiducials);
  successFiducialNo1 = this->FindFiducialNo1(distanceVectorsFiducials);
  successFiducialNo4 = this->FindFiducialNo4(distanceVectorsFiducials);
  successFiducialNo2And3 = this->FindFiducialNo2And3();
  successFiducialNo5 = this->FindFiducialNo5();
  successFiducialNo8 = this->FindFiducialNo8();
  successFiducialNo6 = this->FindFiducialNo6();
  successFiducialNo7 = this->FindFiducialNo7();

  if (!successFiducialNo1 || !successFiducialNo4 || !successFiducialNo2And3 ||
    !successFiducialNo5 || !successFiducialNo8 || !successFiducialNo6 || !successFiducialNo7)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot numerate/localize all fiducials successfully.");
    msgBox.exec();
    return;
  }

  if (m_MarkerFloatingImageCoordinateSystemPointSet.IsNull())
  {
    m_MarkerFloatingImageCoordinateSystemPointSet = mitk::PointSet::New();
  }
  else if (m_MarkerFloatingImageCoordinateSystemPointSet->GetSize() != 0)
  {
    m_MarkerFloatingImageCoordinateSystemPointSet->Clear();
  }

  for (unsigned int counter = 1; counter <= m_FiducialMarkerCentroids.size(); ++counter)
  {
    m_MarkerFloatingImageCoordinateSystemPointSet->InsertPoint(counter - 1, m_FiducialMarkerCentroids.at(counter));
  }
  if( !m_PerformingGroundTruthProtocolEvaluation )
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(m_MarkerFloatingImageCoordinateSystemPointSet);
    node->SetName("MarkerFloatingImageCSPointSet");
    //node->SetFloatProperty("pointsize", 5.0);
    this->GetDataStorage()->Add(node);
  }
}

void QmitkUSNavigationStepCtUsRegistration::CalculateDistancesBetweenFiducials(std::vector<std::vector<double>>& distanceVectorsFiducials)
{
  std::vector<double> distancesBetweenFiducials;

  for (unsigned int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    distancesBetweenFiducials.clear();
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    for (unsigned int n = 0; n < m_CentroidsOfFiducialCandidates.size(); ++n)
    {
      mitk::Point3D otherCentroid(m_CentroidsOfFiducialCandidates.at(n));
      distancesBetweenFiducials.push_back(fiducialCentroid.EuclideanDistanceTo(otherCentroid));
    }
    //Sort the distances from low to big numbers
    std::sort(distancesBetweenFiducials.begin(), distancesBetweenFiducials.end());
    //First entry of the distance vector must be 0, so erase it
    if (distancesBetweenFiducials.at(0) == 0.0)
    {
      distancesBetweenFiducials.erase(distancesBetweenFiducials.begin());
    }
    //Add the distance vector to the collecting distances vector
    distanceVectorsFiducials.push_back(distancesBetweenFiducials);
  }

  for (unsigned int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    MITK_INFO << "Vector " << i << ":";
    for (unsigned int k = 0; k < distanceVectorsFiducials.at(i).size(); ++k)
    {
      MITK_INFO << distanceVectorsFiducials.at(i).at(k);
    }
  }
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo1(std::vector<std::vector<double>>& distanceVectorsFiducials)
{
  for (unsigned int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    std::vector<double> &distances = distanceVectorsFiducials.at(i);
    if (distances.size() < NUMBER_FIDUCIALS_NEEDED - 1 )
    {
      MITK_WARN << "Cannot find fiducial 1, there aren't found enough fiducial candidates.";
      return false;
    }
    double characteristicDistanceAWithUpperMargin = this->GetCharacteristicDistanceAWithUpperMargin();

    if (distances.at(0) <= characteristicDistanceAWithUpperMargin &&
        distances.at(1) <= characteristicDistanceAWithUpperMargin)
    {
      MITK_INFO << "Found Fiducial 1 (PointSet number " << i << ")";
      m_FiducialMarkerCentroids.insert( std::pair<int,mitk::Vector3D>(1, m_CentroidsOfFiducialCandidates.at(i)));
      distanceVectorsFiducials.erase(distanceVectorsFiducials.begin() + i);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + i);
      return true;
    }
  }
  return false;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo2And3()
{
  if (m_FiducialMarkerCentroids.find(1) == m_FiducialMarkerCentroids.end() )
  {
    MITK_WARN << "Cannot find fiducial No 2 and 3. Before must be found fiducial No 1.";
    return false;
  }

  mitk::Point3D fiducialNo1(m_FiducialMarkerCentroids.at(1));
  mitk::Vector3D fiducialVectorA;
  mitk::Vector3D fiducialVectorB;
  mitk::Point3D fiducialPointA;
  mitk::Point3D fiducialPointB;
  bool foundFiducialA = false;
  bool foundFiducialB = false;
  mitk::Vector3D vectorFiducial1ToFiducialA;
  mitk::Vector3D vectorFiducial1ToFiducialB;

  for (unsigned int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    double distance = fiducialNo1.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= this->GetCharacteristicDistanceAWithUpperMargin())
    {
      fiducialVectorA = m_CentroidsOfFiducialCandidates.at(i);
      fiducialPointA = fiducialCentroid;
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + i);
      foundFiducialA = true;
      break;
    }
  }

  for (unsigned int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    double distance = fiducialNo1.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= this->GetCharacteristicDistanceAWithUpperMargin())
    {
      fiducialVectorB = m_CentroidsOfFiducialCandidates.at(i);
      fiducialPointB = fiducialCentroid;
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + i);
      foundFiducialB = true;
      break;
    }
  }

  if (!foundFiducialA || !foundFiducialB)
  {
    MITK_WARN << "Cannot identify fiducial candidates 2 and 3";
    return false;
  }
  else if (m_CentroidsOfFiducialCandidates.size() == 0)
  {
    MITK_WARN << "Too less fiducials detected. Cannot identify fiducial candidates 2 and 3";
    return false;
  }

  vectorFiducial1ToFiducialA = fiducialVectorA - m_FiducialMarkerCentroids.at(1);
  vectorFiducial1ToFiducialB = fiducialVectorB - m_FiducialMarkerCentroids.at(1);

  vnl_vector<double> crossProductVnl = vnl_cross_3d(vectorFiducial1ToFiducialA.GetVnlVector(), vectorFiducial1ToFiducialB.GetVnlVector());
  mitk::Vector3D crossProduct;
  crossProduct.SetVnlVector(crossProductVnl);

  mitk::Vector3D vectorFiducial1ToRandomLeftFiducial = m_CentroidsOfFiducialCandidates.at(0) - m_FiducialMarkerCentroids.at(1);

  double scalarProduct = (crossProduct * vectorFiducial1ToRandomLeftFiducial) /
                         (crossProduct.GetNorm() * vectorFiducial1ToRandomLeftFiducial.GetNorm());

  double alpha = acos(scalarProduct) * 57.29578; //Transform into degree
  MITK_INFO << "Scalar Product = " << alpha;

  if (alpha <= 90)
  {
    m_FiducialMarkerCentroids[3] = fiducialVectorA;
    m_FiducialMarkerCentroids[2] = fiducialVectorB;
  }
  else
  {
    m_FiducialMarkerCentroids[2] = fiducialVectorA;
    m_FiducialMarkerCentroids[3] = fiducialVectorB;
  }

  MITK_INFO << "Found Fiducial 2, PointSet: " << m_FiducialMarkerCentroids.at(2);
  MITK_INFO << "Found Fiducial 3, PointSet: " << m_FiducialMarkerCentroids.at(3);

  return true;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo4(std::vector<std::vector<double>>& distanceVectorsFiducials)
{
  double characteristicDistanceBWithLowerMargin = this->GetCharacteristicDistanceBWithLowerMargin();
  double characteristicDistanceBWithUpperMargin = this->GetCharacteristicDistanceBWithUpperMargin();

  for (unsigned int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    std::vector<double> &distances = distanceVectorsFiducials.at(i);
    if (distances.size() < NUMBER_FIDUCIALS_NEEDED - 1)
    {
      MITK_WARN << "Cannot find fiducial 4, there aren't found enough fiducial candidates.";
      return false;
    }

    if (distances.at(0) > characteristicDistanceBWithLowerMargin &&
        distances.at(0) <= characteristicDistanceBWithUpperMargin &&
        distances.at(1) > characteristicDistanceBWithLowerMargin &&
        distances.at(1) <= characteristicDistanceBWithUpperMargin)
    {
      MITK_INFO << "Found Fiducial 4 (PointSet number " << i << ")";
      m_FiducialMarkerCentroids.insert(std::pair<int, mitk::Vector3D>(4, m_CentroidsOfFiducialCandidates.at(i)));
      distanceVectorsFiducials.erase(distanceVectorsFiducials.begin() + i);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + i);
      return true;
    }
  }
  return false;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo5()
{
  if (m_FiducialMarkerCentroids.find(2) == m_FiducialMarkerCentroids.end())
  {
    MITK_WARN << "To find fiducial No 5, fiducial No 2 has to be found before.";
    return false;
  }

  double characteristicDistanceBWithUpperMargin = this->GetCharacteristicDistanceBWithUpperMargin();

  mitk::Point3D fiducialNo2(m_FiducialMarkerCentroids.at(2));

  for (unsigned int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo2.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= characteristicDistanceBWithUpperMargin)
    {
      m_FiducialMarkerCentroids[5] = m_CentroidsOfFiducialCandidates.at(counter);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + counter);
      MITK_INFO << "Found Fiducial No 5, PointSet: " << m_FiducialMarkerCentroids[5];
      return true;
    }
  }

  MITK_WARN << "Cannot find fiducial No 5.";
  return false;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo6()
{
  if (m_FiducialMarkerCentroids.find(5) == m_FiducialMarkerCentroids.end())
  {
    MITK_WARN << "To find fiducial No 6, fiducial No 5 has to be found before.";
    return false;
  }

  double characteristicDistanceAWithUpperMargin = this->GetCharacteristicDistanceAWithUpperMargin();

  mitk::Point3D fiducialNo5(m_FiducialMarkerCentroids.at(5));

  for (unsigned int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo5.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= characteristicDistanceAWithUpperMargin)
    {
      m_FiducialMarkerCentroids[6] = m_CentroidsOfFiducialCandidates.at(counter);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + counter);
      MITK_INFO << "Found Fiducial No 6, PointSet: " << m_FiducialMarkerCentroids[6];
      return true;
    }
  }

  MITK_WARN << "Cannot find fiducial No 6.";
  return false;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo7()
{
  if (m_FiducialMarkerCentroids.find(8) == m_FiducialMarkerCentroids.end())
  {
    MITK_WARN << "To find fiducial No 7, fiducial No 8 has to be found before.";
    return false;
  }

  double characteristicDistanceAWithUpperMargin = this->GetCharacteristicDistanceAWithUpperMargin();

  mitk::Point3D fiducialNo8(m_FiducialMarkerCentroids.at(8));

  for (unsigned int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo8.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= characteristicDistanceAWithUpperMargin)
    {
      m_FiducialMarkerCentroids[7] = m_CentroidsOfFiducialCandidates.at(counter);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + counter);
      MITK_INFO << "Found Fiducial No 7, PointSet: " << m_FiducialMarkerCentroids[7];
      return true;
    }
  }

  MITK_WARN << "Cannot find fiducial No 7.";
  return false;
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo8()
{
  if (m_FiducialMarkerCentroids.find(3) == m_FiducialMarkerCentroids.end())
  {
    MITK_WARN << "To find fiducial No 8, fiducial No 3 has to be found before.";
    return false;
  }

  double characteristicDistanceBWithUpperMargin = this->GetCharacteristicDistanceBWithUpperMargin();

  mitk::Point3D fiducialNo3(m_FiducialMarkerCentroids.at(3));

  for (unsigned int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo3.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= characteristicDistanceBWithUpperMargin)
    {
      m_FiducialMarkerCentroids[8] = m_CentroidsOfFiducialCandidates.at(counter);
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + counter);
      MITK_INFO << "Found Fiducial No 8, PointSet: " << m_FiducialMarkerCentroids[8];
      return true;
    }
  }

  MITK_WARN << "Cannot find fiducial No 8.";
  return false;
}

void QmitkUSNavigationStepCtUsRegistration::DefineDataStorageImageFilter()
{
  m_IsAPointSetPredicate = mitk::TNodePredicateDataType<mitk::PointSet>::New();
  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

  auto isSegmentation = mitk::NodePredicateDataType::New("Segment");
  m_IsASurfacePredicate = mitk::NodePredicateDataType::New("Surface");

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
  ui->ctImagesToChooseComboBox->SetPredicate(m_IsAPatientImagePredicate);
  ui->segmentationComboBox->SetPredicate(m_IsASegmentationImagePredicate);
  ui->selectedSurfaceComboBox->SetPredicate(m_IsASurfacePredicate);
  ui->pointSetComboBox->SetPredicate(m_IsAPointSetPredicate);

  // create signal/slot connections
  connect(ui->floatingImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(ui->doRegistrationMarkerToImagePushButton, SIGNAL(clicked()),
    this, SLOT(OnRegisterMarkerToFloatingImageCS()));
  connect(ui->localizeFiducialMarkerPushButton, SIGNAL(clicked()),
    this, SLOT(OnLocalizeFiducials()));
  connect(ui->visualizeCTtoUSregistrationPushButton, SIGNAL(clicked()),
    this, SLOT(OnVisualizeCTtoUSregistration()));
  connect(ui->freezeUnfreezePushButton, SIGNAL(clicked()),
    this, SLOT(OnFreeze()));
  connect(ui->addCtImagePushButton, SIGNAL(clicked()),
    this, SLOT(OnAddCtImageClicked()));
  connect(ui->removeCtImagePushButton, SIGNAL(clicked()),
    this, SLOT(OnRemoveCtImageClicked()));
  connect(ui->evaluateProtocolPushButton, SIGNAL(clicked()),
    this, SLOT(OnEvaluateGroundTruthFiducialLocalizationProtocol()));
  connect(ui->actualizeSegmentationSurfacePSetDataPushButton, SIGNAL(clicked()),
    this, SLOT(OnActualizeSegmentationSurfacePointSetData()));
  connect(ui->calculateTREPushButton, SIGNAL(clicked()),
    this, SLOT(OnGetCursorPosition()));
  connect(ui->calculateCenterPushButton, SIGNAL(clicked()),
    this, SLOT(OnCalculateCenter()));
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

void QmitkUSNavigationStepCtUsRegistration::OnRegisterMarkerToFloatingImageCS()
{
  this->CreateMarkerModelCoordinateSystemPointSet();

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
  if (m_PerformingGroundTruthProtocolEvaluation)
  {
    m_GroundTruthProtocolFRE.push_back(FRE);
  }
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

  if( !m_PerformingGroundTruthProtocolEvaluation )
  {
    mitk::PointSet* pointSet_orig = m_MarkerModelCoordinateSystemPointSet;
    mitk::PointSet::Pointer pointSet_moved = mitk::PointSet::New();

    for (int i = 0; i < pointSet_orig->GetSize(); i++)
    {
      pointSet_moved->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(pointSet_orig->GetPoint(i)));
    }

    pointSet_orig->Clear();
    for (int i = 0; i < pointSet_moved->GetSize(); i++)
      pointSet_orig->InsertPoint(pointSet_moved->GetPoint(i));

    //Do a global reinit
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
  }

}

void QmitkUSNavigationStepCtUsRegistration::OnLocalizeFiducials()
{
  m_FiducialMarkerCentroids.clear();
  m_CentroidsOfFiducialCandidates.clear();
  if (m_MarkerFloatingImageCoordinateSystemPointSet.IsNotNull())
  {
    m_MarkerFloatingImageCoordinateSystemPointSet->Clear();
  }

  if (!this->FilterFloatingImage())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot perform filtering of the image. The floating image = nullptr.");
    msgBox.exec();
    return;
  }
  mitk::AffineTransform3D::Pointer transform = m_FloatingImage->GetGeometry()->GetIndexToWorldTransform();
  MITK_WARN << "IndexToWorldTransform_CTimage = " << transform;

  this->GetCentroidsOfLabeledObjects();

  if (!this->EliminateFiducialCandidatesByEuclideanDistances() ||
      m_CentroidsOfFiducialCandidates.size() != NUMBER_FIDUCIALS_NEEDED)
  {
    QMessageBox msgBox;
    QString text = QString("Have found %1 instead of 8 fiducial candidates.\
      Cannot perform fiducial localization procedure.").arg(m_CentroidsOfFiducialCandidates.size());
    msgBox.setText(text);
    msgBox.exec();
    return;
  }

  //Before calling NumerateFiducialMarks it must be sure,
  // that there rested only 8 fiducial candidates.
  this->NumerateFiducialMarks();
}

void QmitkUSNavigationStepCtUsRegistration::OnVisualizeCTtoUSregistration()
{
  emit this->ActualizeCtToUsRegistrationWidget();

  mitk::DataNode* segmentationNode = ui->segmentationComboBox->GetSelectedNode();
  if (segmentationNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot visualize CT-to-US registration. There is no segmentation selected.");
    msgBox.exec();
    return;
  }
  mitk::AffineTransform3D::Pointer transform = segmentationNode->GetData()->GetGeometry()->GetIndexToWorldTransform();
  MITK_WARN << "IndexToWorldTransform_segmentation = " << transform;

  mitk::DataNode* surfaceNode = ui->selectedSurfaceComboBox->GetSelectedNode();
  if (surfaceNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot visualize CT-to-US registration. There is no surface selected.");
    msgBox.exec();
    return;
  }

  mitk::DataNode* pointSetNode = ui->pointSetComboBox->GetSelectedNode();
  if (pointSetNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot visualize CT-to-US registration. There is no pointSet selected.");
    msgBox.exec();
    return;
  }

  if (this->GetCombinedModality(false).IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("CombinedModality not yet set.\nPlease try again and click on the button.");
    msgBox.exec();
    return;
  }

  if (m_FloatingImageToUltrasoundRegistrationFilter.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot visualize CT-to-US registration.\
                    The FloatingImageToUltrasoundRegistrationFilter is not initialized.");
    msgBox.exec();
    return;
  }
  //Set the transformation from  marker-CS to the sensor-CS accordingly to the chosen user-option
  m_FloatingImageToUltrasoundRegistrationFilter
    ->InitializeTransformationMarkerCSToSensorCS(ui->useNdiTrackerCheckBox->isChecked());
  m_FloatingImageToUltrasoundRegistrationFilter->SetPointSet(pointSetNode);
  m_FloatingImageToUltrasoundRegistrationFilter->SetSegmentation(segmentationNode, m_FloatingImage);
  m_FloatingImageToUltrasoundRegistrationFilter->SetSurface(surfaceNode);
  m_FloatingImageToUltrasoundRegistrationFilter
    ->SetTransformMarkerCSToFloatingImageCS(m_TransformMarkerCSToFloatingImageCS);
  m_FloatingImageToUltrasoundRegistrationFilter
    ->SetTransformUSimageCSToTrackingCS(this->GetCombinedModality()->GetCalibration());
  m_FloatingImageToUltrasoundRegistrationFilter
    ->ConnectTo(this->GetCombinedModality()->GetNavigationDataSource());
}

void QmitkUSNavigationStepCtUsRegistration::OnFreeze()
{
  if (this->GetCombinedModality(false).IsNull())
  {
    return;
  }

  if (!m_FreezeCombinedModality)
  {
    m_FreezeCombinedModality = true;
    ui->freezeUnfreezePushButton->setText("Unfreeze");
    this->GetCombinedModality()->SetIsFreezed(true);
  }
  else
  {
    m_FreezeCombinedModality = false;
    ui->freezeUnfreezePushButton->setText("Freeze");
    this->GetCombinedModality()->SetIsFreezed(false);
  }
}

void QmitkUSNavigationStepCtUsRegistration::OnActualizeSegmentationSurfacePointSetData()
{
  mitk::DataNode* segmentationNode = ui->segmentationComboBox->GetSelectedNode();
  if (segmentationNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot actualize segmentation + surface + pointset data. There is no segmentation selected.");
    msgBox.exec();
    return;
  }

  mitk::DataNode* surfaceNode = ui->selectedSurfaceComboBox->GetSelectedNode();
  if (surfaceNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot actualize segmentation + surface + pointset data. There is no surface selected.");
    msgBox.exec();
    return;
  }

  mitk::DataNode* pointSetNode = ui->pointSetComboBox->GetSelectedNode();
  if (pointSetNode == nullptr)
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot actualize segmentation + surface + pointset data. There is no pointSet selected.");
    msgBox.exec();
    return;
  }

  m_FloatingImageToUltrasoundRegistrationFilter->SetPointSet(pointSetNode);
  m_FloatingImageToUltrasoundRegistrationFilter->SetSegmentation(segmentationNode, m_FloatingImage);
  m_FloatingImageToUltrasoundRegistrationFilter->SetSurface(surfaceNode);
}

void QmitkUSNavigationStepCtUsRegistration::OnGetCursorPosition()
{
  emit GetCursorPosition();
}

void QmitkUSNavigationStepCtUsRegistration::OnCalculateTRE(mitk::Point3D centroidOfTargetInUSImage)
{
  mitk::DataNode::Pointer pointSetNode = ui->pointSetComboBox->GetSelectedNode();
  if (pointSetNode.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot calculate TRE. The pointSetComboBox node returned a nullptr.");
    msgBox.exec();
    return;
  }

  mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(pointSetNode->GetData());
  if (pointSet.IsNull())
  {
    ui->distanceTREValue->setText(QString("Unknown"));
    return;
  }
  double distance = pointSet->GetPoint(0).EuclideanDistanceTo(centroidOfTargetInUSImage);
  ui->distanceTREValue->setText(QString("%1").arg(distance));
}

void QmitkUSNavigationStepCtUsRegistration::OnCalculateCenter()
{
  mitk::DataNode::Pointer node = ui->segmentationComboBox->GetSelectedNode();
  if (node.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("Cannot calculate the centroid of the segmentation."\
                    "The segmentationComboBox node returned a nullptr.");
    msgBox.exec();
    return;
  }

  mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
  if (image.IsNull())
  {
    MITK_WARN << "Cannot CalculateCenter - the segmentation cannot be converted to mitk::Image";
    return;
  }

  ImageType::Pointer itkImage = ImageType::New();
  mitk::CastToItkImage(image, itkImage);

  //Initialize binary image to shape label map filter
  BinaryImageToShapeLabelMapFilterType::Pointer shapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
  shapeLabelMapFilter->SetInputForegroundValue(1);

  shapeLabelMapFilter->SetInput(itkImage);
  shapeLabelMapFilter->Update();

  BinaryImageToShapeLabelMapFilterType::OutputImageType::Pointer labelMap =
    shapeLabelMapFilter->GetOutput();
  for (int i = labelMap->GetNumberOfLabelObjects() - 1; i >= 0; --i)
  {
    // Get the ith region
    BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = labelMap->GetNthLabelObject(i);

    mitk::Vector3D centroid;
    centroid[0] = labelObject->GetCentroid()[0];
    centroid[1] = labelObject->GetCentroid()[1];
    centroid[2] = labelObject->GetCentroid()[2];
    MITK_INFO << "Centroid of segmentation = " << centroid;
  }
}

void QmitkUSNavigationStepCtUsRegistration::OnAddCtImageClicked()
{
  mitk::DataNode* selectedCtImage = ui->ctImagesToChooseComboBox->GetSelectedNode();
  if (selectedCtImage == nullptr)
  {
    return;
  }

  mitk::Image::Pointer ctImage = dynamic_cast<mitk::Image*>(selectedCtImage->GetData());
  if (ctImage.IsNull())
  {
    MITK_WARN << "Failed to cast selected segmentation node to mitk::Image*";
    return;
  }
  QString name = QString::fromStdString(selectedCtImage->GetName());

  for( int counter = 0; counter < ui->chosenCtImagesListWidget->count(); ++counter)
  {
    MITK_INFO << ui->chosenCtImagesListWidget->item(counter)->text() << " - " << counter;
    MITK_INFO << m_ImagesGroundTruthProtocol.at(counter).GetPointer();
    if (ui->chosenCtImagesListWidget->item(counter)->text().compare(name) == 0)
    {
      MITK_INFO << "CT image already exist in list of chosen CT images. Do not add the image.";
      return;
    }
  }

  ui->chosenCtImagesListWidget->addItem(name);
  m_ImagesGroundTruthProtocol.push_back(ctImage);
}

void QmitkUSNavigationStepCtUsRegistration::OnRemoveCtImageClicked()
{
  int position = ui->chosenCtImagesListWidget->currentRow();
  if (ui->chosenCtImagesListWidget->count() == 0 || position < 0)
  {
    return;
  }

  m_ImagesGroundTruthProtocol.erase(m_ImagesGroundTruthProtocol.begin() + position);
  QListWidgetItem *item = ui->chosenCtImagesListWidget->currentItem();
  ui->chosenCtImagesListWidget->removeItemWidget(item);
  delete item;
}

void QmitkUSNavigationStepCtUsRegistration::OnEvaluateGroundTruthFiducialLocalizationProtocol()
{
  m_GroundTruthProtocolFRE.clear();
  if (m_ImagesGroundTruthProtocol.size() != 6)
  {
    QMessageBox msgBox;
    msgBox.setText("For evaluating the Ground-Truth-Fiducial-Localization-Protocol there must be loaded 6 different CT images.");
    msgBox.exec();
    return;
  }

  m_PerformingGroundTruthProtocolEvaluation = true;
  this->CreatePointsToTransformForGroundTruthProtocol();
  m_GroundTruthProtocolTransformedPoints.clear();

  for (unsigned int cycleNo = 0; cycleNo < m_ImagesGroundTruthProtocol.size(); ++cycleNo)
  {
    m_FloatingImage = m_ImagesGroundTruthProtocol.at(cycleNo);
    this->SetFloatingImageGeometryInformation(m_FloatingImage.GetPointer());

    this->OnLocalizeFiducials();
    this->OnRegisterMarkerToFloatingImageCS();
    this->TransformPointsGroundTruthProtocol();
  }
  this->AddTransformedPointsToDataStorage();
  double meanFRE = this->CalculateMeanFRE();
  double sdOfFRE = this->CalculateStandardDeviationOfFRE(meanFRE);
  this->CalculateGroundTruthProtocolTRE();

  ui->meanFREValue->setText(QString("%1").arg(meanFRE));
  ui->sdFREValue->setText(QString("%1").arg(sdOfFRE));
  if (ui->protocolEvaluationTypeComboBox->currentText().compare("ANGLE") == 0)
  {
    if (m_GroundTruthProtocolTRE.find(0) != m_GroundTruthProtocolTRE.end())
    {
      ui->TREValue->setText(QString("%1").arg(m_GroundTruthProtocolTRE.at(0)));
    }
  }
  else if (ui->protocolEvaluationTypeComboBox->currentText().compare("PLANE") == 0)
  {
    if (m_GroundTruthProtocolTRE.find(0) != m_GroundTruthProtocolTRE.end() &&
      m_GroundTruthProtocolTRE.find(20) != m_GroundTruthProtocolTRE.end() &&
      m_GroundTruthProtocolTRE.find(40) != m_GroundTruthProtocolTRE.end() &&
      m_GroundTruthProtocolTRE.find(60) != m_GroundTruthProtocolTRE.end() &&
      m_GroundTruthProtocolTRE.find(80) != m_GroundTruthProtocolTRE.end() &&
      m_GroundTruthProtocolTRE.find(100) != m_GroundTruthProtocolTRE.end())
    {
      ui->TREValue->setText(QString("Depth 0mm: %1\nDepth 20mm: %2\nDepth 40mm: %3\
                                    \nDepth 60mm: %4\nDepth 80mm: %5\nDepth 100mm: %6")
                                    .arg(m_GroundTruthProtocolTRE.at(0))
                                    .arg(m_GroundTruthProtocolTRE.at(20))
                                    .arg(m_GroundTruthProtocolTRE.at(40))
                                    .arg(m_GroundTruthProtocolTRE.at(60))
                                    .arg(m_GroundTruthProtocolTRE.at(80))
                                    .arg(m_GroundTruthProtocolTRE.at(100)));
    }
  }

  m_PerformingGroundTruthProtocolEvaluation = false;
}
