/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkDiffImageApplier.h"

#include "mitkImageCast.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkRenderingManager.h"
#include "mitkSegmentationInterpolation.h"

#include <itkImageSliceIteratorWithIndex.h>
#include <itkImageRegionConstIterator.h>

mitk::DiffImageApplier::DiffImageApplier()
{
}

mitk::DiffImageApplier::~DiffImageApplier()
{
}
    
void mitk::DiffImageApplier::ExecuteOperation( Operation* operation )
{
  ApplyDiffImageOperation* imageOperation = dynamic_cast<ApplyDiffImageOperation*>( operation );
  if (    imageOperation                        // we actually have the kind of operation that we can handle
       && imageOperation->IsImageStillValid() ) // AND the image is not yet deleted
  {
    m_Image = imageOperation->GetImage();
    m_SliceDifferenceImage = imageOperation->GetDiffImage();

    m_Factor = imageOperation->GetFactor();

    if ( m_SliceDifferenceImage->GetDimension() == 2 )
    {
      m_SliceIndex = imageOperation->GetSliceIndex();
      m_SliceDimension = imageOperation->GetSliceDimension();
      switch (m_SliceDimension)
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

      if ( m_SliceDifferenceImage->GetDimension() != 2 || m_Image->GetDimension() != 3 ||
           m_SliceDifferenceImage->GetDimension(0) != m_Image->GetDimension(m_Dimension0) ||
           m_SliceDifferenceImage->GetDimension(1) != m_Image->GetDimension(m_Dimension1) ||
           m_SliceIndex >= m_Image->GetDimension(m_SliceDimension)
         )
      {
       itkExceptionMacro("Slice and image dimensions differ or slice index is too large. Sorry, cannot work like this.");
       return;
      }

       // this will do a long long if/else to find out both pixel types
      AccessFixedDimensionByItk( m_Image, ItkImageSwitch2DDiff, 3 );

      if ( m_Factor == 1 || m_Factor == -1 )
      {
        if ( m_Factor == -1 )
        {
          // multiply diff pixels by factor and then send this diff slice
          AccessFixedDimensionByItk( m_SliceDifferenceImage, ItkInvertPixelValues, 2 );
        }

        // just send the diff to SegmentationInterpolation
        SegmentationInterpolation* interpolator = SegmentationInterpolation::InterpolatorForImage( m_Image );
        if (interpolator)
        {
          interpolator->BlockModified(true);
          interpolator->SetChangedSlice( m_SliceDifferenceImage, m_SliceDimension, m_SliceIndex );
        }
        
        m_Image->Modified();

        if (interpolator)
        {
          interpolator->BlockModified(false);
        }

        if ( m_Factor == -1 ) // return to normal values
        {
          AccessFixedDimensionByItk( m_SliceDifferenceImage, ItkInvertPixelValues, 2 );
        }
      }
      else // no trivial case, too lazy to do something else
      {
        m_Image->Modified(); // check if interpolation is called. prefer to send diff directly
      }

      RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if ( m_SliceDifferenceImage->GetDimension() == 3 )
    { 
      // ...
      if (  m_SliceDifferenceImage->GetDimension(0) != m_Image->GetDimension(0) ||
            m_SliceDifferenceImage->GetDimension(1) != m_Image->GetDimension(1) ||
            m_SliceDifferenceImage->GetDimension(2) != m_Image->GetDimension(2) 
         )
        {
         itkExceptionMacro("Diff image size differs from original image size. Sorry, cannot work like this.");
         return;
        }

      // this will do a long long if/else to find out both pixel types
      AccessFixedDimensionByItk( m_Image, ItkImageSwitch3DDiff, 3 );

      if ( m_Factor == 1 || m_Factor == -1 )
      {
        if ( m_Factor == -1 )
        {
          // multiply diff pixels by factor and then send this diff slice
          AccessFixedDimensionByItk( m_SliceDifferenceImage, ItkInvertPixelValues, 3 );
        }

        // just send the diff to SegmentationInterpolation
        SegmentationInterpolation* interpolator = SegmentationInterpolation::InterpolatorForImage( m_Image );
        if (interpolator)
        {
          interpolator->BlockModified(true);
          interpolator->SetChangedVolume( m_SliceDifferenceImage );
        }

        m_Image->Modified();

        if (interpolator)
        {
          interpolator->BlockModified(false);
        }

        if ( m_Factor == -1 ) // return to normal values
        {
          AccessFixedDimensionByItk( m_SliceDifferenceImage, ItkInvertPixelValues, 3 );
        }
      }
      else // no trivial case, too lazy to do something else
      {
        m_Image->Modified(); // check if interpolation is called. prefer to send diff directly
      }
      
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    {
     itkExceptionMacro("Diff image must be 2D or 3D. Sorry, cannot work like this.");
     return;
    }
  }

  m_Image = NULL;
  m_SliceDifferenceImage = NULL;
}

mitk::DiffImageApplier* mitk::DiffImageApplier::GetInstanceForUndo()
{
  static DiffImageApplier::Pointer s_Instance = DiffImageApplier::New();

  return s_Instance;
}

// basically copied from mitk/Core/Algorithms/mitkImageAccessByItk.h
#define myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension, itkimage2)            \
  if ( typeId == typeid(pixeltype) )                                                    \
{                                                                                        \
    typedef itk::Image<pixeltype, dimension> ImageType;                                   \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                    \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                 \
    imagetoitk->SetInput(mitkImage);                                                     \
    imagetoitk->Update();                                                               \
    itkImageTypeFunction(imagetoitk->GetOutput(), itkimage2);                          \
}                                                              


#define myMITKDiffImageApplierFilterAccessAllTypesByItk(mitkImage, itkImageTypeFunction,       dimension, itkimage2)    \
{                                                                                                                           \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, double,         dimension, itkimage2) else   \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, float,          dimension, itkimage2) else    \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, int,            dimension, itkimage2) else     \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned int,   dimension, itkimage2) else      \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, short,          dimension, itkimage2) else     \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned short, dimension, itkimage2) else    \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, char,           dimension, itkimage2) else   \
    myMITKDiffImageApplierFilterAccessByItk(mitkImage, itkImageTypeFunction, unsigned char,  dimension, itkimage2)       \
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::DiffImageApplier::ItkImageSwitch2DDiff( itk::Image<TPixel,VImageDimension>* itkImage )
{
  const std::type_info& typeId=*(m_SliceDifferenceImage->GetPixelType().GetTypeId());

  myMITKDiffImageApplierFilterAccessAllTypesByItk( m_SliceDifferenceImage, ItkImageProcessing2DDiff, 2, itkImage );
}


template<typename TPixel, unsigned int VImageDimension>
void mitk::DiffImageApplier::ItkImageSwitch3DDiff( itk::Image<TPixel,VImageDimension>* itkImage )
{
  const std::type_info& typeId=*(m_SliceDifferenceImage->GetPixelType().GetTypeId());

  myMITKDiffImageApplierFilterAccessAllTypesByItk( m_SliceDifferenceImage, ItkImageProcessing3DDiff, 3, itkImage );
}

template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::DiffImageApplier::ItkImageProcessing2DDiff( itk::Image<TPixel1,VImageDimension1>* diffImage, itk::Image<TPixel2,VImageDimension2>* outputImage )
{
  typedef itk::Image<TPixel1, VImageDimension1> DiffImageType;
  typedef itk::Image<TPixel2, VImageDimension2> VolumeImageType;

  typedef itk::ImageSliceIteratorWithIndex< VolumeImageType > OutputSliceIteratorType;
  typedef itk::ImageRegionConstIterator< DiffImageType >      DiffSliceIteratorType;

  typename VolumeImageType::RegionType            sliceInVolumeRegion;

  sliceInVolumeRegion = outputImage->GetLargestPossibleRegion();
  sliceInVolumeRegion.SetSize( m_SliceDimension, 1 );             // just one slice
  sliceInVolumeRegion.SetIndex( m_SliceDimension, m_SliceIndex ); // exactly this slice, please
  
  OutputSliceIteratorType outputIterator( outputImage, sliceInVolumeRegion );
  outputIterator.SetFirstDirection(m_Dimension0);
  outputIterator.SetSecondDirection(m_Dimension1);

  DiffSliceIteratorType diffIterator( diffImage, diffImage->GetLargestPossibleRegion() );

  // iterate over output slice (and over input slice simultaneously)
  outputIterator.GoToBegin();
  diffIterator.GoToBegin();
  while ( !outputIterator.IsAtEnd() )
  {
    while ( !outputIterator.IsAtEndOfSlice() )
    {
      while ( !outputIterator.IsAtEndOfLine() )
      {
        TPixel2 newValue = outputIterator.Get() + (TPixel2) ((double)diffIterator.Get() * m_Factor);
        outputIterator.Set( newValue );
        ++outputIterator; 
        ++diffIterator; 
      }
      outputIterator.NextLine();
    }
    outputIterator.NextSlice();
  }
}


template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::DiffImageApplier::ItkImageProcessing3DDiff( itk::Image<TPixel1,VImageDimension1>* diffImage, itk::Image<TPixel2,VImageDimension2>* outputImage )
{
  typedef itk::Image<TPixel1, VImageDimension1> DiffImageType;
  typedef itk::Image<TPixel2, VImageDimension2> VolumeImageType;

  typedef itk::ImageRegionIterator< VolumeImageType >      OutputSliceIteratorType;
  typedef itk::ImageRegionConstIterator< DiffImageType >   DiffSliceIteratorType;

  OutputSliceIteratorType outputIterator( outputImage, outputImage->GetLargestPossibleRegion() );
  DiffSliceIteratorType diffIterator( diffImage, diffImage->GetLargestPossibleRegion() );

  // iterate over output slice (and over input slice simultaneously)
  outputIterator.GoToBegin();
  diffIterator.GoToBegin();
  while ( !outputIterator.IsAtEnd() )
  {
    TPixel2 newValue = outputIterator.Get() + (TPixel2) ((double)diffIterator.Get() * m_Factor);
    outputIterator.Set( newValue );
    ++outputIterator; 
    ++diffIterator; 
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::DiffImageApplier::ItkInvertPixelValues( itk::Image<TPixel,VImageDimension>* itkImage )
{
  typedef itk::ImageRegionIterator< itk::Image<TPixel,VImageDimension> >      IteratorType;
  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );

  iter.GoToBegin();
  while ( !iter.IsAtEnd() )
  {
    iter.Set( -( iter.Get() ) );
    ++iter;
  }
}

