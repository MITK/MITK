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
#include <vtkAxisActor2D.h>
#include <vtkLegendScaleActor.h>
#include <vtkTextProperty.h>

mitk::ScaleLegendOverlay::ScaleLegendOverlay()
{
  SetRightAxisVisibility(true);
  SetLeftAxisVisibility(true);
  SetTopAxisVisibility(true);
  SetBottomAxisVisibility(true);
  SetLegendVisibility(true);
  SetRightBorderOffset(50);
  SetLeftBorderOffset(50);
  SetTopBorderOffset(30);
  SetBottomBorderOffset(30);
  SetCornerOffsetFactor(2.0);
  SetFontFactor(1.0);
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
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  if (ls->IsGenerateDataRequired(renderer, this))
  {
    ls->m_legendScaleActor->SetRightAxisVisibility(this->GetRightAxisVisibility());
    ls->m_legendScaleActor->SetTopAxisVisibility(this->GetTopAxisVisibility());
    ls->m_legendScaleActor->SetLeftAxisVisibility(this->GetLeftAxisVisibility());
    ls->m_legendScaleActor->SetBottomAxisVisibility(this->GetBottomAxisVisibility());
    ls->m_legendScaleActor->SetLegendVisibility(this->GetLegendVisibility());
    ls->m_legendScaleActor->SetRightBorderOffset(this->GetRightBorderOffset());
    ls->m_legendScaleActor->SetLeftBorderOffset(this->GetLeftBorderOffset());
    ls->m_legendScaleActor->SetTopBorderOffset(this->GetTopBorderOffset());
    ls->m_legendScaleActor->SetBottomBorderOffset(this->GetBottomBorderOffset());
    ls->m_legendScaleActor->SetCornerOffsetFactor(this->GetCornerOffsetFactor());
    ls->m_legendScaleActor->GetLeftAxis()->SetFontFactor(this->GetFontFactor());
    ls->m_legendScaleActor->GetRightAxis()->SetFontFactor(this->GetFontFactor());
    ls->m_legendScaleActor->GetTopAxis()->SetFontFactor(this->GetFontFactor());
    ls->m_legendScaleActor->GetBottomAxis()->SetFontFactor(this->GetFontFactor());
  }
}

vtkProp *mitk::ScaleLegendOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
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

void mitk::ScaleLegendOverlay::SetLeftBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendOverlay.LeftBorderOffset", offset);
}

int mitk::ScaleLegendOverlay::GetLeftBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendOverlay.LeftBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendOverlay::SetTopBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendOverlay.TopBorderOffset", offset);
}

int mitk::ScaleLegendOverlay::GetTopBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendOverlay.TopBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendOverlay::SetBottomBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendOverlay.BottomBorderOffset", offset);
}

int mitk::ScaleLegendOverlay::GetBottomBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendOverlay.BottomBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendOverlay::SetCornerOffsetFactor(double offsetFactor)
{
  SetDoubleProperty("ScaleLegendOverlay.CornerOffsetFactor", offsetFactor);
}

double mitk::ScaleLegendOverlay::GetCornerOffsetFactor() const
{
  double offsetFactor;
  GetPropertyList()->GetDoubleProperty("ScaleLegendOverlay.CornerOffsetFactor", offsetFactor);
  return offsetFactor;
}

void mitk::ScaleLegendOverlay::SetFontFactor(double fontFactor)
{
  SetDoubleProperty("ScaleLegendOverlay.FontFactor", fontFactor);
}

double mitk::ScaleLegendOverlay::GetFontFactor() const
{
  double offsetFactor;
  GetPropertyList()->GetDoubleProperty("ScaleLegendOverlay.FontFactor", offsetFactor);
  return offsetFactor;
}
