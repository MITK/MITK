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

  class DataTreeFilterFunction
  {
    public:
      virtual bool operator()(DataTreeNode*) const;
      virtual bool NodeMatches(DataTreeNode*) const = 0;
      virtual DataTreeFilterFunction* Clone() const = 0;
  };

  template <class T>
  class IsBaseDataType : public DataTreeFilterFunction
  {
    public:
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return ( node != NULL && node->GetData() && dynamic_cast<T*>(node) );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsBaseDataType<T>();
      }
  };

  // some default filters in mitk:: namespace for use by clients of mitk::DataTreeFilter
  
  /// Accepts all nodes
  /// (accepts nodes that are not NULL)
  class IsDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

  /// Accepts all data objects
  /// (accepts nodes that have associated mitk::BaseData (tested via GetData))
  class IsGoodDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

} // namespace mitk

#endif
// vi: textwidth=90

