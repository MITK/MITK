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

#ifndef MITKMEASUREMENTFRAMEPROPERTY_H
#define MITKMEASUREMENTFRAMEPROPERTY_H

#include "mitkBaseProperty.h"
#include <vnl/vnl_matrix_fixed.h>

#include <MitkDiffusionCoreExports.h>

namespace mitk
{

  /** This property will store the measurement frame */
  class MITKDIFFUSIONCORE_EXPORT MeasurementFrameProperty : public mitk::BaseProperty
  {
  public:
    typedef vnl_matrix_fixed< double, 3, 3 >                            MeasurementFrameType;
    mitkClassMacro(MeasurementFrameProperty, BaseProperty)

      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      mitkNewMacro1Param(MeasurementFrameProperty, const MeasurementFrameProperty&);
    mitkNewMacro1Param(MeasurementFrameProperty, const MeasurementFrameType&);

    const MeasurementFrameType &GetMeasurementFrame() const;
    void SetMeasurementFrame(const MeasurementFrameType & frame);

  protected:

    MeasurementFrameProperty();
    ~MeasurementFrameProperty() override;

    MeasurementFrameProperty(const MeasurementFrameProperty& other);
    MeasurementFrameProperty(const MeasurementFrameType& measurementFrame);

    bool IsEqual(const BaseProperty& property) const override;
    bool Assign(const BaseProperty & property) override;

    MeasurementFrameType m_MeasurementFrame;

    itk::LightObject::Pointer InternalClone() const override;
  };
}
#endif
