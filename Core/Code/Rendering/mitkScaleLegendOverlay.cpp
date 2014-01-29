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

#include "mitkScaleLegendOverlay.h"
#include <vtkLegendScaleActor.h>

mitk::ScaleLegendOverlay::ScaleLegendOverlay()
{
  SetRightAxisVisibility(true);

  SetLeftAxisVisibility(false);

  SetTopAxisVisibility(false);

  SetBottomAxisVisibility(false);

  SetLegendVisibility(false);

  SetRightBorderOffset(30);

  SetCornerOffsetFactor(2.0);
}


mitk::ScaleLegendOverlay::~ScaleLegendOverlay()
{
}

mitk::ScaleLegendOverlay::LocalStorage::~LocalStorage()
{
}

mitk::ScaleLegendOverlay::LocalStorage::LocalStorage()
{
  m_legendScaleActor = vtkSmartPointer<vtkLegendScaleActor>::New();
}

void mitk::ScaleLegendOverlay::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  if(ls->IsGenerateDataRequired(renderer,this))
  {
    ls->m_legendScaleActor->SetRightAxisVisibility(this->GetRightAxisVisibility());
    ls->m_legendScaleActor->SetTopAxisVisibility(this->GetTopAxisVisibility());
    ls->m_legendScaleActor->SetLeftAxisVisibility(this->GetLeftAxisVisibility());
    ls->m_legendScaleActor->SetBottomAxisVisibility(this->GetBottomAxisVisibility());
    ls->m_legendScaleActor->SetLegendVisibility(this->GetLegendVisibility());
    ls->m_legendScaleActor->SetRightBorderOffset(this->GetRightBorderOffset());
    ls->m_legendScaleActor->SetCornerOffsetFactor (this->GetCornerOffsetFactor());
  }

}

vtkProp* mitk::ScaleLegendOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_legendScaleActor;
}

void mitk::ScaleLegendOverlay::SetRightAxisVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendOverlay.RightAxisVisibility", visibility);
}

bool mitk::ScaleLegendOverlay::GetRightAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendOverlay.RightAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendOverlay::SetLeftAxisVisibility(bool visibility)
{
    SetBoolProperty("ScaleLegendOverlay.LeftAxisVisibility", visibility);
}

bool mitk::ScaleLegendOverlay::GetLeftAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendOverlay.LeftAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendOverlay::SetTopAxisVisibility(bool visibility)
{
    SetBoolProperty("ScaleLegendOverlay.TopAxisVisibility", visibility);
}

bool mitk::ScaleLegendOverlay::GetTopAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendOverlay.TopAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendOverlay::SetBottomAxisVisibility(bool visibility)
{
    SetBoolProperty("ScaleLegendOverlay.BottomAxisVisibility", visibility);
}

bool mitk::ScaleLegendOverlay::GetBottomAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendOverlay.BottomAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendOverlay::SetLegendVisibility(bool visibility)
{
    SetBoolProperty("ScaleLegendOverlay.SetLegendVisibility", visibility);
}

bool mitk::ScaleLegendOverlay::GetLegendVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendOverlay.SetLegendVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendOverlay::SetRightBorderOffset(int offset)
{
    SetIntProperty("ScaleLegendOverlay.RightBorderOffset", offset);
}

int mitk::ScaleLegendOverlay::GetRightBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendOverlay.RightBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendOverlay::SetCornerOffsetFactor(float offsetFactor)
{
  SetFloatProperty("ScaleLegendOverlay.CornerOffsetFactor", offsetFactor);
}

float mitk::ScaleLegendOverlay::GetCornerOffsetFactor() const
{
  float offsetFactor;
  GetPropertyList()->GetFloatProperty("ScaleLegendOverlay.CornerOffsetFactor", offsetFactor);
  return offsetFactor;
}
