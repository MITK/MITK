/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkImageWriter.h"
#include "mitkItkImageWrite.h"

#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkPicFileWriter.h"
#include <itksys/SystemTools.hxx>

#include <sstream>

mitk::ImageWriter::ImageWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
  m_MimeType = "";
  SetDefaultExtension();
}

mitk::ImageWriter::~ImageWriter()
{
}

void mitk::ImageWriter::SetDefaultExtension()
{
  m_Extension = ".mhd";
}

#include <vtkConfigure.h>
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
#include <vtkImageData.h>
#include <vtkXMLImageDataWriter.h>
static void writeVti(const char * filename, mitk::Image* image, int t=0)
{
  vtkXMLImageDataWriter * vtkwriter = vtkXMLImageDataWriter::New();
  vtkwriter->SetFileName( filename );
  vtkwriter->SetInput(image->GetVtkImageData(t));
  vtkwriter->Write();
  vtkwriter->Delete();
}
#endif

void mitk::ImageWriter::WriteByITK(mitk::Image* image, const std::string& filename)
{
  if(image->GetPixelType().GetNumberOfComponents()==1)
  {
    AccessByItk_1( image, _mitkItkImageWrite, filename );
  }
  //// Extension for RGB (and maybe also for vector types)
  //// Does not work yet, see bug 320 and mitkImageCastPart2.cpp
  //else
  //if(image->GetPixelType().GetNumberOfComponents()==3)
  //{
  //  const std::type_info& typeId=*(image)->GetPixelType().GetTypeId();
  //  if ( typeId == typeid(unsigned char) )
  //  {
  //    if(image->GetDimension()==2)
  //    {
  //      typedef itk::Image<itk::RGBPixel<unsigned char>, 2>  itkImageRGBUC2;
  //      itkImageRGBUC2::Pointer itkRGBimage;
  //      mitk::CastToItkImage(image, itkRGBimage);
  //      _mitkItkImageWrite(itkRGBimage.GetPointer(), filename);
  //    }
  //    else
  //    if(image->GetDimension()==3)
  //    {
  //      typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;
  //      itkImageRGBUC3::Pointer itkRGBimage;
  //      mitk::CastToItkImage(image, itkRGBimage);
  //      _mitkItkImageWrite(itkRGBimage.GetPointer(), filename);
  //    }
  //  }
  //  else
  //  {
  //    itkWarningMacro(<<"Sorry, cannot write images with GetNumberOfComponents()==3 that " 
  //      << "have pixeltype " << typeId.name() << " using ITK writers .");
  //  }
  //}
  else
  {
    itkWarningMacro(<<"Sorry, cannot write images with GetNumberOfComponents()==" 
      << image->GetPixelType().GetNumberOfComponents() << " using ITK writers .");
  }
}

void mitk::ImageWriter::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::Image::Pointer input = const_cast<mitk::Image*>(this->GetInput());

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  bool vti = (m_Extension.find(".vti") != std::string::npos);
#endif

  if ( m_Extension.find(".pic") == std::string::npos )
  {
    if(input->GetDimension() > 3)
    {
      int t, timesteps;

      timesteps = input->GetDimension(3);
      ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput(input);
      mitk::Image::Pointer image = timeSelector->GetOutput();
      for(t = 0; t < timesteps; ++t)
      {
        ::itk::OStringStream filename;
        timeSelector->SetTimeNr(t);
        timeSelector->Update();
        if(input->GetTimeSlicedGeometry()->IsValidTime(t))
        {
          const mitk::TimeBounds& timebounds = input->GetTimeSlicedGeometry()->GetGeometry3D(t)->GetTimeBounds();
          filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << m_Extension;
        }
        else
        {
          itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of image " << filename << ".");
          filename <<  m_FileName.c_str() << "_T" << t << m_Extension;
        }
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
        if ( vti )
        {
          writeVti(filename.str().c_str(), input, t);
        }
        else
#endif
        {        
          WriteByITK(input, filename.str());
        }
      }
    }
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    else if ( vti )
    {
      ::itk::OStringStream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      writeVti(filename.str().c_str(), input);
    }
#endif
    else
    {
      ::itk::OStringStream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      WriteByITK(input, filename.str());
    }
  }
  else
  {
    PicFileWriter::Pointer picWriter = PicFileWriter::New();
    //case: filename got always an extension
    size_t found;
    found = m_FileName.find( m_Extension );
    if( found == std::string::npos )
    {
      //if Extension not in Filename
      ::itk::OStringStream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      picWriter->SetFileName( filename.str().c_str() );
    }
    else
      picWriter->SetFileName( m_FileName.c_str() );
    picWriter->SetInput( input );
    picWriter->Write();
  }
  m_MimeType = "image/pict";
}

bool mitk::ImageWriter::CanWrite( DataTreeNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
       mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( data );
       if( image.IsNotNull() )
       {
         //"SetDefaultExtension()" set m_Extension to ".mhd" ?????
         m_Extension = ".pic";
         return true;
       }
    }
  }
  return false;
}

void mitk::ImageWriter::SetInput( DataTreeNode* input )
{
  if( input && CanWrite( input ) )
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::Image*>( input->GetData() ) );
}

std::string mitk::ImageWriter::GetWritenMIMEType()
{
  return m_MimeType;
}

std::string mitk::ImageWriter::GetFileExtension()
{
  return m_Extension;
}

void mitk::ImageWriter::SetInput( mitk::Image* image )
{
  this->ProcessObject::SetNthInput( 0, image );
}

const mitk::Image* mitk::ImageWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return static_cast< const mitk::Image * >( this->ProcessObject::GetInput( 0 ) );
  }
}
