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


#include "mitkPointSetDifferenceStatisticsCalculator.h"
#include "mitkImageStatisticsConstants.h"

mitk::PointSetDifferenceStatisticsCalculator::PointSetDifferenceStatisticsCalculator() :
  m_StatisticsCalculated(false)
{
  m_PointSet1 = mitk::PointSet::New();
  m_PointSet2 = mitk::PointSet::New();
  //m_Statistics.Reset();
}

mitk::PointSetDifferenceStatisticsCalculator::PointSetDifferenceStatisticsCalculator(mitk::PointSet::Pointer pSet1, mitk::PointSet::Pointer pSet2)
{
  m_PointSet1 = pSet1;
  m_PointSet2 = pSet2;
  m_StatisticsCalculated = false;
  //m_Statistics.Reset();
}

mitk::PointSetDifferenceStatisticsCalculator::~PointSetDifferenceStatisticsCalculator()
{
}

void mitk::PointSetDifferenceStatisticsCalculator::SetPointSets(mitk::PointSet::Pointer pSet1, mitk::PointSet::Pointer pSet2)
{
  if (pSet1.IsNotNull())
  {
    m_PointSet1 = pSet1;
  }
  if (pSet2.IsNotNull())
  {
    m_PointSet2 = pSet2;
  }
  m_StatisticsCalculated = false;
  //m_Statistics.Reset();
}

std::vector<double> mitk::PointSetDifferenceStatisticsCalculator::GetDifferences()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_DifferencesVector;
}

std::vector<double> mitk::PointSetDifferenceStatisticsCalculator::GetSquaredDifferences()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_SquaredDifferencesVector;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMean()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetSD()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::STANDARDDEVIATION());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetVariance()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::VARIANCE());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetRMS()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::RMS());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMedian()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEDIAN());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMax()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MAXIMUM());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMin()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MINIMUM());
}

double mitk::PointSetDifferenceStatisticsCalculator::GetNumberOfPoints()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.GetValueConverted<ImageStatisticsContainer::VoxelCountType>(mitk::ImageStatisticsConstants::NUMBEROFVOXELS());
}

void mitk::PointSetDifferenceStatisticsCalculator::ComputeStatistics()
{
  if ((m_PointSet1.IsNull())||(m_PointSet2.IsNull()))
  {
    itkExceptionMacro("Point sets specified are not valid. Please specify correct Point sets");
  }
  else if (m_PointSet1->GetSize()!=m_PointSet2->GetSize())
  {
    itkExceptionMacro("PointSets are not equal. Please make sure that your PointSets have the same size and hold corresponding points.");
  }
  else if (m_PointSet1->GetSize()==0)
  {
    itkExceptionMacro("There are no points in the PointSets. Please make sure that the PointSets contain points");
  }
  else
  {
    double mean = 0.0;
    double sd = 0.0;
    double rms= 0.0;

    std::vector<double> differencesVector;
    mitk::Point3D point1;
    mitk::Point3D point2;
    auto numberOfPoints = static_cast<mitk::ImageStatisticsContainer::VoxelCountType>(m_PointSet1->GetSize());

    //Iterate over both pointsets in order to compare all points pair-wise
    mitk::PointSet::PointsIterator end = m_PointSet1->End();
    for( mitk::PointSet::PointsIterator pointSetIterator = m_PointSet1->Begin(), pointSetIterator2 = m_PointSet2->Begin();
         pointSetIterator != end; ++pointSetIterator, ++pointSetIterator2) //iterate simultaneously over both sets
    {
      point1 = pointSetIterator.Value();
      point2 = pointSetIterator2.Value();

      double squaredDistance = point1.SquaredEuclideanDistanceTo(point2);
      mean+=sqrt(squaredDistance);
      rms+=squaredDistance;
      this->m_SquaredDifferencesVector.push_back(squaredDistance);
      differencesVector.push_back(sqrt(squaredDistance));
    }

    m_DifferencesVector = differencesVector;
    mean = mean/static_cast<double>(numberOfPoints);
    rms = sqrt(rms/ static_cast<double>(numberOfPoints));
    for (std::vector<double>::size_type i=0; i<differencesVector.size(); i++)
    {
      sd+=(differencesVector.at(i)-mean)*(differencesVector.at(i)-mean);
    }
    double variance = sd/ static_cast<double>(numberOfPoints);
    sd = sqrt(variance);
    std::sort(differencesVector.begin(),differencesVector.end());
    double median = 0.0;
    if (numberOfPoints%2 == 0)
    {
      median = (differencesVector.at(numberOfPoints/2)+differencesVector.at(numberOfPoints/2-1))/2;
    }
    else
    {
      median = differencesVector.at((numberOfPoints-1)/2+1);
    }
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::MEAN(), mean);
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::STANDARDDEVIATION(), sd);
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::VARIANCE(), sd*sd);
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::RMS(), rms);
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::MINIMUM(), differencesVector.at(0));
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::MAXIMUM(), differencesVector.at(numberOfPoints - 1));
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::MEDIAN(), median);
    m_Statistics.AddStatistic(mitk::ImageStatisticsConstants::NUMBEROFVOXELS(), numberOfPoints);

    m_StatisticsCalculated = true;
  }
}
