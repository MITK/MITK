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


#ifndef MITKNAVIGATIONDATAVISUALIZATIONBYBASEDATATRANSFORMFILTER_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATAVISUALIZATIONBYBASEDATATRANSFORMFILTER_H_HEADER_INCLUDED_

#include "mitkNavigationDataVisualizationFilter.h"
#include "mitkNavigationData.h"

namespace mitk {

  /**Documentation
  * @brief Class that reads navigationData from input and transfers the information to associated DataTreeNodes
  *
  * @ingroup Navigation
  */
  class NavigationDataVisualizationByBaseDataTransformFilter : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(NavigationDataVisualizationByBaseDataTransformFilter, NavigationDataVisualizationFilter);

    itkNewMacro(Self);

    /** Smart Pointer type to a DataTreeNode. */
    typedef BaseData::Pointer RepresentationPointer;

    /** STL map of SmartPointers to DataTreeNodes and an index. Using map to be able to set non continuous indices*/
    typedef std::map<const NavigationData*, RepresentationPointer> RepresentationPointerMap;
    /** Size type of an std::vector */
    typedef RepresentationPointerMap::size_type RepresentationPointerMapSizeType;

    /** Set the DataTreeNode of the tool specified by the given index */
    bool SetBaseData(const NavigationData* nd, BaseData* data);
    
    /** Get the DataTreeNode of the tool specified by the given index */
    const BaseData* GetBaseData(const NavigationData* nd) const;
  
    /** Get the size of the DataTreeNode vector.  This is merely the size of
    * the vector, not the number of dataTreeNodes that have valid
    * DataObject's assigned. Use GetNumberOfValidRequiredDataTreeNodes() to
    * determine how many DataTreeNodes are non-null. */
    RepresentationPointerMapSizeType GetNumberOfToolRepresentations() const
    {return m_RepresentationList.size();}

    /*
    * @brief Transfer the informations from the input to the associated BaseData
    */
    virtual void GenerateData();


  protected:
    NavigationDataVisualizationByBaseDataTransformFilter();
    ~NavigationDataVisualizationByBaseDataTransformFilter();


    /**
    * @brief Create a BaseData that represents the given tool
    **/
    void CreateToolRepresenation();

  private:
    /** An array of the DataTreeNodes which represent the tools. */
    RepresentationPointerMap m_RepresentationList;
  };
} // namespace mitk


#endif /* MITKNAVIGATIONDATAVISUALIZATIONBYBASEDATATRANSFORMFILTER_H_HEADER_INCLUDED_ */


