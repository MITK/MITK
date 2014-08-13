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

#include "mitkOverwriteSliceImageFilter.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkSegmentationInterpolationController.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkOperationEvent.h"
#include "mitkInteractionConst.h"
#include "mitkUndoController.h"
#include "mitkDiffImageApplier.h"
#include "mitkImageTimeSelector.h"

#include <itkImageSliceIteratorWithIndex.h>
#include <itkImageRegionIterator.h>

mitk::OverwriteSliceImageFilter::OverwriteSliceImageFilter()
:m_SliceIndex(0),
 m_SliceDimension(0),
 m_TimeStep(0),
 m_Dimension0(0),
 m_Dimension1(1),
 m_CreateUndoInformation(false)
{
  MITK_WARN << "Class is deprecated! Use mitkVtkImageOverwrite instead.";
}

mitk::OverwriteSliceImageFilter::~OverwriteSliceImageFilter()
{
}

void mitk::OverwriteSliceImageFilter::GenerateData()
{
  //
  // this is the place to implement the major part of undo functionality (bug #491)
  // here we have to create undo/do operations
  //
  // WHO is the operation actor? This object may not be destroyed ever (design of undo stack)!
  // -> some singleton method of this filter?
  //
  // neccessary additional objects:
  //  - something that executes the operations
  //  - the operation class (must hold a binary diff or something)
  //  - observer commands to know when the image is deleted (no further action then, perhaps even remove the operations from the undo stack)
  //
  Image::ConstPointer input = ImageToImageFilter::GetInput(0);
  Image::Pointer input3D = ImageToImageFilter::GetInput(0);

  Image::ConstPointer slice = m_SliceImage;

  if ( input.IsNull() || slice.IsNull() ) return;

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

  if ( slice->GetDimension() < 2 || input->GetDimension() > 4 ||
       slice->GetDimension(0) != input->GetDimension(m_Dimension0) ||
       slice->GetDimension(1) != input->GetDimension(m_Dimension1) ||
       m_SliceIndex >= input->GetDimension(m_SliceDimension)
     )
  {
   itkExceptionMacro("Slice and image dimensions differ or slice index is too large. Sorry, cannot work like this.");
   return;
  }

  if ( input->GetDimension() == 4 )
  {
    ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput( input );
    timeSelector->SetTimeNr( m_TimeStep );
    timeSelector->UpdateLargestPossibleRegion();
    input3D = timeSelector->GetOutput();
  }

  if ( m_SliceDifferenceImage.IsNull() ||
       m_SliceDifferenceImage->GetDimension(0) != m_SliceImage->GetDimension(0) ||
       m_SliceDifferenceImage->GetDimension(1) != m_SliceImage->GetDimension(1) )
  {
    m_SliceDifferenceImage = mitk::Image::New();
    mitk::PixelType pixelType( mitk::MakeScalarPixelType<short signed int>() );
    m_SliceDifferenceImage->Initialize( pixelType, 2, m_SliceImage->GetDimensions() );
  }

  //MITK_INFO << "Overwriting slice " << m_SliceIndex << " in dimension " << m_SliceDimension << " at time step " << m_TimeStep << std::endl;
  // this will do a long long if/else to find out both pixel types
  AccessFixedDimensionByItk( input3D, ItkImageSwitch, 3 );

  SegmentationInterpolationController* interpolator = SegmentationInterpolationController::InterpolatorForImage( input );
  if (interpolator)
  {
    interpolator->BlockModified(true);
    interpolator->SetChangedSlice( m_SliceDifferenceImage, m_SliceDimension, m_SliceIndex, m_TimeStep );
  }

  if ( m_CreateUndoInformation )
  {
    // create do/undo operations (we don't execute the doOp here, because it has already been executed during calculation of the diff image
    ApplyDiffImageOperation* doOp = new ApplyDiffImageOperation( OpTEST, const_cast<Image*>(input.GetPointer()), m_SliceDifferenceImage, m_TimeStep, m_SliceDimension, m_SliceIndex );
    ApplyDiffImageOperation* undoOp = new ApplyDiffImageOperation( OpTEST, const_cast<Image*>(input.GetPointer()), m_SliceDifferenceImage, m_TimeStep, m_SliceDimension, m_SliceIndex );
    undoOp->SetFactor( -1.0 );
    OperationEvent* undoStackItem = new OperationEvent( DiffImageApplier::GetInstanceForUndo(), doOp, undoOp, this->EventDescription(m_SliceDimension, m_SliceIndex, m_TimeStep) );
    UndoController::GetCurrentUndoModel()->SetOperationEvent( undoStackItem );
  }

  // this image is modified (good to know for the renderer)
  input->Modified();

  if (interpolator)
  {
    interpolator->BlockModified(false);
  }
}

// basically copied from mitk/Core/Algorithms/mitkImageAccessByItk.h
#define myMITKOverwriteSliceImageFilterAccessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension, itkimage2)            \
  if ( typeId == MapPixelComponentType<pixeltype>::value )                                                    \
{                                                                                        \
    typedef itk::Image<pixeltype, dimension> ImageType;                                   \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                    \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                 \
    const mitk::Image* constImage = mitkImage;                                           \
    mitk::Image* nonConstImage = const_cast<mitk::Image*>(constImage);                   \
    nonConstImage->Update();                                                             \
    imagetoitk->SetInput(nonConstImage);                                                     \
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


template<typename TPixel, unsigned int VImageDimension>
void mitk::OverwriteSliceImageFilter::ItkImageSwitch( itk::Image<TPixel,VImageDimension>* itkImage )
{
  const int typeId=m_SliceImage->GetPixelType().GetComponentType();

  myMITKOverwriteSliceImageFilterAccessAllTypesByItk( m_SliceImage, ItkImageProcessing, 2, itkImage );
}

template<typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::OverwriteSliceImageFilter::ItkImageProcessing( const itk::Image<TPixel1,VImageDimension1>* inputImage, itk::Image<TPixel2,VImageDimension2>* outputImage )
{
  typedef itk::Image<TPixel1, VImageDimension1> SliceImageType;
  typedef itk::Image<short signed int, VImageDimension1> DiffImageType;
  typedef itk::Image<TPixel2, VImageDimension2> VolumeImageType;

  typedef itk::ImageSliceIteratorWithIndex< VolumeImageType > OutputSliceIteratorType;
  typedef itk::ImageRegionConstIterator< SliceImageType >     InputSliceIteratorType;
  typedef itk::ImageRegionIterator< DiffImageType >     DiffSliceIteratorType;

  typename VolumeImageType::RegionType            sliceInVolumeRegion;

  sliceInVolumeRegion = outputImage->GetLargestPossibleRegion();
  sliceInVolumeRegion.SetSize( m_SliceDimension, 1 );             // just one slice
  sliceInVolumeRegion.SetIndex( m_SliceDimension, m_SliceIndex ); // exactly this slice, please

  OutputSliceIteratorType outputIterator( outputImage, sliceInVolumeRegion );
  outputIterator.SetFirstDirection(m_Dimension0);
  outputIterator.SetSecondDirection(m_Dimension1);

  InputSliceIteratorType inputIterator( inputImage, inputImage->GetLargestPossibleRegion() );

  typename DiffImageType::Pointer diffImage;
  CastToItkImage( m_SliceDifferenceImage, diffImage );
  DiffSliceIteratorType diffIterator( diffImage, diffImage->GetLargestPossibleRegion() );

  // iterate over output slice (and over input slice simultaneously)
  outputIterator.GoToBegin();
  inputIterator.GoToBegin();
  diffIterator.GoToBegin();
  while ( !outputIterator.IsAtEnd() )
  {
    while ( !outputIterator.IsAtEndOfSlice() )
    {
      while ( !outputIterator.IsAtEndOfLine() )
      {
        diffIterator.Set( static_cast<short signed int>(inputIterator.Get() - outputIterator.Get()) ); // oh oh, not good for bigger values
        outputIterator.Set( (TPixel2) inputIterator.Get() );
        ++outputIterator;
        ++inputIterator;
        ++diffIterator;
      }
      outputIterator.NextLine();
    }
    outputIterator.NextSlice();
  }
}

std::string mitk::OverwriteSliceImageFilter::EventDescription( unsigned int sliceDimension, unsigned int sliceIndex, unsigned int timeStep )
{
  std::stringstream s;

  s << "Changed slice (";

  switch (sliceDimension)
  {
    default:
    case 2:
      s << "T";
      break;
    case 1:
      s << "C";
      break;
    case 0:
      s << "S";
      break;
  }

  s << " " << sliceIndex << " " << timeStep << ")";

  return s.str();
}
