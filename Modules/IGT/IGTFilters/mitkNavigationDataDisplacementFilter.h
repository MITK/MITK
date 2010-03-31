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


#ifndef MITKNavigationDataDisplacementFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataDisplacementFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>

namespace mitk 
{
  /**Documentation
  * \brief NavigationDataDisplacementFilter adds an offset to navigation data objects
  *
  * Example class that shows how to write a navigation filter. This filter 
  * adds a offset that can be specified as a parameter to each input navigation data.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataDisplacementFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataDisplacementFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    mitkSetVectorMacro(Offset, mitk::Vector3D); ///< Get Offset parameter
    mitkGetVectorMacro(Offset, mitk::Vector3D); ///< Set Offset parameter

    /** 
    *\brief Set all filter parameters (Offset) as the PropertyList p
    *
    * This method reads the following properties from the PropertyList (name : data type):
    *  - "NavigationDataDisplacementFilter_Offset" : mitk::Vector3DProperty
    */
    virtual void SetParameters(const mitk::PropertyList* p);

    /** 
    *\brief Get all filter parameters (offset) as a PropertyList
    *
    * This method returns a PropertyList containing the following
    * properties (name : data type):
    *  - "NavigationDataDisplacementFilter_Offset" : mitk::Vector3DProperty
    * The returned PropertyList must be assigned to a 
    * SmartPointer immediately, or else it will get destroyed.
    */
    mitk::PropertyList::ConstPointer GetParameters() const;

  protected:
    NavigationDataDisplacementFilter();
    virtual ~NavigationDataDisplacementFilter();

    /**Documentation
    * \brief filter execute method
    *
    * adds the offset m_Offset to all inputs  
    */
    virtual void GenerateData();

    mitk::Vector3D m_Offset; ///< offset that is added to all inputs
  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATATONAVIGATIONDATAFILTER_H_HEADER_INCLUDED_ */
