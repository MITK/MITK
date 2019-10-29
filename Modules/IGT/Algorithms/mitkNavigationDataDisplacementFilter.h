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
  class MITKIGT_EXPORT NavigationDataDisplacementFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataDisplacementFilter, NavigationDataToNavigationDataFilter);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkSetVectorMacro(Offset, mitk::Vector3D); ///< Get Offset parameter
    mitkGetVectorMacro(Offset, mitk::Vector3D); ///< Set Offset parameter
    itkSetMacro(Transform6DOF, bool);
    itkGetMacro(Transform6DOF, bool);

    void SetTransformation( mitk::AffineTransform3D::Pointer transform );

    /**
    *\brief Set all filter parameters (Offset) as the PropertyList p
    *
    * This method reads the following properties from the PropertyList (name : data type):
    *  - "NavigationDataDisplacementFilter_Offset" : mitk::Vector3DProperty
    */
    void SetParameters(const mitk::PropertyList* p) override;

    /**
    *\brief Get all filter parameters (offset) as a PropertyList
    *
    * This method returns a PropertyList containing the following
    * properties (name : data type):
    *  - "NavigationDataDisplacementFilter_Offset" : mitk::Vector3DProperty
    * The returned PropertyList must be assigned to a
    * SmartPointer immediately, or else it will get destroyed.
    */
    mitk::PropertyList::ConstPointer GetParameters() const override;

  protected:
    NavigationDataDisplacementFilter();
    ~NavigationDataDisplacementFilter() override;

    /**Documentation
    * \brief filter execute method
    *
    * adds the offset m_Offset to all inputs
    */
    void GenerateData() override;

    mitk::Vector3D m_Offset; ///< offset that is added to all inputs

    bool m_Transform6DOF;

    mitk::NavigationData::Pointer m_Transformation;

  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATATONAVIGATIONDATAFILTER_H_HEADER_INCLUDED_ */
