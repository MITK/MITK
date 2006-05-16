/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITK_DATATREEFILTERFUNCTOINS_H_INCLUDED
#define MITK_DATATREEFILTERFUNCTOINS_H_INCLUDED

#include <mitkConfig.h>
#include <mitkDataTreeNode.h>

#include <mitkImage.h>
#include <mitkSurface.h>

namespace mitk
{
  class DataTreeNode; 

  // some default filters in mitk:: namespace for use by clients of mitk::DataTreeFilter
  
  /// Accepts all nodes
  /// (accepts nodes that are not NULL)
  bool IsDataTreeNode(DataTreeNode*);

  /// Accepts all data objects
  /// (accepts nodes that have associated mitk::BaseData (tested via GetData))
  bool IsGoodDataTreeNode(DataTreeNode*);

  /// to be used with any BaseData derived class
  /// IsBaseDataType<mitk::Image>
  template <class T>
  bool IsBaseDataType(DataTreeNode* node)
  {
    return ( node!= 0 && node->GetData() && dynamic_cast<T*>( node->GetData() ) );
  }

} // namespace mitk

#endif
// vi: textwidth=90

