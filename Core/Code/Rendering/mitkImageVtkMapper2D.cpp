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

mitk::ImageVtkMapper2D::ImageVtkMapper2D()
{
  this->m_VtkBased = true;
  this->m_TimeStep = 0;
  this->m_VtkActor = vtkSmartPointer<vtkImageActor>::New();
  this->m_VtkImage = vtkImageData::New();
//  m_VtkActor->SetVisibility(1);
  this->m_VtkMapper = vtkPolyDataMapper::New();
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

//  m_VtkImage = this->GenerateTestImageForTSFilter();

  vtkSmartPointer<vtkImageData> image = input->GetVtkImageData(m_TimeStep, 0);
//  vtkSmartPointer<vtkImageShiftScale> imageShiftSacle = vtkImageShiftScale::New();
//  imageShiftSacle->SetOutputScalarTypeToUnsignedChar();
//  imageShiftSacle->ClampOverflowOn();
//  imageShiftSacle->SetInput(image);
  vtkSmartPointer<vtkImageCast> imageCast = vtkSmartPointer<vtkImageCast>::New();
  imageCast->SetOutputScalarTypeToUnsignedChar();
  imageCast->ClampOverflowOn();
  imageCast->SetInput(image);
//  m_VtkImage->SetScalarTypeToUnsignedChar();;
  m_VtkImage = imageCast->GetOutput();




  if( m_VtkImage )
  {
//    vtkSmartPointer<vtkImageQuantizeRGBToIndex> quant =
//      vtkSmartPointer<vtkImageQuantizeRGBToIndex>::New();
//    quant->SetInputConnection(reader->GetOutputPort());
//    quant->SetNumberOfColors(16);

//    vtkSmartPointer<vtkImageToPolyDataFilter> i2pd =
//      vtkSmartPointer<vtkImageToPolyDataFilter>::New();
//    i2pd->SetInputConnection(quant->GetOutputPort());
//    i2pd->SetLookupTable(quant->GetLookupTable());
//    i2pd->SetColorModeToLUT();
//    i2pd->SetOutputStyleToPolygonalize();
//    i2pd->SetError(0);
//    i2pd->DecimationOn();
//    i2pd->SetDecimationError(0.0);
//    i2pd->SetSubImageSize(25);

//    // Need a triangle filter because the polygons are complex and concave
//    vtkSmartPointer<vtkTriangleFilter> tf =
//      vtkSmartPointer<vtkTriangleFilter>::New();
//    tf->SetInputConnection(i2pd->GetOutputPort());#include <vtkImageCast.h>


//    vtkSmartPointer<vtkPolyDataMapper> mapper =
//      vtkSmartPointer<vtkPolyDataMapper>::New();
//    mapper->SetInputConnection(tf->GetOutputPort());

//    vtkSmartPointer<vtkActor> actor =
//      vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);
//    actor->GetProperty()->SetRepresentationToWireframe();


    m_VtkActor->SetInput(m_VtkImage);
    MITK_INFO << "VTK image ist da";
    // make sure, that we have point data with more than 1 component (as vectors)
    //    vtkPointData* pointData = m_VtkImage->GetPointData();
    //    if ( pointData == NULL )
    //    {
    //      itkWarningMacro( << "m_VtkImage->GetPointData() returns NULL!" );
    //      return ;
    //    }
    //    if ( pointData->GetNumberOfArrays() == 0 )
    //    {
    //      itkWarningMacro( << "m_VtkImage->GetPointData()->GetNumberOfArrays() is 0!" );
    //      return ;
    //    }
    //    else if ( pointData->GetArray(0)->GetNumberOfComponents() != N
    //      && pointData->GetArray(0)->GetNumberOfComponents() != 6 /*for tensor visualization*/)
    //    {
    //      itkWarningMacro( << "number of components != number of directions in ODF!" );
    //      return;
    //    }
    //    else if ( pointData->GetArrayName( 0 ) == NULL )
    //    {
    //      m_VtkImage->GetPointData()->GetArray(0)->SetName("vector");
    //    }MITK_INFO << "VTK image ist da";
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
  if ( this->IsVisible( renderer )==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
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
  if(m_VtkActor)
    MITK_INFO << "VTK actor wird returned";
  else
    MITK_INFO << "nix";
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
