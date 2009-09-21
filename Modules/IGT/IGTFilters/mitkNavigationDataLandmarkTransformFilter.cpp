/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataLandmarkTransformFilter.h"
#include "itkIndent.h"
#include "itkEuler3DTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkPointSet.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
#include <algorithm>


mitk::NavigationDataLandmarkTransformFilter::NavigationDataLandmarkTransformFilter() : mitk::NavigationDataToNavigationDataFilter(), 
m_ErrorMean(-1.0), m_ErrorStdDev(-1.0), m_ErrorRMS(-1.0), m_ErrorMin(-1.0), m_ErrorMax(-1.0), m_ErrorAbsMax(-1.0), 
m_SourcePoints(), m_TargetPoints(), m_LandmarkTransformInitializer(NULL), m_LandmarkTransform(NULL),
m_QuatLandmarkTransform(NULL), m_QuatTransform(NULL), m_Errors(), m_UseICPInitialization(false)
{
  m_LandmarkTransform = LandmarkTransformType::New();

  m_LandmarkTransformInitializer = TransformInitializerType::New();
  m_LandmarkTransformInitializer->SetTransform(m_LandmarkTransform);

  //transform to rotate orientation 
  m_QuatLandmarkTransform = QuaternionTransformType::New();
  m_QuatTransform = QuaternionTransformType::New();
}


mitk::NavigationDataLandmarkTransformFilter::~NavigationDataLandmarkTransformFilter()
{
  m_LandmarkTransform = NULL;
  m_LandmarkTransformInitializer = NULL;
  m_QuatLandmarkTransform = NULL;
  m_QuatTransform = NULL;
}


void mitk::NavigationDataLandmarkTransformFilter::InitializeLandmarkTransform(LandmarkPointContainer& sources, const LandmarkPointContainer& targets)
{
  if (m_UseICPInitialization == true)
    if (this->FindCorrespondentLandmarks(sources, targets) == false) // determine landmark correspondences with iterative closest point optimization, sort sort landmarks accordingly
    {
      itkExceptionMacro("Landmark correspondence finding failed.");
    }
    this->UpdateLandmarkTransform(sources, targets);
}


void mitk::NavigationDataLandmarkTransformFilter::SetSourcePoints(mitk::PointSet::Pointer mitkSourcePointSet)
{
  m_SourcePoints.clear();
  mitk::PointSet::PointType mitkSourcePoint;
  TransformInitializerType::LandmarkPointType lPoint;

  for (mitk::PointSet::PointsContainer::ConstIterator it = mitkSourcePointSet->GetPointSet()->GetPoints()->Begin(); 
    it != mitkSourcePointSet->GetPointSet()->GetPoints()->End(); ++it)
  {
    mitk::FillVector3D(lPoint, it->Value().GetElement(0), it->Value().GetElement(1), it->Value().GetElement(2));
    m_SourcePoints.push_back(lPoint);
  }

  if (m_SourcePoints.size() < 3)
  {
    itkExceptionMacro("SourcePointSet must contain at least 3 points");
  }

  if ((m_SourcePoints.size() >= 3) && (m_TargetPoints.size() >= 3))
    this->InitializeLandmarkTransform(m_SourcePoints, m_TargetPoints);
}


void mitk::NavigationDataLandmarkTransformFilter::SetTargetPoints(mitk::PointSet::Pointer mitkTargetPointSet)
{
  m_TargetPoints.clear();
  TransformInitializerType::LandmarkPointType lPoint;
  for (mitk::PointSet::PointsContainer::ConstIterator it = mitkTargetPointSet->GetPointSet()->GetPoints()->Begin(); 
    it != mitkTargetPointSet->GetPointSet()->GetPoints()->End(); ++it)
  {
    mitk::FillVector3D(lPoint, it->Value().GetElement(0), it->Value().GetElement(1), it->Value().GetElement(2));
    m_TargetPoints.push_back(lPoint);
  }

  if (m_TargetPoints.size() < 3)
  {
    itkExceptionMacro("TargetPointSet must contain at least 3 points");
  }

  if ((m_SourcePoints.size() >= 3) && (m_TargetPoints.size() >= 3))
    this->InitializeLandmarkTransform(m_SourcePoints, m_TargetPoints);
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetFRE() const
{
  return m_ErrorMean;
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetFREStdDev() const
{
  return m_ErrorStdDev;
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetRMSError() const
{
  return m_ErrorRMS;
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetMinError() const
{
  return m_ErrorMin;
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetMaxError() const
{
  return m_ErrorMax;
}


mitk::ScalarType mitk::NavigationDataLandmarkTransformFilter::GetAbsMaxError() const
{
  return m_ErrorAbsMax;
}


void mitk::NavigationDataLandmarkTransformFilter::AccumulateStatistics(std::vector<mitk::ScalarType>& vector) 
{
  //mean, min, max
  m_ErrorMean = 0.0;
  m_ErrorMin = itk::NumericTraits<mitk::ScalarType>::max();
  m_ErrorMax = itk::NumericTraits<mitk::ScalarType>::min();
  m_ErrorAbsMax = 0.0;
  m_ErrorRMS = 0.0;
  for (std::vector<mitk::ScalarType>::size_type i = 0; i < vector.size(); i++)
  {
    m_ErrorMean += vector[i];       // mean
    m_ErrorRMS += pow(vector[i],2); // RMS
    if(vector[i] < m_ErrorMin)      // min
      m_ErrorMin = vector[i];
    if(vector[i] > m_ErrorMax)      // max
      m_ErrorMax = vector[i];
    if(fabs(vector[i]) > fabs(m_ErrorAbsMax)) // abs_max
      m_ErrorAbsMax = vector[i];
  }
  m_ErrorMean /= vector.size(); 
  m_ErrorRMS = sqrt(m_ErrorRMS/vector.size());

  //standard deviation
  m_ErrorStdDev = 0.0;
  for (std::vector<mitk::ScalarType>::size_type i = 0; i < vector.size(); i++)
    m_ErrorStdDev += pow(vector[i] - m_ErrorMean, 2);

  if(vector.size() > 1)
    m_ErrorStdDev = sqrt(m_ErrorStdDev / (vector.size() - 1.0));
  this->Modified();
}


void mitk::NavigationDataLandmarkTransformFilter::GenerateData()
{
  this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

  TransformInitializerType::LandmarkPointType lPointIn, lPointOut;

  /* update outputs with tracking data from tools */
  for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    if (input->IsDataValid() == false)
    {
      output->SetDataValid(false);
      continue;
    }
    output->Graft(input); // First, copy all information from input to output

    if (this->IsInitialized() == false) // as long as there is no valid transformation matrix, only graft the outputs
      continue;

    mitk::NavigationData::PositionType tempCoordinate;
    tempCoordinate = input->GetPosition();
    lPointIn[0] = tempCoordinate[0]; // convert navigation data position to transform point
    lPointIn[1] = tempCoordinate[1];
    lPointIn[2] = tempCoordinate[2];

    /* transform position */
    lPointOut = m_LandmarkTransform->TransformPoint(lPointIn); // transform position
    tempCoordinate[0] = lPointOut[0];  // convert back into navigation data position
    tempCoordinate[1] = lPointOut[1];
    tempCoordinate[2] = lPointOut[2];
    output->SetPosition(tempCoordinate); // update output navigation data with new position

    /* transform orientation */
    NavigationData::OrientationType  quatIn = input->GetOrientation();
    vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());  // convert orientation into vnl quaternion
    m_QuatTransform->SetRotation(vnlQuatIn);  // convert orientation into transform

    m_QuatLandmarkTransform->SetMatrix(m_LandmarkTransform->GetRotationMatrix());  // set rotation from landmark transform
    m_QuatLandmarkTransform->Compose(m_QuatTransform, true); // compose navigation data transform and landmark transform

    vnl_quaternion<double> vnlQuatOut = m_QuatLandmarkTransform->GetRotation();  // convert composed transform back into a quaternion
    NavigationData::OrientationType quatOut(vnlQuatOut[0], vnlQuatOut[1], vnlQuatOut[2], vnlQuatOut[3]); // convert back into navigation data orientation
    output->SetOrientation(quatOut); // update output navigation data with new orientation
    output->SetDataValid(true); // operation was successful, therefore data of output is valid.
  }
}


bool mitk::NavigationDataLandmarkTransformFilter::IsInitialized() const
{
  return (m_SourcePoints.size() >= 3) && (m_TargetPoints.size() >= 3);
}


void mitk::NavigationDataLandmarkTransformFilter::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << this->GetNameOfClass() << ":\n";
  os << indent << m_SourcePoints.size() << " SourcePoints exist: \n";
  itk::Indent nextIndent = indent.GetNextIndent();
  unsigned int i = 0;
  for (LandmarkPointContainer::const_iterator it = m_SourcePoints.begin(); it != m_SourcePoints.end(); ++it)
  {
    os << nextIndent << "Point " << i++ << ": [";
    os << it->GetElement(0);
    for (unsigned int i = 1; i < TransformInitializerType::LandmarkPointType::GetPointDimension(); ++i)
    {
      os << ", " << it->GetElement(i);
    }
    os << "]\n";
  }

  os << indent << m_TargetPoints.size() << " TargetPoints exist: \n";
  i = 0;
  for (LandmarkPointContainer::const_iterator it = m_TargetPoints.begin(); it != m_TargetPoints.end(); ++it)
  {
    os << nextIndent << "Point " << i++ << ": [";
    os << it->GetElement(0);
    for (unsigned int i = 1; i < TransformInitializerType::LandmarkPointType::GetPointDimension(); ++i)
    {
      os << ", " << it->GetElement(i);
    }
    os << "]\n";
  }
  os << indent << "Landmarktransform initialized: " << this->IsInitialized() << "\n";  
  if (this->IsInitialized() == true)
    m_LandmarkTransform->Print(os, nextIndent);
  os << indent << "Registration error statistics:\n";
  os << nextIndent << "FRE: " << this->GetFRE() << "\n";
  os << nextIndent << "FRE std.dev.: " << this->GetFREStdDev() << "\n";
  os << nextIndent << "FRE RMS: " << this->GetRMSError() << "\n";
  os << nextIndent << "Minimum registration error: " << this->GetMinError() << "\n";
  os << nextIndent << "Maximum registration error: " << this->GetMaxError() << "\n";
  os << nextIndent << "Absolute Maximum registration error: " << this->GetAbsMaxError() << "\n";
}


const mitk::NavigationDataLandmarkTransformFilter::ErrorVector& mitk::NavigationDataLandmarkTransformFilter::GetErrorVector() const
{
  return m_Errors;
}


bool mitk::NavigationDataLandmarkTransformFilter::FindCorrespondentLandmarks(LandmarkPointContainer& sources, const LandmarkPointContainer& targets) const
{
  if (sources.size() < 6 || targets.size() < 6)
    return false;
  //throw std::invalid_argument("ICP correspondence finding needs at least 6 landmarks");

  /* lots of type definitions */
  typedef itk::PointSet<mitk::ScalarType, 3> PointSetType;
  //typedef itk::BoundingBox<PointSetType::PointIdentifier, PointSetType::PointDimension> BoundingBoxType;

  typedef itk::EuclideanDistancePointMetric< PointSetType, PointSetType> MetricType;
  //typedef MetricType::TransformType TransformBaseType;
  //typedef MetricType::TransformType::ParametersType ParametersType;
  //typedef TransformBaseType::JacobianType JacobianType;
  //typedef itk::Euler3DTransform< double > TransformType;
  typedef itk::VersorRigid3DTransform< double > TransformType;
  typedef TransformType ParametersType;
  typedef itk::PointSetToPointSetRegistrationMethod< PointSetType, PointSetType > RegistrationType;

  /* copy landmarks to itk pointsets for registration */
  PointSetType::Pointer sourcePointSet = PointSetType::New();
  unsigned int i = 0;
  for (LandmarkPointContainer::const_iterator it = sources.begin(); it != sources.end(); ++it)
  {
    PointSetType::PointType doublePoint;
    mitk::itk2vtk(*it, doublePoint); // copy mitk::ScalarType point into double point as workaround to ITK 3.10 bug
    sourcePointSet->SetPoint(i++, doublePoint /**it*/);
  }

  i = 0;
  PointSetType::Pointer targetPointSet = PointSetType::New();
  for (LandmarkPointContainer::const_iterator it = targets.begin(); it != targets.end(); ++it)
  {
    PointSetType::PointType doublePoint;
    mitk::itk2vtk(*it, doublePoint); // copy mitk::ScalarType point into double point as workaround to ITK 3.10 bug
    targetPointSet->SetPoint(i++, doublePoint /**it*/);
  }

  /* get centroid and extends of our pointsets */
  //BoundingBoxType::Pointer sourceBoundingBox = BoundingBoxType::New();
  //sourceBoundingBox->SetPoints(sourcePointSet->GetPoints());
  //sourceBoundingBox->ComputeBoundingBox();
  //BoundingBoxType::Pointer targetBoundingBox = BoundingBoxType::New();
  //targetBoundingBox->SetPoints(targetPointSet->GetPoints());
  //targetBoundingBox->ComputeBoundingBox();


  TransformType::Pointer transform = TransformType::New();
  transform->SetIdentity();
  //transform->SetTranslation(targetBoundingBox->GetCenter() - sourceBoundingBox->GetCenter());

  itk::LevenbergMarquardtOptimizer::Pointer optimizer = itk::LevenbergMarquardtOptimizer::New();
  optimizer->SetUseCostFunctionGradient(false);

  RegistrationType::Pointer registration = RegistrationType::New();

  // Scale the translation components of the Transform in the Optimizer
  itk::LevenbergMarquardtOptimizer::ScalesType scales(transform->GetNumberOfParameters());
  const double translationScale = 5000; //sqrtf(targetBoundingBox->GetDiagonalLength2())  * 1000; // dynamic range of translations
  const double rotationScale = 1.0; // dynamic range of rotations
  scales[0] = 1.0 / rotationScale;
  scales[1] = 1.0 / rotationScale;
  scales[2] = 1.0 / rotationScale;
  scales[3] = 1.0 / translationScale;
  scales[4] = 1.0 / translationScale;
  scales[5] = 1.0 / translationScale;
  //scales.Fill(0.01);
  unsigned long numberOfIterations = 80000;
  double gradientTolerance = 1e-10; // convergence criterion
  double valueTolerance = 1e-10; // convergence criterion
  double epsilonFunction = 1e-10; // convergence criterion
  optimizer->SetScales( scales );
  optimizer->SetNumberOfIterations( numberOfIterations );
  optimizer->SetValueTolerance( valueTolerance );
  optimizer->SetGradientTolerance( gradientTolerance );
  optimizer->SetEpsilonFunction( epsilonFunction );


  registration->SetInitialTransformParameters( transform->GetParameters() );
  //------------------------------------------------------
  // Connect all the components required for Registration
  //------------------------------------------------------
  MetricType::Pointer metric = MetricType::New();  

  registration->SetMetric( metric );
  registration->SetOptimizer( optimizer );
  registration->SetTransform( transform );
  registration->SetFixedPointSet( targetPointSet );
  registration->SetMovingPointSet( sourcePointSet );

  try
  {
    //registration->StartRegistration();
    registration->Update();
  }
  catch( itk::ExceptionObject & e )
  {
    LOG_INFO << "Exception caught during ICP optimization: " << e;
    return false;
    //throw e;
  }
  LOG_INFO << "ICP successful: Solution = " << transform->GetParameters() << std::endl;
  LOG_INFO << "Metric value: " << metric->GetValue(transform->GetParameters());

  /* find point correspondences */
  //mitk::PointLocator::Pointer pointLocator = mitk::PointLocator::New();  // <<- use mitk::PointLocator instead of searching manually?
  //pointLocator->SetPoints()
  for (LandmarkPointContainer::const_iterator sourcesIt = sources.begin(); sourcesIt != sources.end(); ++sourcesIt)
  {
  }
  //MetricType::MeasureType closestDistances = metric->GetValue(transform->GetParameters());
  //unsigned int index = 0;
  LandmarkPointContainer sortedSources;
  for (LandmarkPointContainer::const_iterator targetsIt = targets.begin(); targetsIt != targets.end(); ++targetsIt)
  {
    double minDistance = itk::NumericTraits<double>::max();
    LandmarkPointContainer::iterator minDistanceIterator = sources.end();
    for (LandmarkPointContainer::iterator sourcesIt = sources.begin(); sourcesIt != sources.end(); ++sourcesIt)
    {
      TransformInitializerType::LandmarkPointType transformedSource = transform->TransformPoint(*sourcesIt);
      double dist = targetsIt->EuclideanDistanceTo(transformedSource);
      LOG_INFO << "target: " << *targetsIt << ", source: " << *sourcesIt << ", transformed source: " << transformedSource << ", dist: " << dist;
      if (dist < minDistance )
      {
        minDistanceIterator = sourcesIt;
        minDistance = dist;
      }      
    }
    if (minDistanceIterator == sources.end())
      return false;
    LOG_INFO << "minimum distance point is: " << *minDistanceIterator << " (dist: " << targetsIt->EuclideanDistanceTo(transform->TransformPoint(*minDistanceIterator)) << ", minDist: " << minDistance << ")";
    sortedSources.push_back(*minDistanceIterator); // this point is assigned
    sources.erase(minDistanceIterator); // erase it from sources to avoid duplicate assigns
  }
  //for (LandmarkPointContainer::const_iterator sortedSourcesIt = sortedSources.begin(); targetsIt != sortedSources.end(); ++targetsIt)
  sources = sortedSources;
  return true;
}

void mitk::NavigationDataLandmarkTransformFilter::UpdateLandmarkTransform(const LandmarkPointContainer &sources,  const LandmarkPointContainer &targets)
{
  try
  {
    /* calculate transform from landmarks */
    m_LandmarkTransformInitializer->SetMovingLandmarks(targets);
    m_LandmarkTransformInitializer->SetFixedLandmarks(sources);    // itk registration always maps from fixed object space to moving object space
    m_LandmarkTransform->SetIdentity();
    m_LandmarkTransformInitializer->InitializeTransform();

    LOG_INFO << "Landmarktransform updated. New Parameters: " << m_LandmarkTransform->GetParameters();
    /* Calculate error statistics for the transform */
    TransformInitializerType::LandmarkPointType curData;
    m_Errors.clear();
    for (LandmarkPointContainer::size_type index = 0; index < sources.size(); index++)
    {
      curData = m_LandmarkTransform->TransformPoint(sources.at(index));
      m_Errors.push_back(curData.EuclideanDistanceTo(targets.at(index)));
    }
    this->AccumulateStatistics(m_Errors);
    this->Modified();
  }
  catch (std::exception& e)
  {
    m_Errors.clear();
    m_LandmarkTransform->SetIdentity();
    itkExceptionMacro("Initializing landmark-transform failed\n. " << e.what());
  }
}