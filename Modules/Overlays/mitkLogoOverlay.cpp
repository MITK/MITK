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

#include "mitkLogoOverlay.h"
#include <vtkTextProperty.h>
#include "vtkUnicodeString.h"
#include <vtkImageMapper.h>
#include <vtkTextRendererStringToImage.h>
#include <vtkTextActor.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkTextRenderer.h>
#include <vtkImageData.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>

#include <mitkVtkLogoRepresentation.h>
#include <vtkLogoWidget.h>
#include <vtkProperty2D.h>
#include <vtkImageImport.h>
#include <mbilogo.h>

mitk::LogoOverlay::LogoOverlay()
{
  m_readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
  mitk::Point2D offset;
  offset.Fill(0.03);
  SetOffsetVector(offset);
  SetRelativeSize(0.2);
  SetCornerPosition(4);
}


mitk::LogoOverlay::~LogoOverlay()
{
}

mitk::LogoOverlay::LocalStorage::~LocalStorage()
{
}

mitk::LogoOverlay::LocalStorage::LocalStorage()
{
  m_LogoRep = vtkSmartPointer<mitkVtkLogoRepresentation>::New();
  m_LogoWidget = vtkSmartPointer<vtkLogoWidget>::New();
  m_LogoWidget->SetRepresentation(m_LogoRep);
}

void mitk::LogoOverlay::UpdateVtkOverlay(mitk::BaseRenderer *renderer)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  if(ls->IsGenerateDataRequired(renderer,this))
  {

    vtkImageReader2* imageReader = m_readerFactory->CreateImageReader2(GetLogoImagePath().c_str());
    if(imageReader)
    {
      imageReader->SetFileName(GetLogoImagePath().c_str());
      imageReader->Update();
      ls->m_LogoImage = imageReader->GetOutput();
      imageReader->Delete();
    }
    else
    {
      ls->m_LogoImage = CreateMbiLogo();
    }

    ls->m_LogoRep->SetImage(ls->m_LogoImage);
    ls->m_LogoRep->SetDragable(false);
    ls->m_LogoRep->SetMoving(false);
    ls->m_LogoRep->SetPickable(false);
    ls->m_LogoRep->SetShowBorder(true);
    ls->m_LogoRep->SetRenderer(renderer->GetVtkRenderer());
    float size = GetRelativeSize(renderer);
    ls->m_LogoRep->SetPosition2(size,size);
    int corner = GetCornerPosition(renderer);
    ls->m_LogoRep->SetCornerPosition(corner);
    mitk::Point2D offset = GetOffsetVector(renderer);
    ls->m_LogoRep->SetPosition(offset[0],offset[1]);
    float opacity = 1.0;
    GetOpacity(opacity,renderer);
    ls->m_LogoRep->GetImageProperty()->SetOpacity(opacity);
    ls->m_LogoRep->BuildRepresentation();
    ls->UpdateGenerateDataTime();
  }

}

vtkImageData *mitk::LogoOverlay::CreateMbiLogo()
{
  vtkImageImport*     VtkImageImport = vtkImageImport::New();
  VtkImageImport->SetDataScalarTypeToUnsignedChar();
  VtkImageImport->SetNumberOfScalarComponents(mbiLogo_NumberOfScalars);
  VtkImageImport->SetWholeExtent(0,mbiLogo_Width-1,0,mbiLogo_Height-1,0,1-1);
  VtkImageImport->SetDataExtentToWholeExtent();

  char *              ImageData;
  // flip mbi logo around y axis and change color order
  ImageData = new char[mbiLogo_Height*mbiLogo_Width*mbiLogo_NumberOfScalars];

  unsigned int column, row;
  char * dest   = ImageData;
  char * source = (char*) &mbiLogo_Data[0];;
  char r, g, b, a;
  for (column = 0; column < mbiLogo_Height; column++)
    for (row = 0; row < mbiLogo_Width; row++)
    {   //change r with b
        b = *source++;
        g = *source++;
        r = *source++;
        a = *source++;

        *dest++ = r;
        *dest++ = g;
        *dest++ = b;
        *dest++ = a;
      }

  VtkImageImport->SetImportVoidPointer(ImageData);
  VtkImageImport->Modified();
  VtkImageImport->Update();
  return VtkImageImport->GetOutput();
}

void mitk::LogoOverlay::SetLogoImagePath(std::string path)
{
  SetStringProperty("Overlay.LogoImagePath", path.c_str());
  Modified();
}

std::string mitk::LogoOverlay::GetLogoImagePath() const
{
  std::string path;
  GetPropertyList()->GetStringProperty("Overlay.LogoImagePath", path);
  return path;
}

void mitk::LogoOverlay::SetOffsetVector(const Point2D& OffsetVector, mitk::BaseRenderer *renderer)
{
  mitk::Point2dProperty::Pointer OffsetVectorProperty = mitk::Point2dProperty::New(OffsetVector);
  SetProperty("Overlay.OffsetVector", OffsetVectorProperty.GetPointer(),renderer);
    Modified();
}

mitk::Point2D mitk::LogoOverlay::GetOffsetVector(mitk::BaseRenderer *renderer) const
{
  mitk::Point2D OffsetVector;
  OffsetVector.Fill(0);
  GetPropertyValue<mitk::Point2D>("Overlay.OffsetVector", OffsetVector, renderer);
  return OffsetVector;
}

void mitk::LogoOverlay::SetCornerPosition(const int &corner, mitk::BaseRenderer *renderer)
{
  SetIntProperty("Overlay.CornerPosition", corner, renderer);
  Modified();
}

int mitk::LogoOverlay::GetCornerPosition(mitk::BaseRenderer *renderer) const
{
  int corner = 0;
  GetIntProperty("Overlay.CornerPosition",corner,renderer);
  return corner;
}

void mitk::LogoOverlay::SetRelativeSize(const float &size, mitk::BaseRenderer *renderer)
{
  SetFloatProperty("Overlay.RelativeSize", size, renderer);
  Modified();
}

float mitk::LogoOverlay::GetRelativeSize(mitk::BaseRenderer *renderer) const
{
  float size = 0;
  GetFloatProperty("Overlay.RelativeSize",size,renderer);
  return size;
}

vtkProp* mitk::LogoOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_LogoRep;
}
