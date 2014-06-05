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

#include "mitkNavigationDataSequentialPlayer.h"

#include <itksys/SystemTools.hxx> //for the pause
#include <fstream>
#include <sstream>

//Exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

mitk::NavigationDataSequentialPlayer::NavigationDataSequentialPlayer()
{
}

mitk::NavigationDataSequentialPlayer::~NavigationDataSequentialPlayer()
{
}

void mitk::NavigationDataSequentialPlayer::GoToSnapshot(unsigned int i)
{
  if( !m_Repeat && (this->GetNumberOfSnapshots() <= i) )
  {
    MITK_ERROR << "Snaphot " << i << " does not exist and repat is off: can't go to that snapshot!";
    mitkThrowException(mitk::IGTException) << "Snapshot " << i << " does not exist and repat is off: can't go to that snapshot!";
  }

  // set iterator to given position (modulo for allowing repeat)
  m_NavigationDataSetIterator = m_NavigationDataSet->Begin() + ( i % this->GetNumberOfSnapshots() );

  // set outputs to selected snapshot
  this->GenerateData();
}

bool mitk::NavigationDataSequentialPlayer::GoToNextSnapshot()
{
  if (m_NavigationDataSetIterator == m_NavigationDataSet->End())
  {
    MITK_WARN("NavigationDataSequentialPlayer") << "Cannot go to next snapshot, already at end of NavigationDataset. Ignoring...";
    return false;
  }
  ++m_NavigationDataSetIterator;
  if ( m_NavigationDataSetIterator == m_NavigationDataSet->End() )
  {
    if ( m_Repeat )
    {
      // set data back to start if repeat is enabled
      m_NavigationDataSetIterator = m_NavigationDataSet->Begin();
    }
    else
    {
      return false;
    }
  }
  this->GenerateData();
  return true;
}

void mitk::NavigationDataSequentialPlayer::GenerateData()
{
  if ( m_NavigationDataSetIterator == m_NavigationDataSet->End() )
  {
    // no more data available
    this->GraftEmptyOutput();
  }
  else
  {
    for (unsigned int index = 0; index < GetNumberOfOutputs(); index++)
    {
      mitk::NavigationData* output = this->GetOutput(index);
      if( !output ) { mitkThrowException(mitk::IGTException) << "Output of index "<<index<<" is null."; }

      output->Graft(m_NavigationDataSetIterator->at(index));
    }
  }
}

void mitk::NavigationDataSequentialPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}
