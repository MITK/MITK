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
  : mitk::NavigationDataPlayerBase()
  , m_DataElem(0)
  , m_CurrentElem(0)
  , m_Repeat(false)
  , m_NumberOfSnapshots(0)
  , m_LastGoTo(0)
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
    mitkThrowException(mitk::IGTException) << "Snaphot " << i << " does not exist and repat is off: can't go to that snapshot!";
  }

  // set iterator to given position (modulo for allowing repeat)
  m_NavigationDataSetIterator = m_NavigationDataSet->Begin() + ( i % this->GetNumberOfSnapshots() );

  this->Update();
}

void mitk::NavigationDataSequentialPlayer::GenerateData()
{
  if ( m_NavigationDataSetIterator == m_NavigationDataSet->End() )
  {
     if ( m_Repeat )
     {
       // set data back to start if repeat is enabled
       m_NavigationDataSetIterator = m_NavigationDataSet->Begin();
     }
     else
     {
       // no more data available
       this->GraftEmptyOutput();
       return;
     }
  }


  for (unsigned int index = 0; index < m_NumberOfOutputs; index++)
  {
    mitk::NavigationData* output = this->GetOutput(index);
    if( !output ) { mitkThrowException(mitk::IGTException) << "Output of index "<<index<<" is null."; }

    output->Graft(m_NavigationDataSetIterator->at(index));
  }

  ++m_NavigationDataSetIterator;
}

void mitk::NavigationDataSequentialPlayer::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}
