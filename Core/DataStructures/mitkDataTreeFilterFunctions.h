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
#include <mitkProperties.h>

namespace mitk
{
  class DataTreeNode; 

  /// base class for all filter function that are accepted by mitk::DataTreeFilter
  class DataTreeFilterFunction
  {
    public:
      virtual ~DataTreeFilterFunction() {}
      virtual bool operator()(DataTreeNode*) const;
      virtual bool NodeMatches(DataTreeNode*) const = 0;
      virtual DataTreeFilterFunction* Clone() const = 0;
  };

  /// tests the data entry of nodes for a specific type (given here as template parameter)
  template <class T>
  class IsBaseDataType : public DataTreeFilterFunction
  {
    public:
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return ( node != NULL && node->GetData() && dynamic_cast<T*>(node->GetData()) );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsBaseDataType<T>();
      }

      virtual ~IsBaseDataType() {}
  };

  /// tests the data entry of nodes for a specific type (given here as template parameter)
  /// AND having a given property
  template <class T>
  class IsBaseDataTypeWithProperty : public DataTreeFilterFunction
  {
    public:

      IsBaseDataTypeWithProperty(const char* propertyName)
      :m_PropertyName(propertyName)
      {
      }
      
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return (    node != NULL && node->GetData()      // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<T*>(node->GetData() )   // data is of a certain type
                 && (   node->GetProperty(m_PropertyName.c_str()).IsNotNull()    // there is a certain property
                    )
                );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsBaseDataTypeWithProperty<T>(m_PropertyName.c_str());
      }

      virtual ~IsBaseDataTypeWithProperty() {}

    private:
      
      std::string m_PropertyName;
  };

  /// tests the data entry of nodes for a specific type (given here as template parameter)
  /// AND for NOT having a given property (or it being a false bool property)
  template <class T>
  class IsBaseDataTypeWithoutProperty : public DataTreeFilterFunction
  {
    public:

      IsBaseDataTypeWithoutProperty(const char* propertyName)
      :m_PropertyName(propertyName)
      {
      }
      
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return (    node != NULL && node->GetData()      // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<T*>(node->GetData() )   // data is of a certain type
                 && (   node->GetProperty(m_PropertyName.c_str()).IsNull()    // there either is NO property
                      || (       ( dynamic_cast<BoolProperty*>( node->GetProperty(m_PropertyName.c_str()).GetPointer() )) // OR the property is a BoolProperty that is false
                            && ( (dynamic_cast<BoolProperty*>( node->GetProperty(m_PropertyName.c_str()).GetPointer()))->GetValue() == false )
                         )
                    )
                );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsBaseDataTypeWithoutProperty<T>(m_PropertyName.c_str());
      }

      virtual ~IsBaseDataTypeWithoutProperty() {}

    private:
      
      std::string m_PropertyName;
  };


  // some default filters in mitk:: namespace for use by clients of mitk::DataTreeFilter
  
  /// Accepts all nodes
  /// (accepts nodes that are not NULL)
  class IsDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual ~IsDataTreeNode() {}
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

  /// Accepts all data objects
  /// (accepts nodes that have associated mitk::BaseData (tested via GetData))
  class IsGoodDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual ~IsGoodDataTreeNode() {}
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

} // namespace mitk

#endif
// vi: textwidth=90

