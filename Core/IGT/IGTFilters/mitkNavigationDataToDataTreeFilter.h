/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-01-07 19:23:09 +0100 (Mi, 07 Jan 2009) $
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNAVIGATIONDATATODATATREEFILTER_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATATODATATREEFILTER_H_HEADER_INCLUDED_

#include "mitkNavigationDataVisualizationFilter.h"
#include "mitkNavigationData.h"

namespace mitk {

  /**Documentation
  * @brief Class that reads navigationData from input and transfers the information to associated DataTreeNodes
  *
  * @ingroup Navigation
  */
  class NavigationDataToDataTreeFilter : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(NavigationDataToDataTreeFilter, NavigationDataVisualizationFilter);

    itkNewMacro(Self);

    /** Smart Pointer type to a DataTreeNode. */
    typedef DataTreeNode::Pointer DataTreeNodePointer;

    /** STL map of SmartPointers to DataTreeNodes and an index. Using map to be able to set non continuous indices*/
    typedef std::map<const NavigationData*, DataTreeNodePointer> DataTreeNodePointerMap;
    /** Size type of an std::vector */
    typedef DataTreeNodePointerMap::size_type DataTreeNodePointerMapSizeType;

    /** Set the DataTreeNode of the tool specified by the given index */
    bool SetDataTreeNode(const NavigationData* nd, DataTreeNode* node);
    
    /** Get the DataTreeNode of the tool specified by the given index */
    DataTreeNode* GetDataTreeNode(const NavigationData* nd) const;
  
    /** Get the size of the DataTreeNode vector.  This is merely the size of
    * the vector, not the number of dataTreeNodes that have valid
    * DataObject's assigned. Use GetNumberOfValidRequiredDataTreeNodes() to
    * determine how many DataTreeNodes are non-null. */
    DataTreeNodePointerMapSizeType GetNumberOfDataTreeNodes() const
    {return m_DataTreeNodeList.size();}

  protected:
    NavigationDataToDataTreeFilter();
    ~NavigationDataToDataTreeFilter();

    /** 
    * This method causes the filter 
    * to update the DataTreeNodes of the geometrical objects
    * according to NavigationData. */
    virtual void GenerateData();

  private:
    /** An array of the DataTreeNodes which represent the tools. */
    DataTreeNodePointerMap m_DataTreeNodeList;
  };
} // namespace mitk


#endif /* MITKNAVIGATIONDATATODATATREEFILTER_H_HEADER_INCLUDED_ */

