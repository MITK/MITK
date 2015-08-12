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

#include "mitkLabelSetImageConverter.h"

// mitk includes
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>

// itk includes
#include "itkImage.h"
#include "itkVectorImage.h"
#include <itkComposeImageFilter.h>
#include <itkVectorIndexSelectionCastImageFilter.h>
#include "itkImageDuplicator.h"

template < typename TPixel, unsigned int VImageDimension >
void VectorOfMitkImagesToMitkVectorImage(const itk::Image<TPixel, VImageDimension>* /*source*/, mitk::Image::Pointer &output, mitk::LabelSetImage::ConstPointer input)
{
  typedef itk::ComposeImageFilter< itk::Image<TPixel, VImageDimension> > ComposeFilterType;

  unsigned int numberOfLayers = input->GetNumberOfLayers();
  // 2015/07/01 At the time of writing MITK has problems with mitk::Images encapsulating itk::VectorImage
  // if the vector length is less than 2, which might very well happen for segmentations.
  if ( numberOfLayers > 1 )
  { // if we have only one image we do not need to create a vector
    typename ComposeFilterType::Pointer vectorImageComposer = ComposeFilterType::New();

    unsigned int activeLayer = input->GetActiveLayer();
    for (unsigned int layer(0); layer < numberOfLayers; layer++)
    {
      typename itk::Image<TPixel, VImageDimension>::Pointer itkCurrentLayer;
      // for the active layer use the current state not the saved one in the vector
      if (layer == activeLayer)
      {
        mitk::CastToItkImage(dynamic_cast<const mitk::Image*>(input.GetPointer()), itkCurrentLayer);
      }
      else
      {
        mitk::CastToItkImage(input->GetLayerImage(layer), itkCurrentLayer);
      }

      vectorImageComposer->SetInput(layer, itkCurrentLayer);
    }

    try
    {
      vectorImageComposer->Update();
    }
    catch (const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Caught exception while updating compose filter: " << e.what();
    }

    output = mitk::GrabItkImageMemory(vectorImageComposer->GetOutput());
  }
  else
  {
    // we want the clone to be a mitk::Image, not a mitk::LabelSetImage
    typename itk::Image<TPixel, VImageDimension>::Pointer itkCurrentLayer;
    mitk::CastToItkImage(dynamic_cast<const mitk::Image*>(input.GetPointer()), itkCurrentLayer);
    typedef itk::ImageDuplicator< itk::Image<TPixel, VImageDimension> > DuplicatorType;
    typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(itkCurrentLayer);
    duplicator->Update();

    output = mitk::GrabItkImageMemory(duplicator->GetOutput());
  }
}

mitk::Image::Pointer mitk::LabelSetImageConverter::ConvertLabelSetImageToImage(const mitk::LabelSetImage::ConstPointer input)
{
  unsigned int numberOfLayers = input->GetNumberOfLayers();

  if (numberOfLayers == 0)
  {
    mitkThrow() << "Tried to convert LabelSetImage without layers";
  }

  mitk::Image::Pointer output;
  AccessByItk_2(input->GetLayerImage(0), VectorOfMitkImagesToMitkVectorImage, output, input);

  return output;
}

template<typename TPixel, unsigned int VDimensions>
void MitkImageToMitkLabelSetImage(itk::VectorImage< TPixel, VDimensions> * source, mitk::LabelSetImage::Pointer &output)
{
  typedef itk::VectorImage< TPixel, VDimensions > VectorImageType;
  typedef itk::Image< TPixel, VDimensions > ImageType;
  typedef itk::VectorIndexSelectionCastImageFilter< VectorImageType, ImageType > VectorIndexSelectorType;

  unsigned int numberOfComponents = source->GetVectorLength();

  if (numberOfComponents < 1)
  {
    mitkThrow() << "At least one Component is required.";
  }

  typename VectorIndexSelectorType::Pointer vectorIndexSelector = VectorIndexSelectorType::New();

  vectorIndexSelector->SetIndex(0);
  vectorIndexSelector->SetInput(source);
  vectorIndexSelector->Update();

  mitk::Image::Pointer tempImage;
  mitk::CastToMitkImage(vectorIndexSelector->GetOutput(), tempImage);

  output = mitk::LabelSetImage::New();
  output->InitializeByLabeledImage(tempImage);

  for (unsigned int layer = 1; layer < numberOfComponents; ++layer)
  {
    typename VectorIndexSelectorType::Pointer vectorIndexSelectorLoop = VectorIndexSelectorType::New();
    vectorIndexSelectorLoop->SetIndex(layer);
    vectorIndexSelector->SetInput(source);
    vectorIndexSelector->Update();

    mitk::Image::Pointer loopImage;
    mitk::CastToMitkImage(vectorIndexSelector->GetOutput(), loopImage);

    output->AddLayer(loopImage);
  }

}

mitk::LabelSetImage::Pointer mitk::LabelSetImageConverter::ConvertImageToLabelSetImage(const mitk::Image::Pointer input)
{
  mitk::LabelSetImage::Pointer output;

  if (input->GetChannelDescriptor().GetPixelType().GetPixelType() == itk::ImageIOBase::VECTOR)
  {
    AccessVectorPixelTypeByItk_n(input, MitkImageToMitkLabelSetImage, (output));
  }
  else
  {
    output = mitk::LabelSetImage::New();
    output->InitializeByLabeledImage( input );
  }

  return output;
}
