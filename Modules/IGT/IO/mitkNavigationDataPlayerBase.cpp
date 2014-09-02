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

#include "mitkNavigationDataPlayerBase.h"

// include for exceptions
#include "mitkIGTException.h"

mitk::NavigationDataPlayerBase::NavigationDataPlayerBase()
  : m_Repeat(false)
{
  this->SetName("Navigation Data Player Source");
}

mitk::NavigationDataPlayerBase::~NavigationDataPlayerBase()
{
}

void mitk::NavigationDataPlayerBase::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}

bool mitk::NavigationDataPlayerBase::IsAtEnd()
{
  return m_NavigationDataSetIterator == m_NavigationDataSet->End();
}

void mitk::NavigationDataPlayerBase::SetNavigationDataSet(NavigationDataSet::Pointer navigationDataSet)
{
  m_NavigationDataSet = navigationDataSet;
  m_NavigationDataSetIterator = navigationDataSet->Begin();

  this->InitPlayer();
}

unsigned int mitk::NavigationDataPlayerBase::GetNumberOfSnapshots()
{
  return m_NavigationDataSet.IsNull() ? 0 : m_NavigationDataSet->Size();
}

unsigned int mitk::NavigationDataPlayerBase::GetCurrentSnapshotNumber()
{
  return m_NavigationDataSet.IsNull() ? 0 : m_NavigationDataSetIterator - m_NavigationDataSet->Begin();
}

void mitk::NavigationDataPlayerBase::InitPlayer()
{
  if ( m_NavigationDataSet.IsNull() )
  {
    mitkThrowException(mitk::IGTException)
      << "NavigationDataSet has to be set before initializing player.";
  }

  if (GetNumberOfOutputs() == 0)
  {
    int requiredOutputs = m_NavigationDataSet->GetNumberOfTools();
    this->SetNumberOfRequiredOutputs(requiredOutputs);

    for (unsigned int n = this->GetNumberOfOutputs(); n < requiredOutputs; ++n)
    {
      DataObjectPointer newOutput = this->MakeOutput(n);
      this->SetNthOutput(n, newOutput);
      this->Modified();
    }
  }
  else if (GetNumberOfOutputs() != m_NavigationDataSet->GetNumberOfTools())
  {
    mitkThrowException(mitk::IGTException)
      << "Number of tools cannot be changed in existing player. Please create "
      << "a new player, if the NavigationDataSet has another number of tools now.";
  }

  this->Modified();
  this->GenerateData();
}

void mitk::NavigationDataPlayerBase::GraftEmptyOutput()
{
  for (unsigned int index = 0; index < m_NavigationDataSet->GetNumberOfTools(); index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);
    assert(output);

    mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
    mitk::NavigationData::PositionType position;
    mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
    position.Fill(0.0);

    nd->SetPosition(position);
    nd->SetOrientation(orientation);
    nd->SetDataValid(false);

    output->Graft(nd);
  }
}
