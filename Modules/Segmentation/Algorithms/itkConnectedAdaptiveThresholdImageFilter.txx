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

#ifndef _itkConnectedAdaptiveThresholdImageFilter_txx
#define _itkConnectedAdaptiveThresholdImageFilter_txx

#include "itkAdaptiveThresholdIterator.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkConnectedAdaptiveThresholdImageFilter.h"
#include "itkMinimumMaximumImageFilter.h"
#include "itkThresholdImageFilter.h"

namespace itk
{
  /**
  * Constructor
  */
  template <class TInputImage, class TOutputImage>
  ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::ConnectedAdaptiveThresholdImageFilter()
    : m_OutoutImageMaskFineSegmentation(nullptr),
      m_GrowingDirectionIsUpwards(true),
      m_SeedpointValue(0),
      m_DetectedLeakagePoint(0),
      m_InitValue(0),
      m_AdjLowerTh(0),
      m_AdjUpperTh(0),
      m_FineDetectionMode(false),
      m_DiscardLastPreview(false),
      m_SegmentationCancelled(false)
  {
  }

  template <class TInputImage, class TOutputImage>
  void ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::GenerateData()
  {
    typename ConnectedAdaptiveThresholdImageFilter::InputImageConstPointer inputImage = this->GetInput();
    typename ConnectedAdaptiveThresholdImageFilter::OutputImagePointer outputImage = this->GetOutput();

    typename Superclass::InputPixelObjectType::Pointer lowerThreshold = this->GetLowerInput();
    typename Superclass::InputPixelObjectType::Pointer upperThreshold = this->GetUpperInput();

    // kommt drauf, wie wir hier die Pipeline aufbauen
    this->SetLower(lowerThreshold->Get());
    this->SetUpper(upperThreshold->Get());
    typedef BinaryThresholdImageFunction<InputImageType> FunctionType;
    typedef AdaptiveThresholdIterator<OutputImageType, FunctionType> IteratorType;

    int initValue = IteratorType::CalculateInitializeValue((int)(this->GetLower()), (int)(this->GetUpper()));

    // Initialize the output according to the segmentation (fine or raw)
    if (m_FineDetectionMode)
    {
      outputImage = this->m_OutoutImageMaskFineSegmentation;
    }

    typename ConnectedAdaptiveThresholdImageFilter::OutputImageRegionType region = outputImage->GetRequestedRegion();
    outputImage->SetBufferedRegion(region);
    outputImage->Allocate();
    if (!m_FineDetectionMode)
    { // only initalize the output image if we are using the raw segmentation mode
      outputImage->FillBuffer((typename ConnectedAdaptiveThresholdImageFilter::OutputImagePixelType)initValue);
    }

    typename FunctionType::Pointer function = FunctionType::New();
    function->SetInputImage(inputImage);

    typename Superclass::SeedContainerType seeds;
    seeds = this->GetSeeds();

    // pass parameters needed for region growing to iterator
    IteratorType it(outputImage, function, seeds);
    it.SetFineDetectionMode(m_FineDetectionMode);
    it.SetExpansionDirection(m_GrowingDirectionIsUpwards);
    it.SetMinTH((int)(this->GetLower()));
    it.SetMaxTH((int)(this->GetUpper()));
    it.GoToBegin();
    this->m_SeedpointValue = it.GetSeedPointValue();

    if ((this->GetLower()) > this->m_SeedpointValue || this->m_SeedpointValue > (this->GetUpper()))
    {
      // set m_SegmentationCancelled to true, so if it doesn't reach the point where it is set back to false
      // we can asssume that there was an error
      this->m_SegmentationCancelled = true;
      return;
    }

    // iterate through image until
    while (!it.IsAtEnd())
    {
      // make iterator go one step further (calls method DoFloodStep())
      ++it;
    }
    this->m_DetectedLeakagePoint = it.GetLeakagePoint();
    this->m_SegmentationCancelled = false;
  }

  template <class TInputImage, class TOutputImage>
  TOutputImage *itk::ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::GetResultImage()
  {
    return m_OutoutImageMaskFineSegmentation;
  }

  template <class TInputImage, class TOutputImage>
  typename ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::IndexType
    itk::ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::CorrectSeedPointPosition(
      unsigned int sizeOfVolume, int lowerTh, int upperTh)
  {
    typename ConnectedAdaptiveThresholdImageFilter::InputImageConstPointer inputImage = this->GetInput();
    typedef typename TInputImage::IndexType IndexType;
    IndexType itkIntelligentSeedIndex;

    int seedPixelValue = inputImage->GetPixel(m_SeedPointIndex);

    // set new seed index to the voxel with the darkest value and shortest distance to original seed
    if (seedPixelValue > upperTh || seedPixelValue < lowerTh)
    {
      // MITK_INFO << "seed pixel value [BEFORE] = " << seedPixelValue;
      // ToDo crop region
      itk::Index<3> workindex;
      for (int i = 0; i < 3; i++)
      {
        workindex[i] = m_SeedPointIndex[i] - sizeOfVolume / 2;
        if (workindex[i] < 0)
          workindex[i] = 0;
      }
      itk::Size<3> worksize;
      for (int i = 0; i < 3; i++)
      {
        worksize[i] = sizeOfVolume;
      }

      itk::ImageRegion<3> workregion(workindex, worksize);
      itk::ImageRegionIterator<TInputImage> regionIt(const_cast<TInputImage *>(inputImage.GetPointer()), workregion);

      // int darkestGrayValue=seedPixelValue;
      int currentGrayValue;

      float distance = (float)(sizeOfVolume / 2);
      float relativeDistance = 1; // between 0 and 1

      mitk::Vector3D seedVector, currentVector;
      mitk::FillVector3D(seedVector, m_SeedPointIndex[0], m_SeedPointIndex[1], m_SeedPointIndex[2]);
      currentVector = seedVector;

      float costValue = 0; // beware, Depending on seeking upper or lower value...

      for (regionIt.GoToBegin(); !regionIt.IsAtEnd(); ++regionIt)
      {
        // get current gray value
        currentGrayValue = regionIt.Value();
        // get current seed index
        m_SeedPointIndex = regionIt.GetIndex();

        // fill current vector
        mitk::FillVector3D(currentVector, m_SeedPointIndex[0], m_SeedPointIndex[1], m_SeedPointIndex[2]);
        // calculate distance from original seed to new seed
        mitk::Vector3D distVector = currentVector - seedVector;
        distance = fabs(distVector.GetSquaredNorm());
        relativeDistance = distance / (sizeOfVolume / 2);

        // calculate "cost function"
        float currentCostValue = (1 - relativeDistance) * currentGrayValue;

        if (currentCostValue < costValue && currentGrayValue < upperTh)
        {
          itkIntelligentSeedIndex = regionIt.GetIndex();
          costValue = currentCostValue;

          // MITK_INFO <<"cost value="<< costValue;
          // MITK_INFO <<"darkest and closest Voxel ="<< currentGrayValue;
          // MITK_INFO <<"m_UPPER="<< upperTh;
        }
      }
      // MITK_INFO<< "seed pixel value [AFTER] =" << inputImage->GetPixel(itkIntelligentSeedIndex) <<"\n";
    }
    else
    { // no correction of the seed point is needed, just pass the original seed
      itkIntelligentSeedIndex = m_SeedPointIndex;
    }

    return itkIntelligentSeedIndex;
  }

  template <class TInputImage, class TOutputImage>
  void itk::ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::CropMask(unsigned int croppingSize)
  {
    // initialize center point of the working region
    itk::Index<3> workindex;
    for (int i = 0; i < 3; i++)
    {
      workindex[i] = m_SeedPointIndex[i] - croppingSize / 2;
      if (workindex[i] < 0)
        workindex[i] = 0;
    }
    // initialize working volume
    itk::Size<3> worksize;
    for (int i = 0; i < 3; i++)
    {
      worksize[i] = croppingSize;
    }
    // set working region
    itk::ImageRegion<3> workregion(workindex, worksize);

    // check if the entire region is inside the image
    if (!(m_OutoutImageMaskFineSegmentation->GetLargestPossibleRegion().IsInside(workregion)))
    {
      // if not then crop to the intersection of the image (gemeinsame Schnittmenge Bild und workingRegion)
      if (!(workregion.Crop(m_OutoutImageMaskFineSegmentation->GetLargestPossibleRegion())))
      {
        MITK_ERROR << "Cropping working region failed!";
        return;
      }
    }

    // initialize region iterator
    itk::ImageRegionIterator<TOutputImage> regionIt(m_OutoutImageMaskFineSegmentation, workregion);
    for (regionIt.GoToBegin(); !regionIt.IsAtEnd(); ++regionIt)
    {
      // and set all voxel inside the working region to zero
      regionIt.Set(0);
    }
  }

  template <class TInputImage, class TOutputImage>
  unsigned int itk::ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::AdjustIteratorMask()
  {
    typedef itk::ThresholdImageFilter<TOutputImage> ThresholdFilterType;
    typedef itk::MinimumMaximumImageFilter<TOutputImage> MaxFilterType;

    typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
    typename MaxFilterType::Pointer maxFilter = MaxFilterType::New();

    unsigned int maxValue;

    if (!m_DiscardLastPreview)
    {
      // get the biggest value of the image
      maxFilter->SetInput(m_OutoutImageMaskFineSegmentation);
      maxFilter->UpdateLargestPossibleRegion();
      maxValue = maxFilter->GetMaximum();
    }
    else
    { // use the last biggest value in the preview. This was set in SetParameterForFineSegmentation(...adjLowerTh...) []
      maxValue = m_AdjLowerTh;
    }

    // set all values <lower && >upper to zero (thresouldOutside uses < and > NOT <= and >=)
    thresholdFilter->SetInput(m_OutoutImageMaskFineSegmentation);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->ThresholdOutside(m_AdjLowerTh, maxValue);
    thresholdFilter->UpdateLargestPossibleRegion();

    // set all values in between lower and upper (>=lower && <=upper) to the highest value in the image
    thresholdFilter->SetInput(thresholdFilter->GetOutput());
    thresholdFilter->SetOutsideValue(maxValue);
    thresholdFilter->ThresholdOutside(0, m_AdjLowerTh - 1);
    thresholdFilter->UpdateLargestPossibleRegion();

    m_OutoutImageMaskFineSegmentation = thresholdFilter->GetOutput();

    return maxValue;
  }

  template <class TInputImage, class TOutputImage>
  void itk::ConnectedAdaptiveThresholdImageFilter<TInputImage, TOutputImage>::SetParameterForFineSegmentation(
    TOutputImage *iteratorMaskForFineSegmentation,
    unsigned int adjLowerTh,
    unsigned int adjUpperTh,
    itk::Index<3> seedPoint,
    bool discardLeafSegmentation)
  {
    // just to make sure we´re in the right mode and the mask exsits
    if (m_FineDetectionMode && iteratorMaskForFineSegmentation)
    {
      m_OutoutImageMaskFineSegmentation = iteratorMaskForFineSegmentation;
      m_AdjLowerTh = adjLowerTh;
      m_AdjUpperTh = adjUpperTh; // still needed?
      m_SeedPointIndex = seedPoint;
      m_DiscardLastPreview = discardLeafSegmentation;
    }
    else
    {
      if (!m_FineDetectionMode)
      {
        MITK_ERROR << "Fine-detection-segmentation mode not set!";
      }
      else
      {
        MITK_ERROR << "Iterator-mask-image not set!";
      }
    }
  }

} // end namespace itk

#endif
