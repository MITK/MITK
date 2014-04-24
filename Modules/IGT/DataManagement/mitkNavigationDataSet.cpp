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

#include "mitkNavigationDataSet.h"

mitk::NavigationDataSet::NavigationDataSet( unsigned int numberOfTools )
  : m_NavigationDataVectors(std::vector<std::vector<mitk::NavigationData::Pointer> >()), m_NumberOfTools(numberOfTools)
{
}

mitk::NavigationDataSet::~NavigationDataSet( )
{
}

bool mitk::NavigationDataSet::AddNavigationDatas( std::vector<mitk::NavigationData::Pointer> navigationDatas )
{
  // test if tool with given index exist
  if ( navigationDatas.size() != m_NumberOfTools )
  {
    MITK_WARN("NavigationDataSet") << "Tried to add to many or too few navigation Datas to NavigationDataSet. " << m_NavigationDataVectors.size() << " required, tried to add " << navigationDatas.size() << ".";
    return false;
  }

  // test for consistent timestamp
  if ( m_NavigationDataVectors.size() > 0)
  {
    for (int i = 0; i < navigationDatas.size(); i++)
      if (navigationDatas[i]->GetIGTTimeStamp() <= m_NavigationDataVectors.back()[i]->GetIGTTimeStamp())
      {
        MITK_WARN("NavigationDataSet") << "IGTTimeStamp of new NavigationData should be newer than timestamp of last NavigationData.";
        return false;
      }
  }

  m_NavigationDataVectors.push_back(navigationDatas);
  return true;
}

mitk::NavigationData::Pointer mitk::NavigationDataSet::GetNavigationDataForIndex( unsigned int index, unsigned int toolIndex ) const
{
  if ( index >= m_NavigationDataVectors.size() )
  {
    MITK_WARN("NavigationDataSet") << "There is no NavigationData available at index " << index << ".";
    return NULL;
  }

  if ( toolIndex >= m_NavigationDataVectors.at(index).size() )
  {
    MITK_WARN("NavigationDataSet") << "There is NavigatitionData available at index " << index << " for tool " << toolIndex << ".";
    return NULL;
  }

  return m_NavigationDataVectors.at(index).at(toolIndex);
}

// Method not yet supported, code below compiles but delivers wrong results
//mitk::NavigationData::Pointer mitk::NavigationDataSet::GetNavigationDataBeforeTimestamp(
//  mitk::NavigationData::TimeStampType timestamp, unsigned int toolIndex) const
//{
//  if ( toolIndex >= m_NavigationDataVectors.size() )
//  {
//    MITK_WARN("NavigationDataSet") << "There is no tool with index " << toolIndex << ".";
//    return NULL;
//  }
//
//  std::vector<mitk::NavigationData::Pointer>::const_iterator it;
//
//  // iterate through all NavigationData objects of the given tool index
//  // till the timestamp of the NavigationData is greater then the given timestamp
//  for (it = m_NavigationDataVectors.at(toolIndex).begin();
//    it != m_NavigationDataVectors.at(toolIndex).end(); ++it)
//  {
//    if ( (*it)->GetIGTTimeStamp() > timestamp) { break; }
//  }
//
//  // first element was greater than timestamp -> return null
//  if ( it == m_NavigationDataVectors.at(toolIndex).begin() )
//  {
//    MITK_WARN("NavigationDataSet") << "No NavigationData was recorded before given timestamp.";
//    return NULL;
//  }
//
//  // return last element smaller than the given timestamp
//  return *(it-1);
//}

std::vector< mitk::NavigationData::Pointer > mitk::NavigationDataSet::GetDataStreamForTool(unsigned int toolIndex)
{
  if (toolIndex >= m_NumberOfTools )
  {
    MITK_WARN("NavigationDataSet") << "Invalid toolIndex: " << m_NumberOfTools << " Tools known, requested index " << toolIndex << "";
    return std::vector<mitk::NavigationData::Pointer>();
  }

  std::vector< mitk::NavigationData::Pointer > result;

  for(int i = 0; i < m_NavigationDataVectors.size(); i++)
    result.push_back(m_NavigationDataVectors[i][toolIndex]);

  return result;
}

std::vector< mitk::NavigationData::Pointer > mitk::NavigationDataSet::GetTimeStep(unsigned int index)
{
  return m_NavigationDataVectors[index];
}

unsigned int mitk::NavigationDataSet::GetNumberOfTools()
{
  return m_NumberOfTools;
}

unsigned int mitk::NavigationDataSet::Size()
{
  return m_NavigationDataVectors.size();
}

// ---> methods necessary for BaseData
void mitk::NavigationDataSet::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::NavigationDataSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::NavigationDataSet::VerifyRequestedRegion()
{
  return true;
}

void mitk::NavigationDataSet::SetRequestedRegion(const DataObject * )
{
}
// <--- methods necessary for BaseData

// ---> methods for Iterators

mitk::NavigationDataSet::NavigationDataSetIterator mitk::NavigationDataSet::Begin()
{
  return m_NavigationDataVectors.begin();
}

mitk::NavigationDataSet::NavigationDataSetIterator mitk::NavigationDataSet::End()
{
  return m_NavigationDataVectors.end();
}