/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-09 18:14:41 +0200 (Mo, 09 Jul 2007) $
Version:   $Revision: 11185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkNavigationDataToDataTreeFilter.h"


mitk::NavigationDataToDataTreeFilter::NavigationDataToDataTreeFilter()
{}


mitk::NavigationDataToDataTreeFilter::~NavigationDataToDataTreeFilter()
{}


void mitk::NavigationDataToDataTreeFilter::GenerateData()
{
  //for (unsigned int i = 0; i < m_Input->GetToolCount(); ++i)
  //{
  //  mitk__DataTreeNode* node = this->GetDataTreeNode(i);       

  //  mitk::NavigationData* nd = this->GetOutput(i);
  //  assert(nd);
  //  mitk::NavigationData::PositionType p;
  //  p = nd->GetPosition();

  //  mitk::NavigationData::OrientationType o;
  //  0 = nd->GetOrientation();

  //}

}


mitk::DataTreeNode* mitk::NavigationDataToDataTreeFilter::GetDataTreeNode(const NavigationData* nd) const
{
  DataTreeNodePointerMap::const_iterator iter = m_DataTreeNodeList.find(nd);
  if (iter != m_DataTreeNodeList.end())
    return (*iter).second;
  else
  return NULL;
}

bool mitk::NavigationDataToDataTreeFilter::SetDataTreeNode(const NavigationData* nd, DataTreeNode* node)
{
  std::pair<DataTreeNodePointerMap::iterator, bool> returnEl;
  returnEl.second = false;
  
  //insert the given elements
  if (node != NULL)
    returnEl = m_DataTreeNodeList.insert( DataTreeNodePointerMap::value_type(nd, node) );
  
  return returnEl.second;
}

