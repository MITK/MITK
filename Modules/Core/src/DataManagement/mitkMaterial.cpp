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

#include "mitkMaterial.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include <vtkProperty.h>

mitk::Material::Material(Color color, double opacity)
{
  InitializeStandardValues();
  SetColor(color);
  SetColorCoefficient(GetColorCoefficient());
  SetSpecularColor(GetSpecularColor());
  SetSpecularCoefficient(GetSpecularCoefficient());
  SetSpecularPower(GetSpecularPower());
  SetOpacity(opacity);
  SetInterpolation(GetInterpolation());
  SetRepresentation(GetRepresentation());
  SetLineWidth(GetLineWidth());
  m_Name = "";
}

mitk::Material::Material(double red, double green, double blue, double opacity)
{
  InitializeStandardValues();
  SetColor(red, green, blue);
  SetColorCoefficient(GetColorCoefficient());
  SetSpecularColor(GetSpecularColor());
  SetSpecularCoefficient(GetSpecularCoefficient());
  SetSpecularPower(GetSpecularPower());
  SetOpacity(opacity);
  SetInterpolation(GetInterpolation());
  SetRepresentation(GetRepresentation());
  SetLineWidth(GetLineWidth());
  m_Name = "";
}

mitk::Material::Material(double red,
                         double green,
                         double blue,
                         double colorCoefficient,
                         double specularCoefficient,
                         double specularPower,
                         double opacity)
{
  InitializeStandardValues();
  SetColor(red, green, blue);
  SetColorCoefficient(colorCoefficient);
  SetSpecularColor(GetSpecularColor());
  SetSpecularCoefficient(specularCoefficient);
  SetSpecularPower(specularPower);
  SetOpacity(opacity);
  SetInterpolation(GetInterpolation());
  SetRepresentation(GetRepresentation());
  SetLineWidth(GetLineWidth());
  m_Name = "";
}

mitk::Material::Material(mitk::Material::Color color,
                         double colorCoefficient,
                         double specularCoefficient,
                         double specularPower,
                         double opacity)
{
  InitializeStandardValues();
  SetColor(color);
  SetColorCoefficient(colorCoefficient);
  SetSpecularColor(GetSpecularColor());
  SetSpecularCoefficient(specularCoefficient);
  SetSpecularPower(specularPower);
  SetOpacity(opacity);
  SetInterpolation(GetInterpolation());
  SetRepresentation(GetRepresentation());
  SetLineWidth(GetLineWidth());
}

mitk::Material::Material()
{
  InitializeStandardValues();
  SetColor(GetColor());
  SetColorCoefficient(GetColorCoefficient());
  SetSpecularColor(GetSpecularColor());
  SetSpecularCoefficient(GetSpecularCoefficient());
  SetSpecularPower(GetSpecularPower());
  SetOpacity(GetOpacity());
  SetInterpolation(GetInterpolation());
  SetRepresentation(GetRepresentation());
  SetLineWidth(GetLineWidth());
}

mitk::Material::Material(const Material &property) : itk::Object()
{
  Initialize(property);
}

mitk::Material::Material(
  const Material &property, double red, double green, double blue, double opacity, std::string name)
{
  Initialize(property);
  SetColor(red, green, blue);
  SetOpacity(opacity);
  SetName(name);
}

bool mitk::Material::Assignable(const Material &other) const
{
  try
  {
    const auto &otherinstance =
      dynamic_cast<const Self &>(other); // dear compiler, please don't optimize this away! Thanks.
    otherinstance.GetOpacity();
    return true;
  }
  catch (const std::bad_cast &)
  {
  }
  return false;
}

mitk::Material &mitk::Material::operator=(const mitk::Material &other)
{
  try
  {
    const auto &otherProp(dynamic_cast<const Self &>(other));

    Initialize(otherProp);
  }
  catch (const std::bad_cast &)
  {
    // nothing to do then
  }

  return *this;
}

void mitk::Material::SetColor(mitk::Material::Color color)
{
  m_Color = color;
  Modified();
}

void mitk::Material::SetColor(double red, double green, double blue)
{
  m_Color.Set(red, green, blue);
  Modified();
}

void mitk::Material::SetColorCoefficient(double coefficient)
{
  m_ColorCoefficient = coefficient;
  Modified();
}

void mitk::Material::SetSpecularColor(mitk::Material::Color specularColor)
{
  m_SpecularColor = specularColor;
  Modified();
}

void mitk::Material::SetSpecularColor(double red, double green, double blue)
{
  m_SpecularColor.Set(red, green, blue);
  Modified();
}

void mitk::Material::SetSpecularCoefficient(double specularCoefficient)
{
  m_SpecularCoefficient = specularCoefficient;
  Modified();
}

void mitk::Material::SetSpecularPower(double specularPower)
{
  m_SpecularPower = specularPower;
  Modified();
}

void mitk::Material::SetOpacity(double opacity)
{
  m_Opacity = opacity;
  Modified();
}

void mitk::Material::SetInterpolation(InterpolationType interpolation)
{
  m_Interpolation = interpolation;
  Modified();
}

void mitk::Material::SetRepresentation(RepresentationType representation)
{
  m_Representation = representation;
  Modified();
}

void mitk::Material::SetLineWidth(float lineWidth)
{
  m_LineWidth = lineWidth;
  Modified();
}

mitk::Material::Color mitk::Material::GetColor() const
{
  return m_Color;
}

double mitk::Material::GetColorCoefficient() const
{
  return m_ColorCoefficient;
}

mitk::Material::Color mitk::Material::GetSpecularColor() const
{
  return m_SpecularColor;
}

double mitk::Material::GetSpecularCoefficient() const
{
  return m_SpecularCoefficient;
}

double mitk::Material::GetSpecularPower() const
{
  return m_SpecularPower;
}

double mitk::Material::GetOpacity() const
{
  return m_Opacity;
}

mitk::Material::InterpolationType mitk::Material::GetInterpolation() const
{
  return m_Interpolation;
}

mitk::Material::RepresentationType mitk::Material::GetRepresentation() const
{
  return m_Representation;
}

int mitk::Material::GetVtkInterpolation() const
{
  switch (m_Interpolation)
  {
    case (Flat):
      return VTK_FLAT;
    case (Gouraud):
      return VTK_GOURAUD;
    case (Phong):
      return VTK_PHONG;
  }
  return VTK_GOURAUD;
}

int mitk::Material::GetVtkRepresentation() const
{
  switch (m_Representation)
  {
    case (Points):
      return VTK_POINTS;
    case (Wireframe):
      return VTK_WIREFRAME;
    case (Surface):
      return VTK_SURFACE;
  }
  return VTK_SURFACE;
}

float mitk::Material::GetLineWidth() const
{
  return m_LineWidth;
}

void mitk::Material::Initialize(const Material &property)
{
  this->SetColor(property.GetColor());
  this->SetColorCoefficient(property.GetColorCoefficient());
  this->SetSpecularColor(property.GetSpecularColor());
  this->SetSpecularCoefficient(property.GetSpecularCoefficient());
  this->SetSpecularPower(property.GetSpecularPower());
  this->SetOpacity(property.GetOpacity());
  this->SetInterpolation(property.GetInterpolation());
  this->SetRepresentation(property.GetRepresentation());
  this->SetLineWidth(property.GetLineWidth());
  this->SetName(property.GetName());
}

bool mitk::Material::operator==(const Material &property) const
{
  const auto *other = dynamic_cast<const Self *>(&property);

  if (other == nullptr)
    return false;
  else
    return (m_Color == other->GetColor() && m_ColorCoefficient == other->GetColorCoefficient() &&
            m_SpecularColor == other->GetSpecularColor() && m_SpecularCoefficient == other->GetSpecularCoefficient() &&
            m_SpecularPower == other->GetSpecularPower() && m_Opacity == other->GetOpacity() &&
            m_Interpolation == other->GetInterpolation() && m_Name == other->GetName() &&
            m_Representation == other->GetRepresentation() && m_LineWidth == other->GetLineWidth());
}

void mitk::Material::InitializeStandardValues()
{
  m_Color.Set(0.5, 0.5, 0.0);
  m_ColorCoefficient = 0.5;
  m_SpecularColor.Set(1.0, 1.0, 1.0);
  m_SpecularCoefficient = 0.5;
  m_SpecularPower = 10.0;
  m_Opacity = 1.0;
  m_Interpolation = Gouraud;
  m_Representation = Surface;
  m_LineWidth = 1.0;
  m_Name = "";
}

void mitk::Material::Update()
{
  this->SetColor(this->GetColor());
  this->SetColorCoefficient(this->GetColorCoefficient());
  this->SetSpecularColor(this->GetSpecularColor());
  this->SetSpecularCoefficient(this->GetSpecularCoefficient());
  this->SetSpecularPower(this->GetSpecularPower());
  this->SetOpacity(this->GetOpacity());
  this->SetInterpolation(this->GetInterpolation());
  this->SetRepresentation(this->GetRepresentation());
}

void mitk::Material::PrintSelf(std::ostream &os, itk::Indent /* unused */) const
{
  os << "Name: " << GetName() << std::endl;
  os << "Color: " << GetColor() << std::endl;
  os << "ColorCoefficient" << GetColorCoefficient() << std::endl;
  os << "SpecularColor: " << GetSpecularColor() << std::endl;
  os << "SpecularCoefficient: " << GetSpecularCoefficient() << std::endl;
  os << "SpecularPower: " << GetSpecularPower() << std::endl;
  os << "Opacity: " << GetOpacity() << std::endl;
  os << "Line width: " << GetLineWidth() << std::endl;
  switch (GetInterpolation())
  {
    case (Flat):
      os << "Interpolation: Flat" << std::endl;
      break;
    case (Gouraud):
      os << "Interpolation: Gouraud" << std::endl;
      break;
    case (Phong):
      os << "Interpolation: Phong" << std::endl;
      break;
  }
  switch (GetRepresentation())
  {
    case (Points):
      os << "Representation: Points" << std::endl;
      break;
    case (Wireframe):
      os << "Representation: Wireframe" << std::endl;
      break;
    case (Surface):
      os << "Representation: Surface" << std::endl;
      break;
  }
}
