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

#include "mitkOpenCVToMitkImageFilter.h"

#include <itkImportImageFilter.h>
#include <itkRGBPixel.h>
#include <mitkITKImageImport.txx>
#include <itkOpenCVImageBridge.h>
#include <itkImageFileWriter.h>

#include "mitkImageToOpenCVImageFilter.h"

namespace mitk{

  OpenCVToMitkImageFilter::OpenCVToMitkImageFilter()
    : m_OpenCVImage(0)
  {
  }

  OpenCVToMitkImageFilter::~OpenCVToMitkImageFilter()
  {
  }

  void OpenCVToMitkImageFilter::SetOpenCVImage(const IplImage* image)
  {
    this->m_OpenCVImage = image;
    this->m_OpenCVMat.release();
    this->Modified();
  }

  void OpenCVToMitkImageFilter::GenerateData()
  {
    IplImage cvMatIplImage;
    const IplImage* targetImage = 0;
    if(m_OpenCVImage == 0)
    {
      if( m_OpenCVMat.cols == 0 || m_OpenCVMat.rows == 0 )
      {
        MITK_WARN << "Cannot not start filter. OpenCV Image not set.";
        return;
      }
      else
      {
        cvMatIplImage = m_OpenCVMat;
        targetImage = &cvMatIplImage;
      }
    }
    else
      targetImage = m_OpenCVImage;

    // now convert rgb image
    if( (targetImage->depth>=0) && ((unsigned int)targetImage->depth == IPL_DEPTH_8S) && (targetImage->nChannels == 1) )
      m_Image = ConvertIplToMitkImage< char, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_8U && targetImage->nChannels == 1 )
      m_Image = ConvertIplToMitkImage< unsigned char, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_8U && targetImage->nChannels == 3 )
      m_Image = ConvertIplToMitkImage< UCRGBPixelType, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_16U && targetImage->nChannels == 1 )
      m_Image = ConvertIplToMitkImage< unsigned short, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_16U && targetImage->nChannels == 3 )
      m_Image = ConvertIplToMitkImage< USRGBPixelType, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_32F && targetImage->nChannels == 1 )
      m_Image = ConvertIplToMitkImage< float, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_32F && targetImage->nChannels == 3 )
      m_Image = ConvertIplToMitkImage< FloatRGBPixelType , 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_64F && targetImage->nChannels == 1 )
      m_Image = ConvertIplToMitkImage< double, 2>( targetImage );

    else if( targetImage->depth == IPL_DEPTH_64F && targetImage->nChannels == 3 )
      m_Image = ConvertIplToMitkImage< DoubleRGBPixelType , 2>( targetImage );

    else
    {
      MITK_WARN << "Unknown image depth and/or pixel type. Cannot convert OpenCV to MITK image.";
      return;
    }

    //cvReleaseImage(&rgbOpenCVImage);
  }

  ImageSource::OutputImageType* OpenCVToMitkImageFilter::GetOutput()
  {
    return m_Image;
  }

  /********************************************
  * Converting from OpenCV image to ITK Image
  *********************************************/
  template <typename TPixel, unsigned int VImageDimension>
  Image::Pointer mitk::OpenCVToMitkImageFilter::ConvertIplToMitkImage( const IplImage * input )
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;

    typename ImageType::Pointer output = ImageType::New();
    typename ImageType::RegionType region;
    typename ImageType::RegionType::SizeType size;
    typename ImageType::RegionType::IndexType index;
    typename ImageType::SpacingType spacing;
    size.Fill( 1 );
    size[0] = input->width;
    size[1] = input->height;
    index.Fill(0);
    spacing.Fill(1);
    region.SetSize(size);
    region.SetIndex(index);
    output->SetRegions(region);
    output->SetSpacing(spacing);
    output->Allocate();

    // CAVE: The itk openCV bridge seem to NOT correctly copy the image data, hence the call to
    // itk::OpenCVImageBridge::IplImageToITKImage<ImageType>() is simply used to initialize the itk image
    // and in the next step the image data are copied by hand!

    if(input->nChannels == 3) // these are RGB images and need to be set to BGR before conversion!
    {
      output = itk::OpenCVImageBridge::IplImageToITKImage<ImageType>(input);
    }
    else
    {
      memcpy((void*) output->GetBufferPointer(), (void*) input->imageDataOrigin,
        input->width*input->height*sizeof(TPixel));
    }

    Image::Pointer mitkImage = Image::New();
    mitkImage = GrabItkImageMemory(output);

    return mitkImage;
  }


  void OpenCVToMitkImageFilter::SetOpenCVMat(const cv::Mat &image)
  {
    m_OpenCVMat = image;
    m_OpenCVImage = NULL;
    this->Modified();
  }

} // end namespace mitk
