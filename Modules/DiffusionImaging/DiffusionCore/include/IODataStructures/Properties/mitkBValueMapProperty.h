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

#ifndef MITKBVALUEMAPPROPERTY_H
#define MITKBVALUEMAPPROPERTY_H

#include "mitkBaseProperty.h"
#include <map>
#include <vector>

#include <MitkDiffusionCoreExports.h>
#include <itkVectorContainer.h>
#include <vnl/vnl_vector_fixed.h>
namespace mitk
{

  /** This property will store the b value map */

  class MITKDIFFUSIONCORE_EXPORT BValueMapProperty : public mitk::BaseProperty
  {
  public:
    /**
    * \brief The BValueMap contains seperated IndicesVectors for each b value (index for GradientDirectionContainer)
    * key   := b value
    * value := indicesVector
    */
    typedef std::map< unsigned int , std::vector< unsigned int >  >   BValueMap;
    typedef unsigned int                                              IndexType;
    typedef vnl_vector_fixed< double, 3 >                             ValueType;
    typedef ValueType                                                 GradientDirectionType;
    typedef itk::VectorContainer< IndexType, GradientDirectionType >  GradientDirectionsContainerType;
    mitkClassMacro(BValueMapProperty, BaseProperty)

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkNewMacro1Param(BValueMapProperty, const BValueMapProperty&)
    mitkNewMacro1Param(BValueMapProperty, const BValueMap&)

    const BValueMap & GetBValueMap() const;
    void SetBValueMap(const BValueMap & map);

    static BValueMap CreateBValueMap(const GradientDirectionsContainerType * gdc, float referenceBValue);
    static float GetBValueOfGradientDirection(unsigned int i, float referenceBValue, const GradientDirectionsContainerType *gdc);
  protected:

    BValueMapProperty();
    ~BValueMapProperty() override;

    BValueMapProperty(const BValueMapProperty& other);
    BValueMapProperty(const BValueMap& bValueMap);

    bool IsEqual(const BaseProperty& property) const override;
    bool Assign(const BaseProperty & property) override;

    BValueMap m_BValueMap;

    itk::LightObject::Pointer InternalClone() const override;
  };
}
#endif
