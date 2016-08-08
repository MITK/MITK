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

#include "mitkTextOverlay2D.h"
#include <vtkTextProperty.h>
#include "vtkUnicodeString.h"
#include <vtkTextActor.h>
#include <vtkPropAssembly.h>

mitk::TextOverlay2D::TextOverlay2D()
{
  mitk::Point2D position;
  position[0] = position[1] = 0;
  SetPosition2D(position);
  SetOffsetVector(position);

  m_Orientation = TextOrientation::TextRigth;
}

mitk::TextOverlay2D::~TextOverlay2D()
{
}

void mitk::TextOverlay2D::SetOrientation(const TextOrientation& orientation)
{
  m_Orientation = orientation;
}

mitk::Overlay::Bounds mitk::TextOverlay2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  mitk::Overlay::Bounds bounds;
  bounds.Position = ls->m_TextActor->GetPosition();

  double size[2];
  ls->m_TextActor->GetSize(renderer->GetVtkRenderer(), size);
  bounds.Size[0] = size[0];
  bounds.Size[1] = size[1];
  return bounds;
}

void mitk::TextOverlay2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Overlay::Bounds& bounds)
{
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  actor->SetDisplayPosition(bounds.Position[0], bounds.Position[1]);
}

mitk::TextOverlay2D::LocalStorage::~LocalStorage()
{
}

namespace
{
  const int FONT_SIZE = 16;
}

mitk::TextOverlay2D::LocalStorage::LocalStorage()
{
  m_TextActor = vtkSmartPointer<vtkTextActor>::New();
  m_TextProp = vtkSmartPointer<vtkTextProperty>::New();
  m_STextActor = vtkSmartPointer<vtkTextActor>::New();
  m_STextProp = vtkSmartPointer<vtkTextProperty>::New();

  std::string m_programPath;

#ifndef _WIN32
#include <unistd.h>
#include <limits.h>

  char buff[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
  if (len != -1) {
    buff[len] = '\0';
    m_programPath = std::string(buff);
  }
#else

  m_programPath.resize(1024);
  DWORD pathSize = GetModuleFileNameA(nullptr, &m_programPath[0], m_programPath.size());
  m_programPath.resize(pathSize);

  std::string::size_type pos = m_programPath.rfind("\\");
  if (pos != std::string::npos) {
    m_programPath.erase(pos + 1, m_programPath.size());
  }

#endif

  std::string fontPath = m_programPath + std::string("Fonts\\DejaVuSans.ttf");

  m_TextProp->SetFontFamily(VTK_FONT_FILE);
  m_TextProp->SetFontFile(fontPath.c_str());
  
  m_TextProp->SetFontSize(FONT_SIZE);
  m_STextProp->SetFontSize(FONT_SIZE);

  m_TextActor->SetTextProperty(m_TextProp);
  m_STextActor->SetTextProperty(m_STextProp);
  m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
  m_Assembly->AddPart(m_STextActor);
  m_Assembly->AddPart(m_TextActor);
}

void mitk::TextOverlay2D::UpdateVtkOverlay2D(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  if (ls->IsGenerateDataRequired(renderer, this))
  {
    float color[3] = { 0.0, 1.0, 0.0 };
    float opacity = 1.0;
    GetColor(color, renderer);
    GetOpacity(opacity, renderer);
    ls->m_TextProp->SetColor(color[0], color[1], color[2]);
    ls->m_STextProp->SetColor(0, 0, 0);
    ls->m_TextProp->SetOpacity(opacity);
    ls->m_STextProp->SetOpacity(opacity);

    std::string fontFamilyAsString;
    if ( GetStringProperty( "font.family", fontFamilyAsString ) == false )
    {
      fontFamilyAsString = "Arial";
    }
    ls->m_TextProp->SetFontFamilyAsString( fontFamilyAsString.c_str() );
    ls->m_STextProp->SetFontFamilyAsString( fontFamilyAsString.c_str() );

    bool boldFont(false);
    GetBoolProperty( "font.bold", boldFont );
    ls->m_TextProp->SetBold( boldFont );
    ls->m_STextProp->SetBold( boldFont );

    bool italicFont(false);
    GetBoolProperty( "font.italic", italicFont );
    ls->m_TextProp->SetBold( italicFont );
    ls->m_STextProp->SetBold( italicFont );


    bool drawShadow;
    GetBoolProperty("drawShadow", drawShadow);
    ls->m_TextProp->SetShadow(false);
    ls->m_STextProp->SetShadow(false);
    ls->m_STextActor->SetVisibility(drawShadow);

    ls->m_TextActor->SetInput(GetText().c_str());
    ls->m_STextActor->SetInput(GetText().c_str());

    mitk::Point2D posT, posS;

    posT[0] = GetPosition2D(renderer)[0] + GetOffsetVector(renderer)[0];
    posT[1] = GetPosition2D(renderer)[1] + GetOffsetVector(renderer)[1];
    posS[0] = posT[0] + 1;
    posS[1] = posT[1] - 1;

    applyTextOrientation(ls->m_TextProp, m_Orientation);
    applyTextOrientation(ls->m_STextProp, m_Orientation);

    float xt = posT[0];
    float yt = posT[1];
    float xs = posS[0];
    float ys = posS[1];

    calculateTextPosWithOffset(xt, yt);
    calculateTextPosWithOffset(xs, ys);

    ls->m_TextActor->SetDisplayPosition(xt, yt);
    ls->m_STextActor->SetDisplayPosition(xs, ys);

    ls->UpdateGenerateDataTime();
  }
}

void mitk::TextOverlay2D::applyTextOrientation(vtkSmartPointer<vtkTextProperty>& textProperty, mitk::TextOrientation& orientation)
{
  if (orientation == mitk::TextOrientation::TextRigth)
  {
    textProperty->SetJustificationToLeft();
    textProperty->SetVerticalJustificationToTop();
  }
  else if (orientation == mitk::TextOrientation::TextCenterBottom)
  {
    textProperty->SetJustificationToCentered();
    textProperty->SetVerticalJustificationToTop();
  }
  else if (orientation == mitk::TextOrientation::TextCenterTop)
  {
    textProperty->SetJustificationToCentered();
    textProperty->SetVerticalJustificationToBottom();
  }
  else if (orientation == mitk::TextOrientation::TextLeft)
  {
    textProperty->SetJustificationToRight();
    textProperty->SetVerticalJustificationToTop();
  }
}

void mitk::TextOverlay2D::calculateTextPosWithOffset(float& x, float& y)
{
  x += 10.0;
  y += 10.0;

  if (m_Orientation == mitk::TextOrientation::TextLeft)
  {
    x -= 25.0;
  }
  else if (m_Orientation == mitk::TextOrientation::TextCenterTop)
  {
    y += 7.0;
  }
  else if (m_Orientation == mitk::TextOrientation::TextCenterBottom)
  {
    y -= 30.0;
  }
}

vtkProp *mitk::TextOverlay2D::GetVtkProp(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_Assembly;
}

vtkActor2D* mitk::TextOverlay2D::GetVtkActor2D(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_TextActor;
}
