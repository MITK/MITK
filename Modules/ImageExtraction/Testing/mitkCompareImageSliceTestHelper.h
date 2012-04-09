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

#ifndef mitkCompareImageSliceTestHelperhincluded
#define mitkCompareImageSliceTestHelperhincluded


#include "mitkImageCast.h"
#include <mitkImageAccessByItk.h>
#include <itkImageSliceConstIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>

// copied from mitk/Core/Algorithms/mitkOverwriteSliceImageFilter.cpp
// basically copied from mitk/Core/Algorithms/mitkImageAccessByItk.h
#define myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension, itkimage2)            \
  if ( typeId == typeid(pixeltype) )                                                    \
{                                                                                        \
    typedef itk::Image<pixeltype, dimension> ImageType;                                   \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                    \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                 \
    imagetoitk->SetInput(mitkImage);                                                     \
    imagetoitk->Update();                                                               \
    itkImageTypeFunction(imagetoitk->GetOutput(), itkimage2);                          \
}                                                              

#define myMITKOverwriteSliceImageFilterAccessAllTypesByItk(mitkImage, itkImageTypeFunction,       dimension, itkimage2)    \
{                                                                                                                           \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, double,         dimension, itkimage2) else   \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, float,          dimension, itkimage2) else    \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, int,            dimension, itkimage2) else     \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned int,   dimension, itkimage2) else      \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, short,          dimension, itkimage2) else     \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned short, dimension, itkimage2) else    \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, char,           dimension, itkimage2) else   \
    myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned char,  dimension, itkimage2)       \
}


class CompareImageSliceTestHelper
{
  private:

/* variables to be used by CompareSlice only */  
static unsigned int m_Dimension0;
static unsigned int m_Dimension1;
static unsigned int m_SliceDimension;
static unsigned int m_SliceIndex;
static bool m_ComparisonResult;
static mitk::Image* m_SliceImage;
  
  public:

template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
static void ItkImageCompare( itk::Image<TPixel1,VImageDimension1>* inputImage, itk::Image<TPixel2,VImageDimension2>* outputImage )
{
m_ComparisonResult = false;
  
  typedef itk::Image<TPixel1, VImageDimension1> SliceImageType;
  typedef itk::Image<TPixel2, VImageDimension2> VolumeImageType;

  typedef itk::ImageSliceConstIteratorWithIndex< VolumeImageType > OutputSliceIteratorType;
  typedef itk::ImageRegionConstIterator< SliceImageType >     InputSliceIteratorType;

  typename VolumeImageType::RegionType            sliceInVolumeRegion;

  sliceInVolumeRegion = outputImage->GetLargestPossibleRegion();
  sliceInVolumeRegion.SetSize( m_SliceDimension, 1 );             // just one slice
  sliceInVolumeRegion.SetIndex( m_SliceDimension, m_SliceIndex ); // exactly this slice, please
  
  OutputSliceIteratorType outputIterator( outputImage, sliceInVolumeRegion );
  outputIterator.SetFirstDirection(m_Dimension0);
  outputIterator.SetSecondDirection(m_Dimension1);

  InputSliceIteratorType inputIterator( inputImage, inputImage->GetLargestPossibleRegion() );

  // iterate over output slice (and over input slice simultaneously)
  outputIterator.GoToBegin();
  inputIterator.GoToBegin();
  while ( !outputIterator.IsAtEnd() )
  {
    while ( !outputIterator.IsAtEndOfSlice() )
    {
      while ( !outputIterator.IsAtEndOfLine() )
      {
        m_ComparisonResult = outputIterator.Get() == (TPixel2) inputIterator.Get();
        if (!m_ComparisonResult) return; // return on first mismatch
        ++outputIterator; 
        ++inputIterator; 
      }
      outputIterator.NextLine();
    }
    outputIterator.NextSlice();
  }
}
    
template<typename TPixel, unsigned int VImageDimension>
static void ItkImageSwitch( itk::Image<TPixel,VImageDimension>* itkImage )
{
  AccessFixedDimensionByItk_1( m_SliceImage, ItkImageCompare, 2, itkImage )
  //myMITKOverwriteSliceImageFilterAccessAllTypesByItk( m_SliceImage, ItkImageCompare, 2, itkImage );
}


static bool CompareSlice( mitk::Image* image, unsigned int sliceDimension, unsigned int sliceIndex, mitk::Image* slice )
{
  if ( !image || ! slice ) return false;

  switch (sliceDimension)
  {
    default:
    case 2:
      m_Dimension0 = 0;
      m_Dimension1 = 1;
      break;
    case 1:
      m_Dimension0 = 0;
      m_Dimension1 = 2;
      break;
    case 0:
      m_Dimension0 = 1;
      m_Dimension1 = 2;
      break;
  }

  if ( slice->GetDimension() != 2 || image->GetDimension() != 3 ||
       slice->GetDimension(0) != image->GetDimension(m_Dimension0) ||
       slice->GetDimension(1) != image->GetDimension(m_Dimension1) )
  {
   std::cerr << "Slice and image dimensions differ. Sorry, cannot work like this." << std::endl;
   return false;
  }

  // this will do a long long if/else to find out both pixel typesA
  m_SliceImage = slice;
  m_SliceIndex = sliceIndex;
  m_SliceDimension = sliceDimension;
  m_ComparisonResult = false;
  AccessFixedDimensionByItk( image, ItkImageSwitch, 3 );

  return m_ComparisonResult;
}

}; // end class
  


#endif
