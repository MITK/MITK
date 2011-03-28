/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImageVtkMapper2D.h"
//#include "mitkMapper.h"
//#include "mitkDataNode.h"
//#include "mitkBaseRenderer.h"
//#include "mitkProperties.h"

#include <vtkImageQuantizeRGBToIndex.h>
#include <vtkImageToPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageShiftScale.h>
#include <vtkImageCast.h>
#include <vtkImageReslice.h>
#include <vtkLinearTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkProperty2D.h>
#include <vtkImageBlend.h>
#include <vtkImage.h>
#include <vtkImageProperty.h>
#include <vtkOpenGLImageResliceMapper.h>
#include <mitkVtkPropRenderer.h>



mitk::ImageVtkMapper2D::ImageVtkMapper2D()
{
  this->m_VtkBased = true;
  this->m_TimeStep = 0;
  this->m_VtkActor = vtkSmartPointer<vtkImage>::New();
  this->m_VtkImage = vtkSmartPointer<vtkImageData>::New();
  this->m_VtkMapper = vtkSmartPointer<vtkOpenGLImageResliceMapper>::New();
}


mitk::ImageVtkMapper2D::~ImageVtkMapper2D()
{
}

const mitk::Image* mitk::ImageVtkMapper2D::GetInput()
{
  return static_cast<const mitk::Image* > ( GetData() );
}

void mitk::ImageVtkMapper2D::GenerateData(mitk::BaseRenderer* renderer)
{
  MITK_INFO << "Generate Data called";
  if ( !this->IsVisible(renderer) )
  {
    itkWarningMacro( << "Renderer not visible!" );
    return;
  }
  mitk::Image::Pointer input = const_cast<mitk::Image*>( this->GetInput() );
  if ( input.IsNull() ) return ;


  vtkSmartPointer<vtkImageData> image = input->GetVtkImageData(0,0);
//  image->SetScalarTypeToUnsignedChar();
//  vtkSmartPointer<vtkImageShiftScale> imageShiftSacle = vtkImageShiftScale::New();
//  imageShiftSacle->SetOutputScalarTypeToUnsignedChar();
//  imageShiftSacle->ClampOverflowOn();
//  imageShiftSacle->SetInput(image);

//  vtkSmartPointer<vtkImageCast> imageCast = vtkSmartPointer<vtkImageCast>::New();
//  imageCast->SetOutputScalarTypeToUnsignedChar();
//  imageCast->ClampOverflowOn();
//  imageCast->SetInput(image);
//  m_VtkImage->SetScalarTypeToUnsignedChar();
//  m_VtkImage = imageCast->GetOutput();
  m_VtkImage = image;


//  float opacity = 1.0f;
//  GetOpacity(opacity, renderer);
//  MITK_INFO << opacity;
////  m_VtkActor->GetProperty()->SetOpacity(opacity);
////  MITK_INFO << m_VtkActor->GetProperty()->GetOpacity();

//  vtkSmartPointer<vtkImageBlend> blend =
//    vtkSmartPointer<vtkImageBlend>::New();
////  blend->AddInputConnection(image->GetOutputPort());
//  blend->AddInputConnection(image->GetProducerPort());
//  blend->SetOpacity(0,opacity);

//  m_VtkImage = blend->GetOutput(0);

  if( m_VtkImage )
  {
    mitk::LevelWindow levelWindow;
    GetLevelWindow(levelWindow, renderer);
//    double range =

//    const mitk::VtkPropRenderer* glRenderer = dynamic_cast<const mitk::VtkPropRenderer*>(renderer);
//    if (glRenderer == NULL)
//      return;
//    vtkRenderer* vtkRenderer = glRenderer->GetVtkRenderer();

    vtkSmartPointer<vtkImageProperty> ip = vtkSmartPointer<vtkImageProperty>::New();
    ip->SetColorLevel(levelWindow.GetLevel());
    ip->SetColorWindow(levelWindow.GetWindow());
    ip->SetInterpolationTypeToLinear();
    ip->SetOpacity(0.99);


//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

//    renderWindowInteractor->SetInteractorStyle(imageStyle);
//    renderWindowInteractor->SetRenderWindow(renderWindow);

//    m_VtkMapper->SetInputConnection(iExtended2Dmage->GetProducerPort());
    m_VtkMapper->SetInput(m_VtkImage);
//    m_VtkMapper->SetInputConnection(blend->GetOutputPort());
//    m_VtkMapper->SetColorLevel(levelWindow.GetLowerWindowBound());
//    m_VtkMapper->SetColorWindow(levelWindow.GetUpperWindowBound());

    m_VtkActor->SetMapper(m_VtkMapper);
    m_VtkActor->SetProperty(ip);

    m_VtkActor->SetVisibility(1);


  }
  else
  {
    itkWarningMacro( << "m_VtkImage is NULL!" );
    return ;
  }
}

void mitk::ImageVtkMapper2D::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}
void mitk::ImageVtkMapper2D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  MitkRenderTranslucentGeometry(renderer);
//  if ( this->IsVisible( renderer )==false )
//    return;

//  if ( this->GetVtkProp(renderer)->GetVisibility() )
//  {
//    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
//  }
}

void mitk::ImageVtkMapper2D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
//BUG (#1551) changed VTK_MINOR_VERSION FROM 3 to 2 cause RenderTranslucentGeometry was changed in minor version 2
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
#else
    this->GetVtkProp(renderer)->RenderTranslucentGeometry(renderer->GetVtkRenderer());
#endif
}

void mitk::ImageVtkMapper2D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
    return;

  if ( GetVtkProp(renderer)->GetVisibility() )
    GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());
}

vtkProp* mitk::ImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{
//  m_VtkActor->GetProperty()->SetColor(0,1,0);
  return m_VtkActor;
}

vtkImageData *mitk::ImageVtkMapper2D::GenerateTestImageForTSFilter()
{
  // a 2x2x2 image
  unsigned char myData[] =
  {

    234,234,
    123,565,

//    -213,800,
//    1000,-20
  };
  vtkImageData *i = vtkImageData::New();

  i->SetExtent(0,1,0,1,0,0);

  i->SetScalarTypeToUnsignedChar();

  i->AllocateScalars();

  unsigned char *p = (unsigned char*)i->GetScalarPointer();

  memcpy(p,myData,2*2*1*sizeof(unsigned char));

  return i;
}
