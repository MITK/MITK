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
    std::pair<int,std::vector<mitk::Point3D>> newElement(m_LoggedPositions.size(),std::vector<mitk::Point3D>());
    m_LoggedPositions.insert(newElement);
    }
  while(this->m_LoggedQuaternions.size() < this->GetNumberOfInputs())
    {
    std::pair<int,std::vector<mitk::Quaternion>> newElement(m_LoggedQuaternions.size(),std::vector<mitk::Quaternion>());
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
mitk::Point3D returnValue;
returnValue.Fill(0);

for (int i=0; i<m_LoggedPositions[input].size(); i++)
  {
  returnValue[0] += m_LoggedPositions[input].at(i)[0];
  returnValue[1] += m_LoggedPositions[input].at(i)[1];
  returnValue[2] += m_LoggedPositions[input].at(i)[2];
  }

returnValue[0] /= m_LoggedPositions[input].size();
returnValue[1] /= m_LoggedPositions[input].size();
returnValue[2] /= m_LoggedPositions[input].size();

return returnValue;
}

mitk::Vector3D mitk::NavigationDataEvaluationFilter::GetPositionStandardDerivation(int input)
{
mitk::Vector3D returnValue;
//TODO
return returnValue;
}

mitk::Quaternion mitk::NavigationDataEvaluationFilter::GetQuaternionMean(int input)
{
mitk::Quaternion returnValue;

returnValue[0] = 0;
returnValue[1] = 0;
returnValue[2] = 0;
returnValue[3] = 0;

for (int i=0; i<m_LoggedQuaternions[input].size(); i++)
  {
  returnValue[0] += m_LoggedQuaternions[input].at(i)[0];
  returnValue[1] += m_LoggedQuaternions[input].at(i)[1];
  returnValue[2] += m_LoggedQuaternions[input].at(i)[2];
  returnValue[3] += m_LoggedQuaternions[input].at(i)[3];
  }

returnValue[0] /= m_LoggedQuaternions[input].size();
returnValue[1] /= m_LoggedQuaternions[input].size();
returnValue[2] /= m_LoggedQuaternions[input].size();
returnValue[3] /= m_LoggedQuaternions[input].size();

return returnValue;
}

mitk::Quaternion mitk::NavigationDataEvaluationFilter::GetQuaternionStandardDerivation(int input)
{
mitk::Quaternion returnValue;
//TODO
return returnValue;
}


double mitk::NavigationDataEvaluationFilter::GetPositionErrorMean(int input)
{
double returnValue = 0.0;
mitk::Point3D mean = this->GetPositionMean(input);

for(int i=0; i<m_LoggedPositions[input].size(); i++)
  {
  returnValue += mean.EuclideanDistanceTo(m_LoggedPositions[input].at(i));
  }
returnValue /= m_LoggedPositions[input].size();

return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorStandardDerication(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorRMS(int input)
{
double returnValue = 0.0;

mitk::Point3D mean = this->GetPositionMean(input);

for(int i=0; i<m_LoggedPositions[input].size(); i++)
  {
  returnValue += pow(mean.EuclideanDistanceTo(m_LoggedPositions[input].at(i)),2);
  }
returnValue /= m_LoggedPositions[input].size();
returnValue = sqrt(returnValue);

return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetPositionErrorMedian(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetQuaternionErrorMean(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetQuaternionErrorStandardDerication(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetQuaternionErrorRMS(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

double mitk::NavigationDataEvaluationFilter::GetQuaternionErrorMedian(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}

int GetNumberOfInvalidSamples(int input)
{
int returnValue = 0;
//TODO
return returnValue;
}

double GetPercentageOfInvalidSamples(int input)
{
double returnValue = 0.0;
//TODO
return returnValue;
}