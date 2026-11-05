/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureVisibilityProperty.h>

mitk::PlanarFigureVisibilityProperty::PlanarFigureVisibilityProperty()
  : PlanarFigureVisibilityProperty(Visibility::Always)
{
}

mitk::PlanarFigureVisibilityProperty::PlanarFigureVisibilityProperty(Visibility visibility)
{
  this->AddEnum("Always", static_cast<IdType>(Visibility::Always));
  this->AddEnum("On hover", static_cast<IdType>(Visibility::OnHover));
  this->AddEnum("When selected", static_cast<IdType>(Visibility::WhenSelected));
  this->AddEnum("On hover or when selected", static_cast<IdType>(Visibility::OnHoverOrWhenSelected));
  this->AddEnum("On hover while selected", static_cast<IdType>(Visibility::OnHoverWhileSelected));
  this->AddEnum("Never", static_cast<IdType>(Visibility::Never));
  this->SetVisibility(visibility);
}

mitk::PlanarFigureVisibilityProperty::Visibility mitk::PlanarFigureVisibilityProperty::GetVisibility() const
{
  return static_cast<Visibility>(this->GetValueAsId());
}

void mitk::PlanarFigureVisibilityProperty::SetVisibility(Visibility visibility)
{
  this->SetValue(static_cast<IdType>(visibility));
}

bool mitk::PlanarFigureVisibilityProperty::AddEnum(const std::string &name, const IdType &id)
{
  return Superclass::AddEnum(name, id);
}
