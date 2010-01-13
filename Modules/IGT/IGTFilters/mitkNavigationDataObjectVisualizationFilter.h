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


#ifndef MITKNAVIGATIONDATAOBJECTVISUALIZATIONFILTER_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATAOBJECTVISUALIZATIONFILTER_H_HEADER_INCLUDED_

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"
#include "mitkBaseData.h"

namespace mitk {

  /**
  * \brief Class that reads NavigationData from input and transfers the information to the geometry of the associated BaseData
  *
  * Derived from NavigationDataToNavigationDataFilter
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataObjectVisualizationFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataObjectVisualizationFilter, NavigationDataToNavigationDataFilter);

    itkNewMacro(Self);

    /** 
    * \brief Smart Pointer type to a BaseData. 
    */
    typedef BaseData::ConstPointer RepresentationPointer;

    /** 
    * \brief STL map of index to BaseData . Using map to be able to set non continuous indices
    */
    typedef std::map<unsigned int, RepresentationPointer> RepresentationPointerMap;
    
    /** 
    * \brief Size type of an std::vector 
    */
    typedef RepresentationPointerMap::size_type RepresentationPointerMapSizeType;

    /** 
    * \brief Set the representation object of the input
    * 
    * \param data The BaseData to be associated to the index 
    * \param index the index with which data will be associated
    */
    void SetBaseData(unsigned int index, BaseData* data);
    
    /** 
    * \brief Get the representation object associated with the index idx
    *
    * \param idx the corresponding input number with which the BaseData is associated
    * \return Returns the desired BaseData if it exists for the given input; Returns NULL 
    *         if no BaseData was found.
    */
    const BaseData* GetBaseData(unsigned int idx);

    virtual void SetTransformPosition(unsigned int index, bool applyTransform); ///< if set to true, the filter will use the position part of the input navigation data at the given index to transform the representation object. If set to false, it will not. If no value is set, it defaults to true.
    virtual bool GetTransformPosition(unsigned int index) const; ///< returns whether position part of the input navigation data at the given index is used for the transformation of the representation object.
    virtual void TransformPositionOn(unsigned int index);  ///< sets the TransformPosition flag to true for the given index
    virtual void TransformPositionOff(unsigned int index); ///< sets the TransformPosition flag to false for the given index

    virtual void SetTransformOrientation(unsigned int index, bool applyTransform); ///< if set to true, the filter will use the orientation part of the input navigation data at the given index to transform the representation object. If set to false, it will not. If no value is set, it defaults to true.
    virtual bool GetTransformOrientation(unsigned int index) const; ///< returns whether orientation part of the input navigation data at the given index is used for the transformation of the representation object.
    virtual void TransformOrientationOn(unsigned int index);  ///< sets the TransformOrientation flag to true for the given index
    virtual void TransformOrientationOff(unsigned int index); ///< sets the TransformOrientation flag to false for the given index

    /** 
    *\brief Get the number of added BaseData associated to NavigationData 
    * \return Returns the size of the internal map
    */
    RepresentationPointerMapSizeType GetNumberOfToolRepresentations() const
    {
      return m_RepresentationList.size();
    }

    /*
    * \brief Transfer the information from the input to the associated BaseData
    */
    virtual void GenerateData();


  protected:
    typedef std::map<itk::ProcessObject::DataObjectPointerArraySizeType, bool> BooleanInputMap;

    /**
    * \brief Constructor
    **/
    NavigationDataObjectVisualizationFilter();

    /**
    * \brief Destructor
    **/
    ~NavigationDataObjectVisualizationFilter();

    /** 
    * \brief An array of the BaseData which represent the tools. 
    */
    RepresentationPointerMap m_RepresentationList;
    BooleanInputMap m_TransformPosition;    ///< if set to true, the filter will use the position part of the input navigation data at the given index for the calculation of the transform. If no entry for the index exists, it defaults to true.
    BooleanInputMap m_TransformOrientation; ///< if set to true, the filter will use the orientation part of the input navigation data at the given index for the calculation of the transform. If no entry for the index exists, it defaults to true.
  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATAOBJECTVISUALIZATIONFILTER_H_HEADER_INCLUDED_ */
