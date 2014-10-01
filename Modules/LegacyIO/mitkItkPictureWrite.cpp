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

#include <MitkLegacyIOExports.h>

#include "mitkItkPictureWrite.h"
#include <mitkInstantiateAccessFunctions.h>

#include <itkNumericSeriesFileNames.h>
#include <itkImageSeriesWriter.h>
#include <itkRescaleIntensityImageFilter.h>

#include <itkRGBAPixel.h>

/** Set the filenames to the specified writer in dependace on the number of images passed in */
template< class WriterType >
void SetOutputNames( typename WriterType::Pointer writer, const std::string& baseFileName, unsigned int numberOfImages )
{
  if( numberOfImages > 1 )
  {
    itk::NumericSeriesFileNames::Pointer numericFileNameWriter = itk::NumericSeriesFileNames::New();

    std::string finalFileName = baseFileName;
    std::string::size_type pos = baseFileName.find_last_of(".",baseFileName.length()-1);
    if(pos==std::string::npos)
      finalFileName.append(".%d.png");
    else
      finalFileName.insert(pos,".%d");

    MITK_DEBUG << "Filename: " << finalFileName;

    numericFileNameWriter->SetEndIndex(numberOfImages);
    numericFileNameWriter->SetSeriesFormat(finalFileName.c_str());
    numericFileNameWriter->Modified();
    writer->SetFileNames(numericFileNameWriter->GetFileNames());
  }
  // if the given image is an 2D-png image, do not use the numericFileNameWriter
  // to generate the name, since it alters the fileName given as parameter
  else
  {
    writer->SetFileName( baseFileName.c_str() );
  }
}

template < typename TPixel, unsigned int VImageDimension >
void _mitkItkPictureWrite(itk::Image< TPixel, VImageDimension >* itkImage, const std::string& fileName)
{
  typedef itk::Image< TPixel, VImageDimension > TImageType;

  typedef itk::Image<unsigned char,3> UCharOutputImage3DType;
  typedef itk::Image<unsigned short,3> ShortOutputImage3DType;
  typedef itk::Image<unsigned char,2> OutputImage2D_8bitType;
  typedef itk::Image<unsigned short,2> OutputImage2D_16bitType;

  typedef itk::ImageSeriesWriter<UCharOutputImage3DType, OutputImage2D_8bitType> UCharWriterType;
  typedef itk::ImageSeriesWriter<ShortOutputImage3DType, OutputImage2D_16bitType> ShortWriterType;

  typedef itk::RescaleIntensityImageFilter<TImageType, UCharOutputImage3DType> UCharRescalerFilterType;
  typedef itk::RescaleIntensityImageFilter<TImageType, ShortOutputImage3DType> ShortRescalerFilterType;

  // get the size info
  size_t inputTypeSize = sizeof(TPixel);
  size_t supportedOutputMaxSize = 1; // default value 8bit

  // the PNG and TIFF formats can handle up-to 16-bit images
  if( fileName.find(".png") != std::string::npos || fileName.find(".tif") != std::string::npos )
  {
    supportedOutputMaxSize = 2;
  }

  // get the dimension info
  unsigned int numberOfImages = 1;
  if( itkImage->GetImageDimension() > 2)
    numberOfImages = itkImage->GetLargestPossibleRegion().GetSize()[2];

  typename ShortRescalerFilterType::Pointer sh_rescaler = ShortRescalerFilterType::New();
  sh_rescaler->SetInput( itkImage );
  sh_rescaler->SetOutputMinimum( 0 );
  sh_rescaler->SetOutputMaximum( 65535 );

  typename UCharRescalerFilterType::Pointer rescaler = UCharRescalerFilterType::New();
  rescaler->SetInput( itkImage );
  rescaler->SetOutputMinimum( 0 );
  rescaler->SetOutputMaximum( 255 );

  try
  {
    // input is 8 bit
    if( inputTypeSize == 1)
    {
      UCharWriterType::Pointer writer = UCharWriterType::New();
      SetOutputNames<UCharWriterType>( writer, fileName, numberOfImages );
      writer->SetInput( rescaler->GetOutput() );
      writer->Update();
    }
    // input pixel type is 16bit -> writer can handle 16bit images
    else if ( inputTypeSize == supportedOutputMaxSize && supportedOutputMaxSize == 2 )
    {
      ShortWriterType::Pointer writer = ShortWriterType::New();
      SetOutputNames<ShortWriterType>( writer, fileName, numberOfImages );
      writer->SetInput( sh_rescaler->GetOutput() );
      writer->Update();
    }
    // rescaling input to maximum of supported format
    else
    {
      if( supportedOutputMaxSize == 2)
      {
        typename ShortWriterType::Pointer writer = ShortWriterType::New();
        SetOutputNames<ShortWriterType>( writer, fileName, numberOfImages );
        writer->SetInput(sh_rescaler->GetOutput() );
        writer->Update();
      }
      else
      {
        typename UCharWriterType::Pointer writer = UCharWriterType::New();
        SetOutputNames<UCharWriterType>( writer, fileName, numberOfImages );
        writer->SetInput( rescaler->GetOutput() );
        writer->Update();
      }
    }
  }
  catch(const itk::ExceptionObject& e)
  {
    MITK_ERROR << "ITK Exception occured: " << e.what();
    mitkThrow() << "Caught ITK exception while writing image with scalar type \n" << e.what();
  }
}

template < typename TPixel, unsigned int VImageDimension >
void _mitkItkPictureWriteComposite(itk::Image< TPixel, VImageDimension >* itkImage, const std::string& fileName)
{
  typedef itk::Image< TPixel, VImageDimension > TImageType;
  typedef itk::Image< TPixel, 2 > TImageType2D;

  typedef itk::ImageSeriesWriter< TImageType, TImageType2D > WriterType;
  typename WriterType::Pointer writer = WriterType::New();

  // get the dimension info
  unsigned int numberOfImages = 1;
  if( itkImage->GetImageDimension() > 2)
    numberOfImages = itkImage->GetLargestPossibleRegion().GetSize()[2];

  // create output name(s)
  SetOutputNames<WriterType>( writer, fileName, numberOfImages );

  writer->SetInput( itkImage );
  try
  {
    writer->Update();
  }
  catch(const itk::ExceptionObject &e)
  {
    MITK_ERROR << "ITK Exception occured: " << e.what();
    mitkThrow() << "Caught ITK exception while writing image with composite type \n" << e.what();
  }
}

#define InstantiateAccessFunction__mitkItkPictureWrite(pixelType, dim) \
  template MitkLegacyIO_EXPORT void _mitkItkPictureWrite(itk::Image<pixelType,dim>*, const std::string&);

#define InstantiateAccessFunction__mitkItkPictureWriteComposite(pixelType, dim) \
  template MitkLegacyIO_EXPORT void _mitkItkPictureWriteComposite(itk::Image<pixelType,dim>*, const std::string&);

InstantiateAccessFunction(_mitkItkPictureWrite)

InstantiateAccessFunctionForFixedPixelType( _mitkItkPictureWriteComposite, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ)
