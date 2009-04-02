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

  /**
  * \brief Class that reads NavigationData from input and transfers the information to the geometry of the associated BaseData
  *
  * Derived from NavigationDataVisualizationFilter
  *
  * \ingroup IGT
  */
  class NavigationDataVisualizationByBaseDataTransformFilter : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(NavigationDataVisualizationByBaseDataTransformFilter, NavigationDataVisualizationFilter);

    itkNewMacro(Self);

    /** 
    * \brief Smart Pointer type to a BaseData. 
    */
    typedef BaseData::Pointer RepresentationPointer;

    /** 
    * \brief STL map of SmartPointers to BaseData and an index. Using map to be able to set non continuous indices
    */
    typedef std::map<const NavigationData*, RepresentationPointer> RepresentationPointerMap;
    
    /** 
    * \brief Size type of an std::vector 
    */
    typedef RepresentationPointerMap::size_type RepresentationPointerMapSizeType;

    /** 
    * \brief Set the BaseData of the tool specified by the given NavigationData
    * \param data The BaseData to be associated to the NavigationData nd
    * \param nd Specification of the NavigationData tha data will be associated to
    * \return Returns true if the tool has been added, false otherwise.
    */
    bool SetBaseData(const NavigationData* nd, BaseData* data);
    
    /** 
    * \brief Get the BaseData of the tool specified by the given NavigationData
    * \param nd The NavigationData of the BaseData that has to be returned
    * \return Returns the desired BaseData if the NavigationData could be found; Returns Null if no nd was found or if nd is NULL.
    */
    const BaseData* GetBaseData(const NavigationData* nd) const;
  
    /** 
    *\brief Get the number of added BaseData associated to NavigationData 
    * \return Returns the size of the internal map
    */
    RepresentationPointerMapSizeType GetNumberOfToolRepresentations() const
    {return m_RepresentationList.size();}

    /*
    * \brief Transfer the information from the input to the associated BaseData
    */
    virtual void GenerateData();


  protected:
    /**
    * \brief Constructor
    **/
    NavigationDataVisualizationByBaseDataTransformFilter();

    /**
    * \brief Destructor
    **/
    ~NavigationDataVisualizationByBaseDataTransformFilter();

    /**
    * \brief Create a BaseData that represents the given tool
    **/
    void CreateToolRepresenation();

  private:
    /** 
    * \brief An array of the BaseData which represent the tools. 
    */
    RepresentationPointerMap m_RepresentationList;
  };
} // namespace mitk


#endif /* MITKNAVIGATIONDATAVISUALIZATIONBYBASEDATATRANSFORMFILTER_H_HEADER_INCLUDED_ */


