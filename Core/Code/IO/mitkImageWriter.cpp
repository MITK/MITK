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

#include "mitkImageWriter.h"

#include "mitkItkPictureWrite.h"
#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageReadAccessor.h"

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

#include <itkRGBAPixel.h>

void mitk::ImageWriter::WriteByITK(mitk::Image* image, const std::string& fileName)
{
  // Pictures and picture series like .png are written via a different mechanism then volume images.
  // So, they are still multiplexed and thus not support vector images.
  if (fileName.find(".png") != std::string::npos || fileName.find(".tif") != std::string::npos || fileName.find(".jpg") != std::string::npos)
  {
    try
    {
      // switch processing of single/multi-component images
      if( image->GetPixelType(0).GetNumberOfComponents() == 1)
      {
        AccessByItk_1( image, _mitkItkPictureWrite, fileName );
      }
      else
      {
        AccessFixedPixelTypeByItk_1( image, _mitkItkPictureWriteComposite, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ , fileName);
      }
    }
    catch(itk::ExceptionObject &e)
    {
      std::cerr << "Caught " << e.what() << std::endl;
    }
    catch(std::exception &e)
    {
      std::cerr << "Caught std::exception " << e.what() << std::endl;
    }

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
  imageIO->SetPixelType( pixelType.GetPixelType() );
  imageIO->SetComponentType( pixelType.GetComponentType() < PixelComponentUserType ?
                             static_cast<itk::ImageIOBase::IOComponentType>(pixelType.GetComponentType()) :
                             itk::ImageIOBase::UNKNOWNCOMPONENTTYPE);
  imageIO->SetNumberOfComponents( pixelType.GetNumberOfComponents() );

  itk::ImageIORegion ioRegion( dimension );

  for(unsigned int i=0; i<dimension; i++)
  {
    imageIO->SetDimensions(i,dimensions[i]);
    imageIO->SetSpacing(i,spacing[i]);
    imageIO->SetOrigin(i,origin[i]);

    mitk::Vector3D direction;
    direction.SetVnlVector(image->GetGeometry()->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(i));
    vnl_vector< double > axisDirection(dimension);
    for(unsigned int j=0; j<dimension; j++)
    {
      axisDirection[j] = direction[j]/spacing[i];
    }
    imageIO->SetDirection( i, axisDirection );

    ioRegion.SetSize(i, image->GetLargestPossibleRegion().GetSize(i) );
    ioRegion.SetIndex(i, image->GetLargestPossibleRegion().GetIndex(i) );
  }

  //use compression if available
  imageIO->UseCompressionOn();

  imageIO->SetIORegion(ioRegion);
  imageIO->SetFileName(fileName);

  ImageReadAccessor imageAccess(image);
  imageIO->Write(imageAccess.GetData());
}

void mitk::ImageWriter::GenerateData()
{
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

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

  // Creating clone of input image, since i might change the geometry
  mitk::Image::Pointer input = const_cast<mitk::Image*>(this->GetInput())->Clone();

  // Check if geometry information will be lost
  if (input->GetDimension() == 2)
  {
     if (!input->GetGeometry()->Is2DConvertable())
     {
        MITK_WARN << "Saving a 2D image with 3D geometry information. Geometry information will be lost! You might consider using Convert2Dto3DImageFilter before saving.";

        // set matrix to identity
        mitk::AffineTransform3D::Pointer affTrans = mitk::AffineTransform3D::New();
        affTrans->SetIdentity();
        mitk::Vector3D spacing = input->GetGeometry()->GetSpacing();
        mitk::Point3D origin = input->GetGeometry()->GetOrigin();
        input->GetGeometry()->SetIndexToWorldTransform(affTrans);
        input->GetGeometry()->SetSpacing(spacing);
        input->GetGeometry()->SetOrigin(origin);
     }
  }

  bool vti = (m_Extension.find(".vti") != std::string::npos);

  // If the extension is NOT .pic and NOT .nrrd and NOT .nii and NOT .nii.gz the following block is entered
  if ( m_Extension.find(".pic") == std::string::npos
       && m_Extension.find(".nrrd") == std::string::npos
       && m_Extension.find(".nii") == std::string::npos
       && m_Extension.find(".nii.gz") == std::string::npos
       )
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
        std::ostringstream filename;
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
        if ( vti )
        {
          writeVti(filename.str().c_str(), input, t);
        }
        else
        {
          WriteByITK(image, filename.str());
        }
      }
    }
    else if ( vti )
    {
      std::ostringstream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      writeVti(filename.str().c_str(), input);
    }
    else
    {
      std::ostringstream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      WriteByITK(input, filename.str());
    }
  }
  else
  {
    // use the PicFileWriter for the .pic data type
    if( m_Extension.find(".pic") != std::string::npos )
    {
/*    PicFileWriter::Pointer picWriter = PicFileWriter::New();
    size_t found;
    found = m_FileName.find( m_Extension ); // !!! HAS to be at the very end of the filename (not somewhere in the middle)
    if( m_FileName.length() > 3 && found != m_FileName.length() - 4 )
    {
      //if Extension not in Filename
      std::ostringstream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      picWriter->SetFileName( filename.str().c_str() );
    }
    else
    {
      picWriter->SetFileName( m_FileName.c_str() );
    }
    picWriter->SetInputImage( input );
    picWriter->Write();
*/    }

    // use the ITK .nrrd Image writer
    if( m_Extension.find(".nrrd") != std::string::npos
        || m_Extension.find(".nii") != std::string::npos
        || m_Extension.find(".nii.gz") != std::string::npos
        )
    {
        std::ostringstream filename;
        filename <<  this->m_FileName.c_str() << this->m_Extension;
        WriteByITK(input, filename.str());
    }
  }
  m_MimeType = "application/MITK.Pic";

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    MITK_INFO << "Could not reset locale " << currLocale;
  }
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
