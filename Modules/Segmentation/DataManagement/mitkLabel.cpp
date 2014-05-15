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


#include "mitkLabel.h"

#include "itkProcessObject.h"


mitk::Label::Label():
m_Locked(true),
m_Visible(true),
m_Filled(true),
m_Exterior(false),
m_Opacity(0.6),
m_Volume(0.0),
m_PixelValue(-1),
m_Layer(0)
{

}

mitk::Label::Label(const Label& other)
  : itk::Object()
{
  this->m_Locked = other.GetLocked();
  this->m_Visible = other.GetVisible();
  this->m_Filled = other.GetFilled();
  this->m_Exterior = other.GetExterior();
  this->m_Opacity = other.GetOpacity();
  this->m_Selected = other.GetSelected();
  this->m_Name = other.GetName();
  this->m_LatinName = other.GetLatinName();
  this->m_Volume = other.GetVolume();
  this->m_LastModified = other.GetLastModified();
  this->m_PixelValue = other.GetPixelValue();
  this->m_Layer = other.GetLayer();
  this->m_Color = other.GetColor();
}

mitk::Label::~Label()
{

}

void mitk::Label::SetColor(const mitk::Color &_color)
{
  this->m_Color = _color;
}

void mitk::Label::PrintSelf(std::ostream &os, itk::Indent indent) const
{
 // todo
}

void mitk::Label::SetCenterOfMassIndex(const mitk::Point3D& center)
{
  this->m_CenterOfMassIndex = center;
}

const mitk::Point3D& mitk::Label::GetCenterOfMassIndex()
{
  return this->m_CenterOfMassIndex;
}

void mitk::Label::SetCenterOfMassCoordinates(const mitk::Point3D& center)
{
  this->m_CenterOfMassCoordinates = center;
}

const mitk::Point3D& mitk::Label::GetCenterOfMassCoordinates()
{
  return this->m_CenterOfMassCoordinates;
}
