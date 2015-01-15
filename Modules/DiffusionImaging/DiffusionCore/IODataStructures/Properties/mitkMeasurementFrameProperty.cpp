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

#include "mitkMeasurementFrameProperty.h"

mitk::MeasurementFrameProperty::MeasurementFrameProperty()
  : m_MeasurementFrame()
{
}

mitk::MeasurementFrameProperty::MeasurementFrameProperty(const MeasurementFrameProperty& other)
  : mitk::BaseProperty(other)
{
  m_MeasurementFrame = other.GetMeasurementFrame();
}

mitk::MeasurementFrameProperty::MeasurementFrameProperty(const MeasurementFrameType& measurementFrame)
{
  m_MeasurementFrame = measurementFrame;
}

mitk::MeasurementFrameProperty::~MeasurementFrameProperty()
{
}

const mitk::MeasurementFrameProperty::MeasurementFrameType & mitk::MeasurementFrameProperty::GetMeasurementFrame() const
{
  return m_MeasurementFrame;
}

void mitk::MeasurementFrameProperty::SetMeasurementFrame(const MeasurementFrameType & frame)
{
  m_MeasurementFrame = frame;
}

bool mitk::MeasurementFrameProperty::IsEqual(const BaseProperty& property) const
{
  return this->m_MeasurementFrame == static_cast<const Self&>(property).m_MeasurementFrame;
}

bool mitk::MeasurementFrameProperty::Assign(const BaseProperty& property)
{
  this->m_MeasurementFrame = static_cast<const Self&>(property).m_MeasurementFrame;
  return true;
}

itk::LightObject::Pointer  mitk::MeasurementFrameProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
