/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>

#include <itkVersorRigid3DTransform.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataTransformFilter applies an user-defined rigid transformation on navigation data objects.
  * Input navigation data are mapped 1:1 on output navigation data.  To run the filter SetRigid3DTransform(TransformType::Pointer transform) has to be called first.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationDataTransformFilter : public NavigationDataToNavigationDataFilter
  {

  public:

    // The epsilon ITK uses to check the orthogonality of rotation matrices
    // is too small for float precision so we must use double precision to
    // compose the transforms and convert back to float (mitk::ScalarType) at
    // the end
    typedef itk::VersorRigid3DTransform< double > TransformType;

    mitkClassMacro(NavigationDataTransformFilter, NavigationDataToNavigationDataFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**Documentation
    * \brief Set the rigid transform used to transform the input navigation data.
    */
    itkSetObjectMacro(Rigid3DTransform, TransformType);
    itkGetConstObjectMacro(Rigid3DTransform, TransformType);

    /**Documentation
    * \brief Set transform composition order
    *
    * If precompose is true, then transform is precomposed with the input
    * NavigationData transform; that is, the resulting transformation consists
    * of first applying transform, then applying the input NavigationData
    * transformation.
    *
    * If precompose is false or omitted, then transform is post-composed with
    * the input NavigationData transform; that is the resulting transformation
    * consists of first applying the NavigationData transformation, followed by
    * transform.
    */
    itkSetMacro(Precompose, bool);
    itkGetMacro(Precompose, bool);
    itkBooleanMacro(Precompose);

  protected:

    NavigationDataTransformFilter();
    ~NavigationDataTransformFilter() override;

    /**Documentation
    * \brief filter execute method
    *
    * transforms navigation data
    */
    void GenerateData() override;

    TransformType::Pointer m_Rigid3DTransform; ///< transform which will be applied on navigation data(s)
    bool m_Precompose;
  };
} // namespace mitk

#endif /* MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_ */
