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
#include <vtkImageMapper.h>
#include <vtkTextRendererStringToImage.h>
#include <vtkTextActor.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkTextRenderer.h>
#include <vtkImageData.h>

mitk::TextOverlay2D::TextOverlay2D()
{
  mitk::Point2D position;
  position[0] = position[1] = 0;
  SetPosition2D(position);
  SetOffsetVector(position);
}


mitk::TextOverlay2D::~TextOverlay2D()
{
}

mitk::Overlay::Bounds mitk::TextOverlay2D::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  mitk::Overlay::Bounds bounds;
  bounds.Position = ls->m_textActor->GetPosition();
  bounds.Size[0] = ls->m_textImage->GetDimensions()[0];
  bounds.Size[1] = ls->m_textImage->GetDimensions()[1];
  return bounds;
}

void mitk::TextOverlay2D::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Overlay::Bounds& bounds)
{
  vtkSmartPointer<vtkActor2D> actor = GetVtkActor2D(renderer);
  actor->SetDisplayPosition(bounds.Position[0],bounds.Position[1]);
//  actor->SetWidth(bounds.Size[0]);
//  actor->SetHeight(bounds.Size[1]);
}

mitk::TextOverlay2D::LocalStorage::~LocalStorage()
{
}

mitk::TextOverlay2D::LocalStorage::LocalStorage()
{
  m_textActor = vtkSmartPointer<vtkActor2D>::New();
  m_textImage = vtkSmartPointer<vtkImageData>::New();
  m_imageMapper = vtkSmartPointer<vtkImageMapper>::New();
  m_imageMapper->SetInputData(m_textImage);
  m_textActor->SetMapper(m_imageMapper);
}

void mitk::TextOverlay2D::UpdateVtkOverlay2D(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);

  if(ls->IsGenerateDataRequired(renderer,this))
  {
    vtkSmartPointer<vtkTextRendererStringToImage> freetype = vtkSmartPointer<vtkTextRendererStringToImage>::New();
    vtkSmartPointer<vtkTextProperty> prop = vtkSmartPointer<vtkTextProperty>::New();
    float color[3] = {1,1,1};
    float opacity = 1.0;
    GetColor(color,renderer);
    GetOpacity(opacity,renderer);
    prop->SetColor( color[0], color[1], color[2]);
    prop->SetFontSize(GetFontSize());
    prop->SetOpacity(opacity);

    freetype->SetScaleToPowerOfTwo(false);

    freetype->RenderString(prop,vtkUnicodeString::from_utf8(GetText().c_str()),ls->m_textImage);
//    vtkSmartPointer<vtkTextRenderer> fds = vtkTextRenderer::New();
//    int bbox[4];
//    fds->GetBoundingBox(prop,vtkUnicodeString::from_utf8(GetText().c_str()),bbox);

    ls->m_textImage->Modified();

    //Levelwindow has to be set to full range, that the colors are displayed properly.
    ls->m_imageMapper->SetColorWindow(255);
    ls->m_imageMapper->SetColorLevel(127.5);

    ls->m_imageMapper->Update();
    ls->m_textActor->SetPosition(GetPosition2D(renderer)[0]+GetOffsetVector(renderer)[0], GetPosition2D(renderer)[1]+GetOffsetVector(renderer)[1]);
    ls->UpdateGenerateDataTime();
  }

}

vtkActor2D* mitk::TextOverlay2D::GetVtkActor2D(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_textActor;
}

