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


#include "mitkPointSetStatisticsCalculator.h"

mitk::PointSetStatisticsCalculator::PointSetStatisticsCalculator()
{
  m_PointSet = mitk::PointSet::New();
}

mitk::PointSetStatisticsCalculator::PointSetStatisticsCalculator(mitk::PointSet::Pointer pSet)
{
  m_PointSet = pSet;
}

mitk::PointSetStatisticsCalculator::~PointSetStatisticsCalculator()
{
}

void mitk::PointSetStatisticsCalculator::SetPointSet(mitk::PointSet::Pointer pSet)
{
  if (pSet.IsNull()) return;
  m_PointSet = pSet;
}

std::vector<mitk::Point3D> mitk::PointSetStatisticsCalculator::PointSetToVector(mitk::PointSet::Pointer pSet)
{
  std::vector<mitk::Point3D> returnValue = std::vector<mitk::Point3D>();
  for (int i=0; i<pSet->GetSize(); i++) returnValue.push_back(pSet->GetPoint(i));
  return returnValue;
}

double mitk::PointSetStatisticsCalculator::GetMax(std::vector<double> list)
{
if (list.empty()) return 0;
std::sort(list.begin(), list.end());
return list.at(list.size()-1);
}

double mitk::PointSetStatisticsCalculator::GetMin(std::vector<double> list)
{
if (list.empty()) return 0;
std::sort(list.begin(), list.end());
return list.at(0);
}

double mitk::PointSetStatisticsCalculator::GetStabw(std::vector<double> list)
{
if (list.empty()) return 0;
double returnValue = 0;
double mean = GetMean(list);
for(std::vector<double>::size_type i=0; i<list.size(); i++)
  {
  returnValue += pow((list.at(i)-mean),2);
  }

returnValue /= list.size();
returnValue = sqrt(returnValue);
return returnValue;
}

double mitk::PointSetStatisticsCalculator::GetSampleStabw(std::vector<double> list)
{
if (list.empty()) return 0;
double returnValue = 0;
double mean = GetMean(list);
for(std::vector<double>::size_type i=0; i<list.size(); i++)
  {
  returnValue += pow((list.at(i)-mean),2);
  }

returnValue /= (list.size()-1);
returnValue = sqrt(returnValue);
return returnValue;
}

double mitk::PointSetStatisticsCalculator::GetMean(std::vector<double> list)
{
if (list.empty()) return 0;
double mean = 0;
for(std::vector<double>::size_type i=0; i<list.size(); i++)
  {
  mean += list.at(i);
  }
mean /= list.size();
return mean;
}

double mitk::PointSetStatisticsCalculator::GetMedian(std::vector<double> list)
{
if (list.empty()) return 0;
std::sort(list.begin(), list.end());
if (list.size() % 2 == 0.) //even
  {
  double element1 = list.at(list.size()/2);
  double element2 = list.at(list.size()/2);
  return ((element1+element2)/2.0);
  }
else //odd
  {
  return list.at((list.size())/2);
  }
}

mitk::Point3D mitk::PointSetStatisticsCalculator::GetMean(std::vector<mitk::Point3D> list)
{
if (list.empty()) 
  {
    mitk::Point3D emptyPoint;
    emptyPoint.Fill(0);
    return emptyPoint;
  }

//calculate mean
mitk::Point3D mean;
mean.Fill(0);

for (std::vector<mitk::Point3D>::size_type i=0; i<list.size(); i++)
  {
  mean[0] += list.at(i)[0];
  mean[1] += list.at(i)[1];
  mean[2] += list.at(i)[2];
  }

mean[0] /= list.size();
mean[1] /= list.size();
mean[2] /= list.size();

return mean;
}

double mitk::PointSetStatisticsCalculator::GetPositionErrorMean()
{
double returnValue = 0.0;

if (CheckIfAllPositionsAreEqual()) return returnValue;

std::vector<mitk::Point3D> pSet = PointSetToVector(m_PointSet);

if (pSet.empty()) return 0;

mitk::Point3D mean = GetMean(pSet);

for(std::vector<mitk::Point3D>::size_type i=0; i<pSet.size(); i++)
  {
  returnValue += mean.EuclideanDistanceTo(pSet.at(i));
  }
returnValue /= pSet.size();

return returnValue;
}

double mitk::PointSetStatisticsCalculator::GetPositionErrorStandardDeviation()
{
return GetStabw(GetErrorList(PointSetToVector(m_PointSet)));
}

double mitk::PointSetStatisticsCalculator::GetPositionErrorSampleStandardDeviation()
{
return GetSampleStabw(GetErrorList(PointSetToVector(m_PointSet)));
}


double mitk::PointSetStatisticsCalculator::GetPositionErrorRMS()
{
double returnValue = 0.0;

if (CheckIfAllPositionsAreEqual()) return returnValue;

std::vector<mitk::Point3D> pSet = PointSetToVector(m_PointSet);

if(pSet.empty()) return 0;

mitk::Point3D mean = GetMean(pSet);

for(std::vector<mitk::Point3D>::size_type i=0; i<pSet.size(); i++)
  {
  returnValue += pow(mean.EuclideanDistanceTo(pSet.at(i)),2);
  }
returnValue /= pSet.size();
returnValue = sqrt(returnValue);

return returnValue;
}

double mitk::PointSetStatisticsCalculator::GetPositionErrorMedian()
{
return GetMedian(GetErrorList(PointSetToVector(m_PointSet)));
}

double mitk::PointSetStatisticsCalculator::GetPositionErrorMax()
{
return GetMax(GetErrorList(PointSetToVector(m_PointSet)));
}
  
double mitk::PointSetStatisticsCalculator::GetPositionErrorMin()
{
return GetMin(GetErrorList(PointSetToVector(m_PointSet)));
}

std::vector<double> mitk::PointSetStatisticsCalculator::GetErrorList(std::vector<mitk::Point3D> list)
{
std::vector<double> errorList = std::vector<double>();
mitk::Point3D mean = GetMean(list);
if (CheckIfAllPositionsAreEqual()) for(std::vector<mitk::Point3D>::size_type i=0; i<list.size(); i++) {errorList.push_back(0.0);}
else for(std::vector<mitk::Point3D>::size_type i=0; i<list.size(); i++) {errorList.push_back(mean.EuclideanDistanceTo(list.at(i)));}
return errorList;
}

mitk::Vector3D mitk::PointSetStatisticsCalculator::GetPositionStandardDeviation()
{
mitk::Vector3D returnValue;

if (CheckIfAllPositionsAreEqual())
  {
  returnValue[0]=0;
  returnValue[1]=0;
  returnValue[2]=0;
  return returnValue;
  }

std::vector<mitk::Point3D> pSet = PointSetToVector(m_PointSet);
std::vector<double> listX = std::vector<double>();
std::vector<double> listY = std::vector<double>();
std::vector<double> listZ = std::vector<double>();
for (std::vector<mitk::Point3D>::size_type i=0; i<pSet.size(); i++)
  {
  listX.push_back(pSet.at(i)[0]);
  listY.push_back(pSet.at(i)[1]);
  listZ.push_back(pSet.at(i)[2]);
  }
returnValue[0] = GetStabw(listX);
returnValue[1] = GetStabw(listY);
returnValue[2] = GetStabw(listZ);
return returnValue;
}

mitk::Vector3D mitk::PointSetStatisticsCalculator::GetPositionSampleStandardDeviation()
{
mitk::Vector3D returnValue;

if (CheckIfAllPositionsAreEqual())
  {
  returnValue[0]=0;
  returnValue[1]=0;
  returnValue[2]=0;
  return returnValue;
  }

std::vector<mitk::Point3D> pSet = PointSetToVector(m_PointSet);
std::vector<double> listX = std::vector<double>();
std::vector<double> listY = std::vector<double>();
std::vector<double> listZ = std::vector<double>();
for (std::vector<mitk::Point3D>::size_type i=0; i<pSet.size(); i++)
  {
  listX.push_back(pSet.at(i)[0]);
  listY.push_back(pSet.at(i)[1]);
  listZ.push_back(pSet.at(i)[2]);
  }
returnValue[0] = GetSampleStabw(listX);
returnValue[1] = GetSampleStabw(listY);
returnValue[2] = GetSampleStabw(listZ);
return returnValue;
}

mitk::Point3D mitk::PointSetStatisticsCalculator::GetPositionMean()
{
return GetMean(PointSetToVector(m_PointSet));
}

bool mitk::PointSetStatisticsCalculator::CheckIfAllPositionsAreEqual()
{
if (m_PointSet->GetSize()==0) return false;
if (m_PointSet->GetSize()==1) return true;

mitk::Point3D lastPoint = m_PointSet->GetPoint(0);
for(int i=1; i<m_PointSet->GetSize(); i++)
  {
  if((m_PointSet->GetPoint(i)[0]!=lastPoint[0])||(m_PointSet->GetPoint(i)[1]!=lastPoint[1])||(m_PointSet->GetPoint(i)[2]!=lastPoint[2])) return false;
  lastPoint = m_PointSet->GetPoint(i);
  }

return true;
}
