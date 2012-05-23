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
#ifndef mitkToFTestingCOMMON_H
#define mitkToFTestingCOMMON_H

#include "mitkToFProcessingExports.h"
#include "mitkVector.h"
#include <vnl/vnl_math.h>
#include "mitkToFProcessingCommon.h"
#include "mitkPicFileReader.h"
#include <mitkSurface.h>
#include <mitkSTLFileReader.h>
#include <mitkImageWriter.h>
#include <itksys/SystemTools.hxx>

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk
{
class mitkToFProcessing_EXPORT ToFTestingCommon
{
public:

//creates a test image with given dimension filled with random values
inline static mitk::Image::Pointer CreateTestImage(unsigned int dimX = 100, unsigned int dimY = 100)
{
  typedef itk::Image<float,2> ItkImageType2D;
  typedef itk::ImageRegionIterator<ItkImageType2D> ItkImageRegionIteratorType2D;

  ItkImageType2D::Pointer image = ItkImageType2D::New();
  ItkImageType2D::IndexType start;
  start[0] = 0;
  start[1] = 0;
  ItkImageType2D::SizeType size;
  size[0] = dimX;
  size[1] = dimY;
  ItkImageType2D::RegionType region;
  region.SetSize(size);
  region.SetIndex( start);
  ItkImageType2D::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;

  image->SetRegions( region );
  image->SetSpacing ( spacing );
  image->Allocate();

  //Correlate inten values to PixelIndex//
  ItkImageRegionIteratorType2D imageIterator(image,image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  while (!imageIterator.IsAtEnd())
  {
    double pixelValue = randomGenerator->GetUniformVariate(0.0,1000.0);
    imageIterator.Set(pixelValue);
    ++imageIterator;
  }
  mitk::Image::Pointer mitkImage = mitk::Image::New();
  mitk::CastToMitkImage(image,mitkImage);
  return mitkImage;
}

//loads an image from file
inline static mitk::Image::Pointer LoadImage( std::string filename )
{
  mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
  reader->SetFileName ( filename.c_str() );
  reader->Update();
  if ( reader->GetOutput() == NULL )
    itkGenericExceptionMacro("File "<<filename <<" could not be read!");
  mitk::Image::Pointer image = reader->GetOutput();
  return image;
}

//loads a surface from file
inline static mitk::Surface::Pointer LoadSurface( std::string filename )
{
  mitk::STLFileReader::Pointer reader = mitk::STLFileReader::New();
  reader->SetFileName( filename.c_str() );
  reader->Update();
  if ( reader->GetOutput() == NULL )
    itkGenericExceptionMacro("File "<< filename <<" could not be read!");
  mitk::Surface::Pointer surface = reader->GetOutput();
  return surface;
}

inline static bool SaveImage( mitk::Image* data, std::string filename )
{
  std::string extension = itksys::SystemTools::GetFilenameLastExtension( filename );
  if (extension == ".gz")
  {
    filename.assign( filename, 0, filename.length() - 7 ); // remove last 7 characters (.pic.gz)
  }
  else if (extension == ".pic")
  {
    filename.assign( filename, 0, filename.length() - 4 ); // remove last 4 characters
  }

  try
  {
    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    imageWriter->SetInput(data);

    imageWriter->SetFileName(filename.c_str());
    imageWriter->SetExtension(".pic.gz");
    imageWriter->Write();
  }
  catch ( std::exception& e )
  {
    std::cerr << "Error during attempt to write '" << filename << "'.pic Exception says:" << std::endl;
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

};

}
#endif
