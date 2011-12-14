/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkNavigationDataEvaluationFilter.h"
#include <mitkPointSetStatisticsCalculator.h>

#define _USE_MATH_DEFINES
#include <math.h>

mitk::NavigationDataEvaluationFilter::NavigationDataEvaluationFilter()
: mitk::NavigationDataToNavigationDataFilter()
{

}


mitk::NavigationDataEvaluationFilter::~NavigationDataEvaluationFilter()
{

}

void mitk::NavigationDataEvaluationFilter::GenerateData()
{

    this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs
    this->CreateMembersForAllInputs();

    /* update outputs with tracking data from tools */
    for (unsigned int i = 0; i < this->GetNumberOfOutputs() ; ++i)
    {
      //first copy outputs to inputs
      mitk::NavigationData* output = this->GetOutput(i);
      assert(output);
      const mitk::NavigationData* input = this->GetInput(i);
      assert(input);
      if (input->IsDataValid() == false) {output->SetDataValid(false);}
      else {output->Graft(input);}

      //then save statistics
      if(input->IsDataValid())
        {
        m_LoggedPositions[i].push_back(input->GetPosition());
        m_LoggedQuaternions[i].push_back(input->GetOrientation());
        }
      else 
        {
        m_InavildSamples[i]++;
        }
    }
  
}
void mitk::NavigationDataEvaluationFilter::CreateMembersForAllInputs()
{
  while(this->m_LoggedPositions.size() < this->GetNumberOfInputs())
    {
    std::pair<int,std::vector<mitk::Point3D> > newElement(m_LoggedPositions.size(),std::vector<mitk::Point3D>());
    m_LoggedPositions.insert(newElement);
    }
  while(this->m_LoggedQuaternions.size() < this->GetNumberOfInputs())
    {
    std::pair<int,std::vector<mitk::Quaternion> > newElement(m_LoggedQuaternions.size(),std::vector<mitk::Quaternion>());
    m_LoggedQuaternions.insert(newElement);
    }
  while(this->m_InavildSamples.size() < this->GetNumberOfInputs())
    {
    std::pair<int,int> newElement(m_LoggedQuaternions.size(),0);
    m_InavildSamples.insert(newElement);
    }

  
}

void mitk::NavigationDataEvaluationFilter::ResetStatistic()
{
for (int i = 0; i < m_LoggedPositions.size(); i++) m_LoggedPositions[i] = std::vector<mitk::Point3D>();
for (int i = 0; i < m_LoggedQuaternions.size(); i++) m_LoggedQuaternions[i] = std::vector<mitk::Quaternion>();
for (int i = 0; i < m_InavildSamples.size(); i++) m_InavildSamples[i] = 0;
}

int mitk::NavigationDataEvaluationFilter::GetNumberOfAnalysedNavigationData(int input)
{
return this->m_LoggedPositions[input].size();
}


mitk::Point3D mitk::NavigationDataEvaluationFilter::GetPositionMean(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionMean();
}

mitk::Vector3D mitk::NavigationDataEvaluationFilter::GetPositionStandardDeviation(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionStandardDeviation();
}

mitk::Vector3D mitk::NavigationDataEvaluationFilter::GetPositionSampleStandardDeviation(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionSampleStandardDeviation();
}

mitk::Quaternion mitk::NavigationDataEvaluationFilter::GetQuaternionMean(int input)
{
return GetMean(m_LoggedQuaternions[input]);
}

mitk::Quaternion mitk::NavigationDataEvaluationFilter::GetQuaternionStandardDeviation(int input)
{
mitk::Quaternion returnValue;
std::vector<double> list1 = std::vector<double>();
std::vector<double> list2 = std::vector<double>();
std::vector<double> list3 = std::vector<double>();
std::vector<double> list4 = std::vector<double>();
for (int i=0; i<m_LoggedQuaternions[input].size(); i++)
  {
  list1.push_back(m_LoggedQuaternions[input].at(i)[0]);
  list2.push_back(m_LoggedQuaternions[input].at(i)[1]);
  list3.push_back(m_LoggedQuaternions[input].at(i)[2]);
  list4.push_back(m_LoggedQuaternions[input].at(i)[3]);
  }
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New();
returnValue[0] = myCalculator->GetStabw(list1);
returnValue[1] = myCalculator->GetStabw(list2);
returnValue[2] = myCalculator->GetStabw(list3);
returnValue[3] = myCalculator->GetStabw(list4);
return returnValue;
}

mitk::Vector3D mitk::NavigationDataEvaluationFilter::GetEulerAnglesMean(int input)
{ 
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(QuaternionsToEulerAngles(m_LoggedQuaternions[input])));
mitk::Vector3D returnValue;
returnValue[0] = myCalculator->GetPositionMean()[0];
returnValue[1] = myCalculator->GetPositionMean()[1];
returnValue[2] = myCalculator->GetPositionMean()[2];
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetEulerAnglesRMS(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(QuaternionsToEulerAngles(m_LoggedQuaternions[input])));
return myCalculator->GetPositionErrorRMS();
}

double mitk::NavigationDataEvaluationFilter::GetEulerAnglesRMSDegree(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(QuaternionsToEulerAnglesGrad(m_LoggedQuaternions[input])));
return myCalculator->GetPositionErrorRMS();
}



double mitk::NavigationDataEvaluationFilter::GetPositionErrorMean(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorMean();
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorStandardDeviation(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorStandardDeviation();
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorSampleStandardDeviation(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorSampleStandardDeviation();
}


double mitk::NavigationDataEvaluationFilter::GetPositionErrorRMS(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorRMS();
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorMedian(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorMedian();
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorMax(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorMax();
}
  
double mitk::NavigationDataEvaluationFilter::GetPositionErrorMin(int input)
{
mitk::PointSetStatisticsCalculator::Pointer myCalculator = mitk::PointSetStatisticsCalculator::New(VectorToPointSet(m_LoggedPositions[input]));
return myCalculator->GetPositionErrorMin();
}

int mitk::NavigationDataEvaluationFilter::GetNumberOfInvalidSamples(int input)
{
return m_InavildSamples[input];
}

double mitk::NavigationDataEvaluationFilter::GetPercentageOfInvalidSamples(int input)
{
return (m_InavildSamples[input]/(m_InavildSamples[input]+((double)m_LoggedPositions[input].size())))*100.0;
}

mitk::Quaternion mitk::NavigationDataEvaluationFilter::GetMean(std::vector<mitk::Quaternion> list)
{
//calculate mean
mitk::Quaternion mean;
mean[0] = 0;
mean[1] = 0;
mean[2] = 0;
mean[3] = 0;

for (int i=0; i<list.size(); i++)
  {
  mean[0] += list.at(i)[0];
  mean[1] += list.at(i)[1];
  mean[2] += list.at(i)[2];
  mean[3] += list.at(i)[3];
  }

mean[0] /= list.size();
mean[1] /= list.size();
mean[2] /= list.size();
mean[3] /= list.size();

return mean;
}

mitk::PointSet::Pointer mitk::NavigationDataEvaluationFilter::VectorToPointSet(std::vector<mitk::Point3D> pSet)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  for (int i=0; i<pSet.size(); i++) returnValue->InsertPoint(i,pSet.at(i)); 
  return returnValue;
}

mitk::PointSet::Pointer mitk::NavigationDataEvaluationFilter::VectorToPointSet(std::vector<mitk::Vector3D> pSet)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  for (int i=0; i<pSet.size(); i++) 
    {
    mitk::Point3D thisPoint;
    thisPoint[0] = pSet.at(i)[0];
    thisPoint[1] = pSet.at(i)[1];
    thisPoint[2] = pSet.at(i)[2];
    returnValue->InsertPoint(i,thisPoint); 
    }
  return returnValue;
}

std::vector<mitk::Vector3D> mitk::NavigationDataEvaluationFilter::QuaternionsToEulerAngles(std::vector<mitk::Quaternion> quaterions)
{
  std::vector<mitk::Vector3D> returnValue = std::vector<mitk::Vector3D>();
  for (int i=0; i<quaterions.size(); i++) 
    {
    mitk::Vector3D eulerAngles;
    mitk::Quaternion currentQuaternion = quaterions.at(i);
    currentQuaternion.normalize(); //must be normalized due to the documentation of the vnl method rotation_euler_angles()
    eulerAngles[0] = currentQuaternion.rotation_euler_angles()[0];
    eulerAngles[1] = currentQuaternion.rotation_euler_angles()[1];
    eulerAngles[2] = currentQuaternion.rotation_euler_angles()[2];
    returnValue.push_back(eulerAngles);
    }
  return returnValue;
}

std::vector<mitk::Vector3D> mitk::NavigationDataEvaluationFilter::QuaternionsToEulerAnglesGrad(std::vector<mitk::Quaternion> quaterions)
{
  double PI = M_PI;
  std::vector<mitk::Vector3D> returnValue = std::vector<mitk::Vector3D>();
  std::vector<mitk::Vector3D> eulerAnglesRadians = QuaternionsToEulerAngles(quaterions);
  for (int i=0; i<eulerAnglesRadians.size(); i++)
    {
    mitk::Vector3D currentAngles;
    currentAngles[0] = (eulerAnglesRadians.at(i)[0]/PI)*180;
    currentAngles[1] = (eulerAnglesRadians.at(i)[1]/PI)*180;
    currentAngles[2] = (eulerAnglesRadians.at(i)[2]/PI)*180;
    returnValue.push_back(currentAngles);
    }
  return returnValue;
}