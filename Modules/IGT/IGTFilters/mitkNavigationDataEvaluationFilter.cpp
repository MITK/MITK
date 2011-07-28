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
