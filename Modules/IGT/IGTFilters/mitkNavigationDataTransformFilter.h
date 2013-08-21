/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>

#include <itkQuaternionRigidTransform.h>
#include <itkTransform.h>
#include <itkVersorRigid3DTransform.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataTransformFilter applies an user-defined rigid transformation on navigation data objects.
  * Input navigation data are mapped 1:1 on output navigation data.  To run the filter SetRigid3DTransform(TransformType::Pointer transform) has to be called first.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataTransformFilter : public NavigationDataToNavigationDataFilter
  {

  public:

    typedef itk::VersorRigid3DTransform< double > TransformType;

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
