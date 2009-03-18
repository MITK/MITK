/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>

#include <itkQuaternionRigidTransform.h>
#include <itkTransform.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataTransformFilter applies an user-defined rigid transformation on navigation data objects. 
  * Input navigation data are mapped 1:1 on output navigation data.  To run the filter SetRigid3DTransform(TransformType::Pointer transform) has to be called first.
  *
  * \ingroup Navigation
  */
  class NavigationDataTransformFilter : public NavigationDataToNavigationDataFilter
  {

  public:

    typedef itk::Rigid3DTransform< float > TransformType;

    mitkClassMacro(NavigationDataTransformFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    /**Documentation 
    *\brief Set the rigid transform used to transform the input navigation data.
    *
    */
    void SetRigid3DTransform(TransformType::Pointer transform);


  protected:

    NavigationDataTransformFilter();
    virtual ~NavigationDataTransformFilter();

    itk::QuaternionRigidTransform<double>::Pointer m_QuatOrgRigidTransform; ///< transform needed to rotate orientation
    itk::QuaternionRigidTransform<double>::Pointer m_QuatTmpTransform;      ///< further transform needed to rotate orientation


    /**Documentation
    * \brief filter execute method
    *
    * transforms navigation data   
    */
    virtual void GenerateData();

    TransformType::Pointer m_Transform; ///< transform which will be applied on navigation data(s)
  };
} // namespace mitk

#endif /* MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_ */
