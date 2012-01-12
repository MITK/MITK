/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPointSetDifferenceStatisticsCalculator.h"

mitk::PointSetDifferenceStatisticsCalculator::PointSetDifferenceStatisticsCalculator() :
  m_StatisticsCalculated(false)
{
  m_PointSet1 = mitk::PointSet::New();
  m_PointSet2 = mitk::PointSet::New();
  m_Statistics.Reset();
}

mitk::PointSetDifferenceStatisticsCalculator::PointSetDifferenceStatisticsCalculator(mitk::PointSet::Pointer pSet1, mitk::PointSet::Pointer pSet2)
{
  m_PointSet1 = pSet1;
  m_PointSet2 = pSet2;
  m_StatisticsCalculated = false;
  m_Statistics.Reset();
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
  m_Statistics.Reset();
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
  return m_Statistics.Mean;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetSD()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.Sigma;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetVariance()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.Variance;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetRMS()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.RMS;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMedian()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.Median;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMax()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.Max;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetMin()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.Min;
}

double mitk::PointSetDifferenceStatisticsCalculator::GetNumberOfPoints()
{
  if (!m_StatisticsCalculated)
  {
    this->ComputeStatistics();
  }
  return m_Statistics.N;
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
    int numberOfPoints = m_PointSet1->GetSize();
    for (int i=0; i<numberOfPoints; i++)
    {
      point1 = m_PointSet1->GetPoint(i);
      point2 = m_PointSet2->GetPoint(i);
      double squaredDistance = point1.SquaredEuclideanDistanceTo(point2);
      mean+=sqrt(squaredDistance);
      rms+=squaredDistance;
      this->m_SquaredDifferencesVector.push_back(squaredDistance);
      differencesVector.push_back(sqrt(squaredDistance));
    }
    m_DifferencesVector = differencesVector;
    mean = mean/numberOfPoints;
    rms = sqrt(rms/numberOfPoints);
    for (std::vector<double>::size_type i=0; i<differencesVector.size(); i++)
    {
      sd+=(differencesVector.at(i)-mean)*(differencesVector.at(i)-mean);
    }
    double variance = sd/numberOfPoints;
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
    m_Statistics.Mean = mean;
    m_Statistics.Sigma = sd;
    m_Statistics.Variance = variance;
    m_Statistics.RMS = rms;
    m_Statistics.Min = differencesVector.at(0);
    m_Statistics.Max = differencesVector.at(numberOfPoints-1);
    m_Statistics.Median = median;
    m_Statistics.N = numberOfPoints;

    m_StatisticsCalculated = true;
  }
}
