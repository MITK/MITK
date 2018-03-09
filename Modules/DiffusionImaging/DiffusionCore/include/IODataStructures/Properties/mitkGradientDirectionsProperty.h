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

#ifndef MITKGRADIENTDIRECTIONSPROPERTY_H
#define MITKGRADIENTDIRECTIONSPROPERTY_H

#include "mitkBaseProperty.h"
#include <vnl/vnl_vector_fixed.h>
#include <itkVectorContainer.h>
#include <itkVector.h>

#include <MitkDiffusionCoreExports.h>

namespace mitk
{

  /** This property will store the gradients directions and the original gradient directions */
  class MITKDIFFUSIONCORE_EXPORT GradientDirectionsProperty : public mitk::BaseProperty
  {
  public:
    typedef unsigned int                                             IndexType;
    typedef vnl_vector_fixed< double, 3 >                            ValueType;
    typedef ValueType                                                GradientDirectionType;
    typedef itk::VectorContainer< IndexType, GradientDirectionType > GradientDirectionsContainerType;
    typedef std::vector< itk::Vector<double,3> >                     AlternativeGradientDirectionsContainerType;

    mitkClassMacro(GradientDirectionsProperty, BaseProperty)

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkNewMacro1Param(GradientDirectionsProperty, const GradientDirectionsProperty&);
    mitkNewMacro1Param(GradientDirectionsProperty, const GradientDirectionsContainerType::Pointer);
    mitkNewMacro1Param(GradientDirectionsProperty, const AlternativeGradientDirectionsContainerType );

    const GradientDirectionsContainerType::Pointer GetGradientDirectionsContainer() const;

  protected:

    GradientDirectionsProperty();
    ~GradientDirectionsProperty() override;

    GradientDirectionsProperty(const GradientDirectionsProperty& other);
    GradientDirectionsProperty(const GradientDirectionsContainerType::Pointer gradientDirectionsContainer);
    GradientDirectionsProperty(const AlternativeGradientDirectionsContainerType gradientDirectionsContainer);

    bool IsEqual(const BaseProperty& property) const override;
    bool Assign(const BaseProperty & property) override;

    GradientDirectionsContainerType::Pointer m_GradientDirectionsContainer;

    itk::LightObject::Pointer InternalClone() const override;
  };
}
#endif
