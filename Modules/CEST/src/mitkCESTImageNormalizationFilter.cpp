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

#include "mitkCESTImageNormalizationFilter.h"

#include <mitkCustomTagParser.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <boost/algorithm/string.hpp>

mitk::CESTImageNormalizationFilter::CESTImageNormalizationFilter()
{
}

mitk::CESTImageNormalizationFilter::~CESTImageNormalizationFilter()
{
}

void mitk::CESTImageNormalizationFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput(0);
  if ((inputImage->GetDimension() != 4))
  {
    mitkThrow() << "mitk::CESTImageNormalizationFilter:GenerateData works only with 4D images, sorry.";
    return;
  }

  auto resultMitkImage = this->GetOutput();
  AccessFixedDimensionByItk(inputImage, NormalizeTimeSteps, 4);

  auto originalTimeGeometry = this->GetInput()->GetTimeGeometry();
  auto resultTimeGeometry = mitk::ProportionalTimeGeometry::New();

  unsigned int numberOfNonM0s = m_NonM0Indices.size();
  resultTimeGeometry->Expand(numberOfNonM0s);

  for (unsigned int index = 0; index < numberOfNonM0s; ++index)
  {
    resultTimeGeometry->SetTimeStepGeometry(originalTimeGeometry->GetGeometryCloneForTimeStep(m_NonM0Indices.at(index)), index);
  }
  resultMitkImage->SetTimeGeometry(resultTimeGeometry);

  resultMitkImage->SetPropertyList(this->GetInput()->GetPropertyList()->Clone());
  resultMitkImage->GetPropertyList()->SetStringProperty(mitk::CustomTagParser::m_OffsetsPropertyName.c_str(), m_RealOffsets.c_str());
  // remove uids
  resultMitkImage->GetPropertyList()->DeleteProperty("DICOM.0008.0018");
  resultMitkImage->GetPropertyList()->DeleteProperty("DICOM.0020.000D");
  resultMitkImage->GetPropertyList()->DeleteProperty("DICOM.0020.000E");

}

template <typename TPixel, unsigned int VImageDimension>
void mitk::CESTImageNormalizationFilter::NormalizeTimeSteps(const itk::Image<TPixel, VImageDimension>* image)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> OutputImageType;

  std::string offsets = "";
  this->GetInput()->GetPropertyList()->GetStringProperty(mitk::CustomTagParser::m_OffsetsPropertyName.c_str(), offsets);

  std::vector<std::string> parts;
  boost::split(parts, offsets, boost::is_any_of(" "));

  // determine normalization images
  std::vector<unsigned int> mZeroIndices;
  std::stringstream offsetsWithoutM0;
  m_NonM0Indices.clear();
  for (unsigned int index = 0; index < parts.size(); ++index)
  {
    if ((std::stod(parts.at(index)) < -299) || (std::stod(parts.at(index)) > 299))
    {
      mZeroIndices.push_back(index);
    }
    else
    {
      offsetsWithoutM0 << parts.at(index) << " ";
      m_NonM0Indices.push_back(index);
    }
  }


  auto resultImage = OutputImageType::New();
  typename ImageType::RegionType targetEntireRegion = image->GetLargestPossibleRegion();
  targetEntireRegion.SetSize(3, m_NonM0Indices.size());
  resultImage->SetRegions(targetEntireRegion);
  resultImage->Allocate();
  resultImage->FillBuffer(0);

  unsigned int numberOfTimesteps = image->GetLargestPossibleRegion().GetSize(3);

  typename ImageType::RegionType lowerMZeroRegion = image->GetLargestPossibleRegion();
  lowerMZeroRegion.SetSize(3, 1);
  typename ImageType::RegionType upperMZeroRegion = image->GetLargestPossibleRegion();
  upperMZeroRegion.SetSize(3, 1);
  typename ImageType::RegionType sourceRegion = image->GetLargestPossibleRegion();
  sourceRegion.SetSize(3, 1);
  typename OutputImageType::RegionType targetRegion = resultImage->GetLargestPossibleRegion();
  targetRegion.SetSize(3, 1);
  unsigned int targetTimestep = 0;
  for (unsigned int sourceTimestep = 0; sourceTimestep < numberOfTimesteps; ++sourceTimestep)
  {
    unsigned int lowerMZeroIndex = mZeroIndices[0];
    unsigned int upperMZeroIndex = mZeroIndices[0];
    for (unsigned int loop = 0; loop < mZeroIndices.size(); ++loop)
    {
      if (mZeroIndices[loop] <= sourceTimestep)
      {
        lowerMZeroIndex = mZeroIndices[loop];
      }
      if (mZeroIndices[loop] > sourceTimestep)
      {
        upperMZeroIndex = mZeroIndices[loop];
        break;
      }
    }
    bool isMZero = (lowerMZeroIndex == sourceTimestep);

    double weight = 0.0;
    if (lowerMZeroIndex == upperMZeroIndex)
    {
      weight = 1.0;
    }
    else
    {
      weight = 1.0 - double(sourceTimestep - lowerMZeroIndex) / double(upperMZeroIndex - lowerMZeroIndex);
    }


    if (isMZero)
    {
      //do nothing
    }
    else
    {
      lowerMZeroRegion.SetIndex(3, lowerMZeroIndex);
      upperMZeroRegion.SetIndex(3, upperMZeroIndex);
      sourceRegion.SetIndex(3, sourceTimestep);
      targetRegion.SetIndex(3, targetTimestep);

      itk::ImageRegionConstIterator<ImageType> lowerMZeroIterator(image, lowerMZeroRegion);
      itk::ImageRegionConstIterator<ImageType> upperMZeroIterator(image, upperMZeroRegion);
      itk::ImageRegionConstIterator<ImageType> sourceIterator(image, sourceRegion);
      itk::ImageRegionIterator<OutputImageType> targetIterator(resultImage.GetPointer(), targetRegion);

      while (!sourceIterator.IsAtEnd())
      {
        targetIterator.Set(double(sourceIterator.Get()) /
          (weight * lowerMZeroIterator.Get() + (1.0 - weight) * upperMZeroIterator.Get()));

        ++lowerMZeroIterator;
        ++upperMZeroIterator;
        ++sourceIterator;
        ++targetIterator;
      }
      ++targetTimestep;
    }
  }

  // get  Pointer to output image
  mitk::Image::Pointer resultMitkImage = this->GetOutput();
  // write into output image
  mitk::CastToMitkImage<OutputImageType>(resultImage, resultMitkImage);

  m_RealOffsets = offsetsWithoutM0.str();
}

void mitk::CESTImageNormalizationFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");
}
