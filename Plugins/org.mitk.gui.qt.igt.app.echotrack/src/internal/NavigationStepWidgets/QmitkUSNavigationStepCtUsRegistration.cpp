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

static const int NUMBER_FIDUCIALS_NEEDED = 8; 

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

void QmitkUSNavigationStepCtUsRegistration::CalculatePCA()
{
  //Step 1: Construct data matrix
  int columnSize = m_CentroidsOfFiducialCandidates.size();
  if (columnSize == 0)
  {
    MITK_INFO << "Cannot calculate PCA. There are no fiducial candidates.";
    return;
  }

  vnl_matrix<double> pointSetMatrix(3, columnSize, 0.0);
  for (int counter = 0; counter < columnSize; ++counter)
  {
    pointSetMatrix[0][counter] = m_CentroidsOfFiducialCandidates.at(counter)[0];
    pointSetMatrix[1][counter] = m_CentroidsOfFiducialCandidates.at(counter)[1];
    pointSetMatrix[2][counter] = m_CentroidsOfFiducialCandidates.at(counter)[2];
  }

  //Step 2: Remove average for each row (Mittelwertbefreiung)
  for (int counter = 0; counter < columnSize; ++counter)
  {
    m_MeanCentroidFiducialCandidates += mitk::Vector3D(pointSetMatrix.get_column(counter));
  }
  //TODO: für später überprüfen, ob Division durch integer nicht zu Rechenproblemen führt.
  m_MeanCentroidFiducialCandidates /= columnSize;
  for (int counter = 0; counter < columnSize; ++counter)
  {
    pointSetMatrix.get_column(counter) -= m_MeanCentroidFiducialCandidates;
  }

  //Step 3: Compute covariance matrix
  vnl_matrix<double> covarianceMatrix = (1.0 / (columnSize - 1.0)) * pointSetMatrix * pointSetMatrix.transpose();

  //Step 4: Singular value composition
  vnl_svd<double> svd(covarianceMatrix);

  //Storing results:
  for (int counter = 0; counter < 3; ++counter)
  {
    mitk::Vector3D eigenVector = svd.U().get_column(counter);
    double eigenValue = sqrt(svd.W(counter));
    m_EigenVectorsFiducialCandidates[eigenValue] = eigenVector;
    m_EigenValuesFiducialCandidates.push_back(eigenValue);
  }
  std::sort( m_EigenValuesFiducialCandidates.begin(), m_EigenValuesFiducialCandidates.end() );

  mitk::DataNode::Pointer axis1Node = mitk::DataNode::New();
  axis1Node->SetName("Eigenvector 1");
  mitk::PointSet::Pointer axis1 = mitk::PointSet::New();
  axis1->InsertPoint(0, m_MeanCentroidFiducialCandidates);
  axis1->InsertPoint(1, (m_MeanCentroidFiducialCandidates + m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(2))*m_EigenValuesFiducialCandidates.at(2)));
  axis1Node->SetData(axis1);
  axis1Node->SetBoolProperty("show contour", true);
  axis1Node->SetColor(1, 0, 0);
  this->GetDataStorage()->Add(axis1Node);

  mitk::DataNode::Pointer axis2Node = mitk::DataNode::New();
  axis2Node->SetName("Eigenvector 2");
  mitk::PointSet::Pointer axis2 = mitk::PointSet::New();
  axis2->InsertPoint(0, m_MeanCentroidFiducialCandidates);
  axis2->InsertPoint(1, (m_MeanCentroidFiducialCandidates + m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(1))*m_EigenValuesFiducialCandidates.at(1)));
  axis2Node->SetData(axis2);
  axis2Node->SetBoolProperty("show contour", true);
  axis2Node->SetColor(2, 0, 0);
  this->GetDataStorage()->Add(axis2Node);

  mitk::DataNode::Pointer axis3Node = mitk::DataNode::New();
  axis3Node->SetName("Eigenvector 3");
  mitk::PointSet::Pointer axis3 = mitk::PointSet::New();
  axis3->InsertPoint(0, m_MeanCentroidFiducialCandidates);
  axis3->InsertPoint(1, (m_MeanCentroidFiducialCandidates + m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(0))*m_EigenValuesFiducialCandidates.at(0)));
  axis3Node->SetData(axis3);
  axis3Node->SetBoolProperty("show contour", true);
  axis3Node->SetColor(3, 0, 0);
  this->GetDataStorage()->Add(axis3Node);

  MITK_INFO << "Mean: " << m_MeanCentroidFiducialCandidates;

  MITK_INFO << "Eigenvektor 1: " << m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(2));
  MITK_INFO << "Eigenvektor 2: " << m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(1));
  MITK_INFO << "Eigenvektor 3: " << m_EigenVectorsFiducialCandidates.at(m_EigenValuesFiducialCandidates.at(0));

  MITK_INFO << "Eigenwert 1: " << m_EigenValuesFiducialCandidates.at(2);
  MITK_INFO << "Eigenwert 2: " << m_EigenValuesFiducialCandidates.at(1);
  MITK_INFO << "Eigenwert 3: " << m_EigenValuesFiducialCandidates.at(0);

}

void QmitkUSNavigationStepCtUsRegistration::NumerateFiducialMarks()
{
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
  for (int counter = 1; counter <= m_FiducialMarkerCentroids.size(); ++counter)
  {
    m_MarkerFloatingImageCoordinateSystemPointSet->InsertPoint(counter - 1, m_FiducialMarkerCentroids.at(counter));
  }
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(m_MarkerFloatingImageCoordinateSystemPointSet);
  node->SetName("MarkerFloatingImageCSPointSet");
  //node->SetFloatProperty("pointsize", 5.0);
  this->GetDataStorage()->Add(node);
}

void QmitkUSNavigationStepCtUsRegistration::CalculateDistancesBetweenFiducials(std::vector<std::vector<double>>& distanceVectorsFiducials)
{
  std::vector<double> distancesBetweenFiducials;

  for (int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    distancesBetweenFiducials.clear();
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    for (int n = 0; n < m_CentroidsOfFiducialCandidates.size(); ++n)
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

  for (int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    MITK_INFO << "Vector " << i << ":";
    for (int k = 0; k < distanceVectorsFiducials.at(i).size(); ++k)
    {
      MITK_INFO << distanceVectorsFiducials.at(i).at(k);
    }
  }
}

bool QmitkUSNavigationStepCtUsRegistration::FindFiducialNo1(std::vector<std::vector<double>>& distanceVectorsFiducials)
{
  for (int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    std::vector<double> &distances = distanceVectorsFiducials.at(i);
    if (distances.size() < NUMBER_FIDUCIALS_NEEDED - 1 )
    {
      MITK_WARN << "Cannot find fiducial, there aren't found enough fiducial candidates.";
      return false;
    }

    if (distances.at(0) <= 12.07 && distances.at(1) <= 12.07)
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
  mitk::Point3D fiducialNo1(m_FiducialMarkerCentroids.at(1));
  mitk::Vector3D fiducialVectorA;
  mitk::Vector3D fiducialVectorB;
  mitk::Point3D fiducialPointA;
  mitk::Point3D fiducialPointB;
  bool foundFiducialA = false;
  bool foundFiducialB = false;
  mitk::Vector3D vectorFiducial1ToFiducialA;
  mitk::Vector3D vectorFiducial1ToFiducialB;


  for (int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    double distance = fiducialNo1.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 12.07)
    {
      fiducialVectorA = m_CentroidsOfFiducialCandidates.at(i);
      fiducialPointA = fiducialCentroid;
      m_CentroidsOfFiducialCandidates.erase(m_CentroidsOfFiducialCandidates.begin() + i);
      foundFiducialA = true;
      break;
    }
  }

  for (int i = 0; i < m_CentroidsOfFiducialCandidates.size(); ++i)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(i));
    double distance = fiducialNo1.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 12.07)
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
  for (int i = 0; i < distanceVectorsFiducials.size(); ++i)
  {
    std::vector<double> &distances = distanceVectorsFiducials.at(i);
    if (distances.size() < NUMBER_FIDUCIALS_NEEDED - 1)
    {
      MITK_WARN << "Cannot find fiducial, there aren't found enough fiducial candidates.";
      return false;
    }

    if (distances.at(0) > 12.07 && distances.at(0) <= 15.73 &&
        distances.at(1) > 12.07 && distances.at(1) <= 15.73)
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

  mitk::Point3D fiducialNo2(m_FiducialMarkerCentroids.at(2));

  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo2.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 15.73)
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

  mitk::Point3D fiducialNo5(m_FiducialMarkerCentroids.at(5));

  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo5.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 12.07)
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

  mitk::Point3D fiducialNo8(m_FiducialMarkerCentroids.at(8));

  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo8.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 12.07)
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

  mitk::Point3D fiducialNo3(m_FiducialMarkerCentroids.at(3));

  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    mitk::Point3D fiducialCentroid(m_CentroidsOfFiducialCandidates.at(counter));
    double distance = fiducialNo3.EuclideanDistanceTo(fiducialCentroid);
    if (distance <= 15.73)
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

  // create signal/slot connections
  connect(ui->floatingImageComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode*)));
  connect(ui->fiducialMarkerModelPointSetComboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
    this, SLOT(OnFiducialMarkerModelComboBoxSelectionChanged(const mitk::DataNode*)));
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
  {*/
    mitk::PointSet* pointSet_orig = m_MarkerModelCoordinateSystemPointSet;
    mitk::PointSet::Pointer pointSet_moved = mitk::PointSet::New();

    for (int i = 0; i < pointSet_orig->GetSize(); i++)
    {
      pointSet_moved->InsertPoint(m_TransformMarkerCSToFloatingImageCS->TransformPoint(pointSet_orig->GetPoint(i)));
    }

    pointSet_orig->Clear();
    for (int i = 0; i < pointSet_moved->GetSize(); i++)
      pointSet_orig->InsertPoint(pointSet_moved->GetPoint(i));
  /*}*/

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

    mitk::Vector3D centroid;
    centroid[0] = labelObject->GetCentroid()[0];
    centroid[1] = labelObject->GetCentroid()[1];
    centroid[2] = labelObject->GetCentroid()[2];
    m_CentroidsOfFiducialCandidates.push_back(centroid);
  }
  //evtl. for later: itk::LabelMapOverlayImageFilter

  mitk::CastToMitkImage(binaryImage, m_FloatingImage);
  
  /*mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  for (int counter = 0; counter < m_CentroidsOfFiducialCandidates.size(); ++counter)
  {
    pointSet->InsertPoint(counter, m_CentroidsOfFiducialCandidates.at(counter)); //Alternativ:  InsertPoint()
  }
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(pointSet);
  node->SetName("PointSet");
  node->SetFloatProperty("pointsize", 5.0);
  this->GetDataStorage()->Add(node);*/

  //this->CalculatePCA();
  this->NumerateFiducialMarks();

}


