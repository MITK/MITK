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

#include <vtkLogoRepresentation.h>
#include <vtkLogoWidget.h>
#include <vtkProperty2D.h>
#include <vtkImageImport.h>
#include <mbilogo.h>

mitk::LogoOverlay::LogoOverlay()
{
  m_readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
  mitk::Point2D position;
  position[0] = position[1] = 0;
}


mitk::LogoOverlay::~LogoOverlay()
{
}

vtkImageData *mitk::LogoOverlay::CreateMbiLogo()
{
  vtkImageImport*     VtkImageImport = vtkImageImport::New();
  VtkImageImport->SetDataScalarTypeToUnsignedChar();
  VtkImageImport->SetNumberOfScalarComponents(mbiLogo_NumberOfScalars);
  VtkImageImport->SetWholeExtent(0,mbiLogo_Width-1,0,mbiLogo_Height-1,0,1-1);
  VtkImageImport->SetDataExtentToWholeExtent();

  char * ImageData;
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

mitk::Overlay::Bounds mitk::LogoOverlay::GetBoundsOnDisplay(mitk::BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  mitk::Overlay::Bounds bounds;
  bounds.Position = ls->m_LogoRep->GetPosition();
//  double* size = new double[2];
  bounds.Size[0] = ls->m_LogoRep->GetPosition2()[0];
  bounds.Size[1] = ls->m_LogoRep->GetPosition2()[1];
  return bounds;
}

void mitk::LogoOverlay::SetBoundsOnDisplay(mitk::BaseRenderer *renderer, const mitk::Overlay::Bounds& bounds)
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  ls->m_LogoRep->SetPosition(bounds.Position[0],bounds.Position[1]);
  ls->m_LogoRep->SetPosition2(bounds.Size[0],bounds.Size[1]);
}

mitk::LogoOverlay::LocalStorage::~LocalStorage()
{
}

mitk::LogoOverlay::LocalStorage::LocalStorage()
{
  m_LogoRep = vtkSmartPointer<vtkLogoRepresentation>::New();
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
    ls->m_LogoRep->SetShowBorder(false);
    ls->m_LogoRep->SetRenderer(renderer->GetVtkRenderer());
    float opacity = 1.0;
    this->GetOpacity(opacity,renderer);
    ls->m_LogoRep->GetImageProperty()->SetOpacity(opacity);

    ls->m_LogoRep->BuildRepresentation();
    ls->UpdateGenerateDataTime();
  }

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

vtkProp* mitk::LogoOverlay::GetVtkProp(BaseRenderer *renderer) const
{
  LocalStorage* ls = this->m_LSH.GetLocalStorage(renderer);
  return ls->m_LogoRep;
}

