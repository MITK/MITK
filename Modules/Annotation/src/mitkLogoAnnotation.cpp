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

#include "mitkLogoAnnotation.h"
#include <mitkIOUtil.h>
#include "vtkUnicodeString.h"
#include <vtkImageData.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkImageMapper.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRenderer.h>
#include <vtkTextRendererStringToImage.h>

#include <mitkVtkLogoRepresentation.h>
#include <vtkImageImport.h>
#include <vtkProperty2D.h>
#include <vtkImageFlip.h>
#include <vtkNew.h>

mitk::LogoAnnotation::LogoAnnotation()
{
  m_readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
  mitk::Point2D offset;
  offset.Fill(0.03);
  SetOffsetVector(offset);
  SetRelativeSize(0.2);
  SetLogoImagePath("mbiLogo");
  SetCornerPosition(3);
  m_VtkImageImport = vtkSmartPointer<vtkImageImport>::New();
  m_UpdatedLogoImage = vtkSmartPointer<vtkImageData>::New();
}

mitk::LogoAnnotation::~LogoAnnotation()
{
  for (BaseRenderer *renderer : m_LSH.GetRegisteredBaseRenderer())
  {
    if (renderer)
    {
      this->RemoveFromBaseRenderer(renderer);
    }
  }
}

mitk::LogoAnnotation::LocalStorage::~LocalStorage()
{
}

mitk::LogoAnnotation::LocalStorage::LocalStorage()
{
  m_LogoRep = vtkSmartPointer<mitkVtkLogoRepresentation>::New();
}

void mitk::LogoAnnotation::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this))
  {
    ls->m_LogoImage = m_UpdatedLogoImage;

    ls->m_LogoRep->SetImage(ls->m_LogoImage);
    ls->m_LogoRep->SetDragable(false);
    ls->m_LogoRep->SetMoving(false);
    ls->m_LogoRep->SetPickable(false);
    ls->m_LogoRep->SetShowBorder(true);
    ls->m_LogoRep->SetRenderer(renderer->GetVtkRenderer());
    float size = GetRelativeSize();
    ls->m_LogoRep->SetPosition2(size, size);
    int corner = GetCornerPosition();
    ls->m_LogoRep->SetCornerPosition(corner);
    mitk::Point2D offset = GetOffsetVector();
    ls->m_LogoRep->SetPosition(offset[0], offset[1]);
    float opacity = 1.0;
    GetOpacity(opacity);
    ls->m_LogoRep->GetImageProperty()->SetOpacity(opacity);
    ls->m_LogoRep->BuildRepresentation();
    ls->UpdateGenerateDataTime();
  }
}

void mitk::LogoAnnotation::SetLogoImagePath(std::string path)
{
  SetStringProperty("Annotation.LogoImagePath", path.c_str());
  Modified();
}

void mitk::LogoAnnotation::LoadLogoImageFromPath()
{
  auto imageNodeVector = mitk::IOUtil::Load(this->GetLogoImagePath());

  if (imageNodeVector.size() == 1)
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(imageNodeVector.front().GetPointer());

    if (image.IsNotNull())
    {
      vtkNew<vtkImageFlip> flip;
      flip->SetInputData(image->GetVtkImageData());
      flip->SetFilteredAxis(1);
      flip->Update();
      m_UpdatedLogoImage->DeepCopy(flip->GetOutput());
      Modified();
    }
  }
}

void mitk::LogoAnnotation::SetLogoImage(vtkSmartPointer<vtkImageData> logo)
{
  m_UpdatedLogoImage = logo;
  Modified();
}

std::string mitk::LogoAnnotation::GetLogoImagePath() const
{
  std::string path;
  GetPropertyList()->GetStringProperty("Annotation.LogoImagePath", path);
  return path;
}

void mitk::LogoAnnotation::SetOffsetVector(const Point2D &OffsetVector)
{
  mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
  SetProperty("Annotation.OffsetVector", OffsetVectorProperty.GetPointer());
  Modified();
}

mitk::Point2D mitk::LogoAnnotation::GetOffsetVector() const
{
  mitk::Point2D OffsetVector;
  OffsetVector.Fill(0);
  GetPropertyValue<mitk::Point2D>("Annotation.OffsetVector", OffsetVector);
  return OffsetVector;
}

void mitk::LogoAnnotation::SetCornerPosition(const int &corner)
{
  SetIntProperty("Annotation.CornerPosition", corner);
  Modified();
}

int mitk::LogoAnnotation::GetCornerPosition() const
{
  int corner = 0;
  GetIntProperty("Annotation.CornerPosition", corner);
  return corner;
}

void mitk::LogoAnnotation::SetRelativeSize(const float &size)
{
  SetFloatProperty("Annotation.RelativeSize", size);
  Modified();
}

float mitk::LogoAnnotation::GetRelativeSize() const
{
  float size = 0;
  GetFloatProperty("Annotation.RelativeSize", size);
  return size;
}

vtkProp *mitk::LogoAnnotation::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_LogoRep;
}
