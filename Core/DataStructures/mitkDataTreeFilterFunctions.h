/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <mitkDataTreeNode.h>

#include <mitkProperties.h>
#include <mitkDataStorage.h>

namespace mitk
{
  class DataTreeNode; 

  /*! \brief Base class for all filter function that are accepted by mitk::DataTreeFilter.

    Subclasses are required to implement the Clone() method, which should return a copy of
    the object, and the NodeMatches() method. NodeMatches() will receive a
    mitk::DataTreeNode* everytime it is called, and should return true. <b>This pointer can be NULL</b>. 
  */
   class DataTreeFilterFunction
  {
    public:
      virtual ~DataTreeFilterFunction() {}
      virtual bool operator()(DataTreeNode*) const;
      virtual bool NodeMatches(DataTreeNode*) const = 0;
      virtual DataTreeFilterFunction* Clone() const = 0;
  };

  /*! \brief tests the data entry of nodes for a specific type (given here as template parameter).

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataType<mitk::Image>() );
    \endcode
  */
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

  /*! \brief Tests the data entry of nodes for a specific type (given here as template parameter) AND having a given property.

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithProperty<mitk::Surface>("volume") );
    \endcode
  */
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
        return (    node != NULL && node->GetData()                               // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<T*>(node->GetData() )                            // data is of a certain type
                 && (   node->GetProperty(m_PropertyName.c_str())!= NULL          // there is a certain property
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

  /*! \brief Tests the data entry of nodes for a specific type (given here as template parameter) AND having a TRUE BoolProperty.

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("segmentation") );
    \endcode
  */
  template <class T>
  class IsBaseDataTypeWithBoolProperty : public DataTreeFilterFunction
  {
    public:

      IsBaseDataTypeWithBoolProperty(const char* propertyName)
      :m_PropertyName(propertyName)
      {
      }
      
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        bool propVal(false);
        return (    node != NULL && node->GetData()                            // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<T*>( node->GetData() )                       // data is of a certain type
                 && node->GetPropertyValue(m_PropertyName.c_str(), propVal)  // there is a certain BoolProperty
                 && propVal
                );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsBaseDataTypeWithBoolProperty<T>(m_PropertyName.c_str());
      }

      virtual ~IsBaseDataTypeWithBoolProperty() {}

    private:
      
      std::string m_PropertyName;
  };


  /*! \brief Tests the data entry of nodes for a specific type (given here as template parameter) AND for NOT having a given property (or it being a false bool property).

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("segmentation") );
    \endcode
    */
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
        bool propVal(false);
        bool propertyExists(false);

        if (node)
          propertyExists = node->GetPropertyValue(m_PropertyName.c_str(), propVal );

        return (    node != NULL && node->GetData()         // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<T*>(node->GetData() )     // data is of a certain type
                 && ( !propertyExists || !propVal ));     // the property does not exist, OR its value is false
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
  
  /*! \brief Accepts all nodes (accepts nodes that are not NULL).

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("segmentation") );
    \endcode
  */
  class IsDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual ~IsDataTreeNode() {}
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

  /*! \brief Accepts all data objects (accepts nodes that have associated mitk::BaseData (tested via GetData)).

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("segmentation") );
    \endcode
  */
  class IsGoodDataTreeNode : public DataTreeFilterFunction
  {
    public:
      virtual ~IsGoodDataTreeNode() {}
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
  };

  /*! \brief Accepts all data objects (accepts nodes that have associated mitk::BaseData (tested via GetData)).

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("segmentation") );
    \endcode
  */
  class IsInResultSet : public DataTreeFilterFunction
  {
    public:
      IsInResultSet( std::set<const DataTreeNode*> rs )
        :m_ResultSet(rs)
      {
      }

      IsInResultSet(const DataStorage::SetOfObjects* rs)
      {
        if (rs)
        {
          for (DataStorage::SetOfObjects::const_iterator iter = rs->begin();
               iter != rs->end();
               ++iter)
          {
            m_ResultSet.insert( iter->GetPointer() );
          }
        }
      }

      virtual ~IsInResultSet() {}
      virtual bool NodeMatches(DataTreeNode*) const;
      virtual DataTreeFilterFunction* Clone() const;
    protected:
      //DataStorage::SetOfObjects::ConstPointer m_ResultSet;
      std::set<const DataTreeNode*> m_ResultSet;
  };

  /*! \brief Tests if the node contains an image with a specified dimensionality (template parameter)

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::ImageWithDimensionAndProperty<3>("volume") );
    \endcode
  */
  template <unsigned int DIM>
  class IsImageWithDimensionAndWithoutProperty : public DataTreeFilterFunction
  {
    public:

      IsImageWithDimensionAndWithoutProperty(const char* propertyName)
      :m_PropertyName(propertyName)
      {
      }
      
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return (    node != NULL && node->GetData()                                // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<mitk::Image*>(node->GetData() )                            // data is an image
                 && (dynamic_cast<mitk::Image*>(node->GetData() )->GetDimension() == DIM)
                 && (   node->GetProperty(m_PropertyName.c_str()) == NULL    // there is a certain property
                    )
                );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsImageWithDimensionAndWithoutProperty<DIM>(m_PropertyName.c_str());
      }

      virtual ~IsImageWithDimensionAndWithoutProperty() {}

    private:
      
      std::string m_PropertyName;
  };

    /*! \brief Tests if the node contains an image with a dimensionality greater or equal to the template parameter

    To be used with mitk::DataTreeFilter, e.g.
    \code
     treeFilter->SetFilter( mitk::IsImageWithMinimumDimension<3>() );
    \endcode
  */
  template <unsigned int DIM>
  class IsImageWithMinimumDimension : public DataTreeFilterFunction
  {
    public:
      
      virtual bool NodeMatches(DataTreeNode* node) const
      {
        return (    node != NULL && node->GetData()                                // node is not NULL, and node->GetData is also not NULL
                 && dynamic_cast<mitk::Image*>(node->GetData() )                            // data is an image
                 && (dynamic_cast<mitk::Image*>(node->GetData() )->GetDimension() >= DIM)
                );
      }

      virtual DataTreeFilterFunction* Clone() const
      {
        return new IsImageWithMinimumDimension<DIM>();
      }

      virtual ~IsImageWithMinimumDimension() {}
  };

} // namespace mitk

#endif
// vi: textwidth=90

