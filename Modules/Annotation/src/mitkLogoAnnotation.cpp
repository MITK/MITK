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
#include "mitkEqual.h"
#include "vtkUnicodeString.h"
#include <vtkImageData.h>
#include <vtkImageData.h>
#include <vtkImageMapper.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRenderer.h>
#include <vtkTextRendererStringToImage.h>
#include <vtkCommand.h>

#include <mbilogo.h>
#include <vtkImageProperty.h>
#include <vtkActor2D.h>
#include <vtkImageImport.h>
#include <vtkProperty2D.h>
#include <vtkImageResize.h>

class vtkWindowModifiedCallback : public vtkCommand
{
public:
  static vtkWindowModifiedCallback *New()
  {
    return new vtkWindowModifiedCallback;
  }
  
  mitk::LogoAnnotation::LocalStorage* ls;

  virtual void Execute(vtkObject *caller, unsigned long, void* calldata)
  {
    vtkRenderWindow* window = static_cast<vtkRenderWindow*>(caller);

    int dims[3];
    ls->m_LogoImage->GetDimensions(dims);    
    int *size = window->GetSize();
    double currentFactor = double(dims[0]) / double(size[0]);

    double desiredFactor = 0.20;
    double scaleFactor = desiredFactor / currentFactor;

    double magnificationFactor[3];
    ls->m_ImageResize->GetMagnificationFactors(magnificationFactor);

    if (!mitk::Equal(scaleFactor, magnificationFactor[0])){
      //float size = GetRelativeSize();            
      ls->m_ImageResize->SetMagnificationFactors(scaleFactor, scaleFactor, scaleFactor);
      ls->m_ImageResize->Update();
    }

    ls->m_ImageActor->GetProperty()->SetColor(0., 0., 1.);
    ls->m_ImageActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    ls->m_ImageActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
    ls->m_ImageActor->SetPosition(0.75, 0.05);
  }

  vtkWindowModifiedCallback() { this->ls = 0; }
};


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
  m_ImageActor = vtkSmartPointer<vtkActor2D>::New();
  m_ImageMapper = vtkSmartPointer<vtkImageMapper>::New();
  m_ImageActor->SetMapper(m_ImageMapper);

  m_ImageResize = vtkSmartPointer<vtkImageResize>::New();
}

void mitk::LogoAnnotation::UpdateVtkAnnotation(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = this->m_LSH.GetLocalStorage(renderer);
  if (ls->IsGenerateDataRequired(renderer, this))
  {
    if (GetLogoImagePath().empty())
    {
      ls->m_ImageActor->SetVisibility(0);
      return;
    }
    vtkImageReader2 *imageReader = m_readerFactory->CreateImageReader2(GetLogoImagePath().c_str());
    if (imageReader)
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
    ls->m_ImageMapper->SetInputConnection(ls->m_ImageResize->GetOutputPort());
    ls->m_ImageMapper->SetColorWindow(255);
    ls->m_ImageMapper->SetColorLevel(127.5);
    ls->m_ImageResize->SetInputData(ls->m_LogoImage);
    ls->m_ImageResize->SetResizeMethodToMagnificationFactors();
    ls->m_ImageResize->SetMagnificationFactors(1., 1., 1.);
    vtkSmartPointer<vtkWindowModifiedCallback> wCallback = vtkSmartPointer<vtkWindowModifiedCallback>::New();
    wCallback->ls = ls;
    renderer->GetRenderWindow()->AddObserver(vtkCommand::ModifiedEvent, wCallback);
    
    ls->UpdateGenerateDataTime();
  }
}

vtkImageData *mitk::LogoAnnotation::CreateMbiLogo()
{
  m_VtkImageImport->SetDataScalarTypeToUnsignedChar();
  m_VtkImageImport->SetNumberOfScalarComponents(mbiLogo_NumberOfScalars);
  m_VtkImageImport->SetWholeExtent(0, mbiLogo_Width - 1, 0, mbiLogo_Height - 1, 0, 1 - 1);
  m_VtkImageImport->SetDataExtentToWholeExtent();

  char *ImageData;
  // flip mbi logo around y axis and change color order
  ImageData = new char[mbiLogo_Height * mbiLogo_Width * mbiLogo_NumberOfScalars];

  unsigned int column, row;
  char *dest = ImageData;
  char *source = (char *)&mbiLogo_Data[0];
  ;
  char r, g, b, a;
  for (column = 0; column < mbiLogo_Height; column++)
    for (row = 0; row < mbiLogo_Width; row++)
    { // change r with b
      b = *source++;
      g = *source++;
      r = *source++;
      a = *source++;

      *dest++ = r;
      *dest++ = g;
      *dest++ = b;
      *dest++ = a;
    }

  m_VtkImageImport->SetImportVoidPointer(ImageData);
  m_VtkImageImport->Modified();
  m_VtkImageImport->Update();
  return m_VtkImageImport->GetOutput();
}

void mitk::LogoAnnotation::SetLogoImagePath(std::string path)
{
  SetStringProperty("Annotation.LogoImagePath", path.c_str());
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
  
  return ls->m_ImageActor;
}
