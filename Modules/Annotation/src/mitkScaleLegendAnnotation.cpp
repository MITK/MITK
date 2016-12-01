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

#include "mitkScaleLegendAnnotation.h"
#include <vtkAxisActor2D.h>
#include <vtkLegendScaleActor.h>
#include <vtkTextProperty.h>

mitk::ScaleLegendAnnotation::ScaleLegendAnnotation()
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

mitk::ScaleLegendAnnotation::~ScaleLegendAnnotation()
{
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::ScaleLegendAnnotation::LocalStorage::~LocalStorage()
{
}

mitk::ScaleLegendAnnotation::LocalStorage::LocalStorage()
{
  m_legendScaleActor = vtkSmartPointer<vtkLegendScaleActor>::New();
}

void mitk::ScaleLegendAnnotation::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
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

vtkProp *mitk::ScaleLegendAnnotation::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_legendScaleActor;
}

void mitk::ScaleLegendAnnotation::SetRightAxisVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendAnnotation.RightAxisVisibility", visibility);
}

bool mitk::ScaleLegendAnnotation::GetRightAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendAnnotation.RightAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendAnnotation::SetLeftAxisVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendAnnotation.LeftAxisVisibility", visibility);
}

bool mitk::ScaleLegendAnnotation::GetLeftAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendAnnotation.LeftAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendAnnotation::SetTopAxisVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendAnnotation.TopAxisVisibility", visibility);
}

bool mitk::ScaleLegendAnnotation::GetTopAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendAnnotation.TopAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendAnnotation::SetBottomAxisVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendAnnotation.BottomAxisVisibility", visibility);
}

bool mitk::ScaleLegendAnnotation::GetBottomAxisVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendAnnotation.BottomAxisVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendAnnotation::SetLegendVisibility(bool visibility)
{
  SetBoolProperty("ScaleLegendAnnotation.SetLegendVisibility", visibility);
}

bool mitk::ScaleLegendAnnotation::GetLegendVisibility() const
{
  bool visibility;
  GetPropertyList()->GetBoolProperty("ScaleLegendAnnotation.SetLegendVisibility", visibility);
  return visibility;
}

void mitk::ScaleLegendAnnotation::SetRightBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendAnnotation.RightBorderOffset", offset);
}

int mitk::ScaleLegendAnnotation::GetRightBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendAnnotation.RightBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendAnnotation::SetLeftBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendAnnotation.LeftBorderOffset", offset);
}

int mitk::ScaleLegendAnnotation::GetLeftBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendAnnotation.LeftBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendAnnotation::SetTopBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendAnnotation.TopBorderOffset", offset);
}

int mitk::ScaleLegendAnnotation::GetTopBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendAnnotation.TopBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendAnnotation::SetBottomBorderOffset(int offset)
{
  SetIntProperty("ScaleLegendAnnotation.BottomBorderOffset", offset);
}

int mitk::ScaleLegendAnnotation::GetBottomBorderOffset() const
{
  int offset;
  GetPropertyList()->GetIntProperty("ScaleLegendAnnotation.BottomBorderOffset", offset);
  return offset;
}

void mitk::ScaleLegendAnnotation::SetCornerOffsetFactor(double offsetFactor)
{
  SetDoubleProperty("ScaleLegendAnnotation.CornerOffsetFactor", offsetFactor);
}

double mitk::ScaleLegendAnnotation::GetCornerOffsetFactor() const
{
  double offsetFactor;
  GetPropertyList()->GetDoubleProperty("ScaleLegendAnnotation.CornerOffsetFactor", offsetFactor);
  return offsetFactor;
}

void mitk::ScaleLegendAnnotation::SetFontFactor(double fontFactor)
{
  SetDoubleProperty("ScaleLegendAnnotation.FontFactor", fontFactor);
}

double mitk::ScaleLegendAnnotation::GetFontFactor() const
{
  double offsetFactor;
  GetPropertyList()->GetDoubleProperty("ScaleLegendAnnotation.FontFactor", offsetFactor);
  return offsetFactor;
}
