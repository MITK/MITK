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

#include "mitkImageWriter.h"

#include "mitkItkPictureWrite.h"
#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkPicFileWriter.h"
#include "mitkImageAccessByItk.h"

#include <itkImageIOBase.h>
#include <itkImageIOFactory.h>


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

void mitk::ImageWriter::WriteByITK(mitk::Image* image, const std::string& fileName)
{
  // Pictures and picture series like .png are written via a different mechanism then volume images.
  // So, they are still multiplexed and thus not support vector images.
  if (fileName.find(".png") != std::string::npos || fileName.find(".tif") != std::string::npos || fileName.find(".jpg") != std::string::npos)
  {
    AccessByItk_1( image, _mitkItkPictureWrite, fileName );
    return;
  }

  // Implementation of writer using itkImageIO directly. This skips the use
  // of templated itkImageFileWriter, which saves the multiplexing on MITK side.

  unsigned int dimension = image->GetDimension();
  unsigned int* dimensions = image->GetDimensions();
  mitk::PixelType pixelType = image->GetPixelType();
  mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  mitk::Point3D origin = image->GetGeometry()->GetOrigin();

  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO( fileName.c_str(), 
    itk::ImageIOFactory::WriteMode );

  if(imageIO.IsNull())
  {
    itkExceptionMacro(<< "Error: Could not create itkImageIO via factory for file " << fileName);
  }

  // Set the necessary information for imageIO
  imageIO->SetNumberOfDimensions(dimension);
  imageIO->SetPixelTypeInfo( *(pixelType.GetItkTypeId()) );

  if(pixelType.GetNumberOfComponents() > 1)
    imageIO->SetNumberOfComponents(pixelType.GetNumberOfComponents());

  itk::ImageIORegion ioRegion( dimension );

  for(unsigned int i=0; i<dimension; i++)
  {
    imageIO->SetDimensions(i,dimensions[i]);
    imageIO->SetSpacing(i,spacing[i]);
    imageIO->SetOrigin(i,origin[i]);

    mitk::Vector3D direction = image->GetGeometry()->GetAxisVector(i);
    vnl_vector< double > axisDirection(dimension);
    for(unsigned int j=0; j<dimension; j++)
    {
      axisDirection[j] = direction[j];
    }
    imageIO->SetDirection( i, axisDirection );

    ioRegion.SetSize(i, image->GetLargestPossibleRegion().GetSize(i) );
    ioRegion.SetIndex(i, image->GetLargestPossibleRegion().GetIndex(i) );
  }

  imageIO->SetIORegion(ioRegion);
  imageIO->SetFileName(fileName);

  const void * data = image->GetData();
  imageIO->Write(data);
}

void mitk::ImageWriter::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  FILE* tempFile = fopen(m_FileName.c_str(),"w");
  if (tempFile==NULL)
  {
    itkExceptionMacro(<<"File location not writeable");
    return;
  }
  fclose(tempFile);
  remove(m_FileName.c_str());

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
    size_t found;
    found = m_FileName.find( m_Extension ); // !!! HAS to be at the very end of the filename (not somewhere in the middle)
    if( m_FileName.length() > 3 && found != m_FileName.length() - 4 )
    {
      //if Extension not in Filename
      ::itk::OStringStream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      m_FileName = filename.str().c_str();
    }
    picWriter->SetFileName( m_FileName.c_str() );
    picWriter->SetInput( input );
    picWriter->Write();
  }
  m_MimeType = "application/MITK.Pic";
}

bool mitk::ImageWriter::CanWriteDataType( DataNode* input )
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

void mitk::ImageWriter::SetInput( DataNode* input )
{
  if( input && CanWriteDataType( input ) )
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::Image*>( input->GetData() ) );
}

std::string mitk::ImageWriter::GetWritenMIMEType()
{
  return m_MimeType;
}

std::vector<std::string> mitk::ImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".pic");
  possibleFileExtensions.push_back(".bmp");
  possibleFileExtensions.push_back(".dcm");
  possibleFileExtensions.push_back(".DCM");
  possibleFileExtensions.push_back(".dicom");
  possibleFileExtensions.push_back(".DICOM");
  possibleFileExtensions.push_back(".gipl");
  possibleFileExtensions.push_back(".gipl.gz");
  possibleFileExtensions.push_back(".mha");
  possibleFileExtensions.push_back(".nii");
  possibleFileExtensions.push_back(".nrrd");
  possibleFileExtensions.push_back(".nhdr");
  possibleFileExtensions.push_back(".png");
  possibleFileExtensions.push_back(".PNG");
  possibleFileExtensions.push_back(".spr");
  possibleFileExtensions.push_back(".mhd");
  possibleFileExtensions.push_back(".vtk");
  possibleFileExtensions.push_back(".vti");
  possibleFileExtensions.push_back(".hdr");
  possibleFileExtensions.push_back(".png");
  possibleFileExtensions.push_back(".tif");
  possibleFileExtensions.push_back(".jpg");
  return possibleFileExtensions;
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
