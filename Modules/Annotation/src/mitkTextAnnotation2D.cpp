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

#include "mitkTextAnnotation2D.h"
#include "vtkUnicodeString.h"
#include <vtkPropAssembly.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

mitk::TextAnnotation2D::TextAnnotation2D()
{
  mitk::Point2D position;
  position[0] = position[1] = 0;
  this->SetPosition2D(position);
  this->SetOffsetVector(position);
  this->SetText("");
  this->SetFontSize(20);
  this->SetColor(1.0, 1.0, 1.0);
  this->SetStringProperty("font.family", "Arial");
  this->SetBoolProperty("font.bold", false);
  this->SetBoolProperty("font.italic", false);
  this->SetBoolProperty("drawShadow", false);
}

mitk::TextAnnotation2D::~TextAnnotation2D()
{
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::Annotation::Bounds mitk::TextAnnotation2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  mitk::Annotation::Bounds bounds;
  bounds.Position = ls->m_TextActor->GetPosition();

  double size[2];
  ls->m_TextActor->GetSize(renderer->GetVtkRenderer(), size);
  bounds.Size[0] = size[0];
  bounds.Size[1] = size[1];
  return bounds;
}

void mitk::TextAnnotation2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Annotation::Bounds &bounds)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  mitk::Point2D posT, posS;
  posT[0] = bounds.Position[0];
  posT[1] = bounds.Position[1];
  posS[0] = posT[0] + 1;
  posS[1] = posT[1] - 1;

  ls->m_TextActor->SetDisplayPosition(posT[0], posT[1]);
  ls->m_STextActor->SetDisplayPosition(posS[0], posS[1]);
}

mitk::TextAnnotation2D::LocalStorage::~LocalStorage()
{
}

mitk::TextAnnotation2D::LocalStorage::LocalStorage()
{
  m_TextActor = vtkSmartPointer<vtkTextActor>::New();
  m_TextProp = vtkSmartPointer<vtkTextProperty>::New();
  m_STextActor = vtkSmartPointer<vtkTextActor>::New();
  m_STextProp = vtkSmartPointer<vtkTextProperty>::New();
  m_TextActor->SetTextProperty(m_TextProp);
  m_STextActor->SetTextProperty(m_STextProp);
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
  m_Assembly->AddPart(m_STextActor);
  m_Assembly->AddPart(m_TextActor);
}

void mitk::TextAnnotation2D::UpdateVtkAnnotation2D(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);

  if (ls->IsGenerateDataRequired(renderer, this))
  {
    float color[3] = {0.0, 1.0, 0.0};
    float opacity = 1.0;
    GetColor(color);
    GetOpacity(opacity);
    ls->m_TextProp->SetColor(color[0], color[1], color[2]);
    ls->m_STextProp->SetColor(0, 0, 0);
    ls->m_TextProp->SetFontSize(GetFontSize());
    ls->m_TextProp->SetOpacity(opacity);
    ls->m_STextProp->SetFontSize(GetFontSize());
    ls->m_STextProp->SetOpacity(opacity);

    std::string fontFamilyAsString;
    if (GetStringProperty("font.family", fontFamilyAsString) == false)
    {
      fontFamilyAsString = "Arial";
    }
    ls->m_TextProp->SetFontFamilyAsString(fontFamilyAsString.c_str());
    ls->m_STextProp->SetFontFamilyAsString(fontFamilyAsString.c_str());

    bool boldFont(false);
    GetBoolProperty("font.bold", boldFont);
    ls->m_TextProp->SetBold(boldFont);
    ls->m_STextProp->SetBold(boldFont);

    bool italicFont(false);
    GetBoolProperty("font.italic", italicFont);
    ls->m_TextProp->SetBold(italicFont);
    ls->m_STextProp->SetBold(italicFont);

    bool drawShadow;
    GetBoolProperty("drawShadow", drawShadow);
    ls->m_TextProp->SetShadow(false);
    ls->m_STextProp->SetShadow(false);
    ls->m_STextActor->SetVisibility(drawShadow);

    ls->m_TextActor->SetInput(GetText().c_str());
    ls->m_STextActor->SetInput(GetText().c_str());

    mitk::Point2D posT, posS;
    posT[0] = GetPosition2D()[0] + GetOffsetVector()[0];
    posT[1] = GetPosition2D()[1] + GetOffsetVector()[1];
    posS[0] = posT[0] + 1;
    posS[1] = posT[1] - 1;

    ls->m_TextActor->SetDisplayPosition(posT[0], posT[1]);
    ls->m_STextActor->SetDisplayPosition(posS[0], posS[1]);
    ls->UpdateGenerateDataTime();
  }
}

vtkProp *mitk::TextAnnotation2D::GetVtkProp(mitk::BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_Assembly;
}

vtkActor2D *mitk::TextAnnotation2D::GetVtkActor2D(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_TextActor;
}
