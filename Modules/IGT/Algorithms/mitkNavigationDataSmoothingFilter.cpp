/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNavigationDataSmoothingFilter.h"

mitk::NavigationDataSmoothingFilter::NavigationDataSmoothingFilter()
  : mitk::NavigationDataToNavigationDataFilter(),
    m_NumerOfValues(5)
{
}

mitk::NavigationDataSmoothingFilter::~NavigationDataSmoothingFilter()
{
}

void mitk::NavigationDataSmoothingFilter::GenerateData()
{
  DataObjectPointerArraySizeType numberOfInputs = this->GetNumberOfInputs();

  if ( numberOfInputs == 0 ) return;

  this->CreateOutputsForAllInputs();

  //initialize list if nessesary
  if ( m_LastValuesList.size() != numberOfInputs )
  {
    this->InitializeLastValuesList();
  }

  //add current value to list
  for ( unsigned int i = 0; i < numberOfInputs; ++i )
  {
    this->AddValue(i,this->GetInput(i)->GetPosition());
  }

  //generate output
  for (unsigned int i = 0; i < numberOfInputs; ++i)
  {
    const mitk::NavigationData* nd = this->GetInput(i);
    assert(nd);

    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);

    output->Graft(nd); // copy all information from input to output

    output->SetPosition(GetMean(i));

    output->SetDataValid(nd->IsDataValid());
  }
}

void mitk::NavigationDataSmoothingFilter::InitializeLastValuesList()
{
  m_LastValuesList = std::map< int, std::map< int , mitk::Point3D> >();

  for ( unsigned int i = 0; i < this->GetNumberOfOutputs(); ++i )
  {
    std::map<int,mitk::Point3D> currentList;
    for ( int j = 0; j < m_NumerOfValues; ++j )
    {
      mitk::Point3D emptyPoint;
      emptyPoint.Fill(0);
      currentList.insert(std::pair<int, mitk::Point3D>(j, emptyPoint));
    }
    m_LastValuesList.insert(std::pair<int, std::map<int,mitk::Point3D> > (i,currentList));
  }
}

void mitk::NavigationDataSmoothingFilter::AddValue(int outputID, mitk::Point3D value)
{
  for (int i = 1; i < m_NumerOfValues; ++i)
  {
    m_LastValuesList[outputID][i-1] = m_LastValuesList[outputID][i];
  }

  m_LastValuesList[outputID][m_NumerOfValues-1] = value;
}

mitk::Point3D mitk::NavigationDataSmoothingFilter::GetMean(int outputID)
{
  mitk::Point3D mean;
  mean.Fill(0);
  for (int i=0; i<m_NumerOfValues; i++)
  {
    mean[0] += m_LastValuesList[outputID][i][0];
    mean[1] += m_LastValuesList[outputID][i][1];
    mean[2] += m_LastValuesList[outputID][i][2];
  }
  mean[0] /= m_NumerOfValues;
  mean[1] /= m_NumerOfValues;
  mean[2] /= m_NumerOfValues;
  return mean;
}
