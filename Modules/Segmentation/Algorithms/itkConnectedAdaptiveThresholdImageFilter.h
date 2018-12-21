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
#ifndef __itkConnectedAdaptiveThresholdImageFilter_h
#define __itkConnectedAdaptiveThresholdImageFilter_h

#include "itkConnectedThresholdImageFilter.h"
#include "itkImage.h"

namespace itk
{
  /** /class ConnectedAdaptiveThreholdImageFilter
  * \brief ImageFilter used for processing an image with an adaptive
  *        iterator (such as itkAdaptiveThresholdIterator)
  *
  * \ingroup RegionGrowingSegmentation
  */
  template <class TInputImage, class TOutputImage>
  class ITK_EXPORT ConnectedAdaptiveThresholdImageFilter
    : public ConnectedThresholdImageFilter<TInputImage, TOutputImage>
  {
  public:
    /** Standard class typedefs. */
    typedef ConnectedAdaptiveThresholdImageFilter Self;
    typedef ConnectedThresholdImageFilter<TInputImage, TOutputImage> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /** Run-time type information (and related methods).  */
      itkTypeMacro(ConnectedAdaptiveThresholdImageFilter, ConnectedThresholdImageFilter);

    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;
    typedef typename OutputImageType::Pointer OutputImagePointer;
    typedef typename InputImageType::IndexType IndexType;
    typedef typename InputImageType::PixelType PixelType;

    void SetGrowingDirectionIsUpwards(bool upwards) { m_GrowingDirectionIsUpwards = upwards; }
    /* Switch between fine and raw leakage detection. */
    void SetFineDetectionMode(bool fine)
    {
      m_FineDetectionMode = fine;
      m_DiscardLastPreview = false;
    }

    int GetSeedpointValue(void) { return m_SeedpointValue; }
    int GetLeakagePoint(void) { return m_DetectedLeakagePoint; }

    /*
    * Correct the position of the seed point, only performed if seed point value is outside threshold range
    * @param sizeOfVolume edge length of the square volume in which the search for a "better" seed is performed
    */
    IndexType CorrectSeedPointPosition(unsigned int sizeOfVolume, int lowerTh, int upperTh);

    /* Sets all voxels in a square volume with the size of @param croppingSize
    * and the center point equal to @param seedPoint to the value zero.
    */
    void CropMask(unsigned int croppingSize);

    /* Modifies the iterator mask to keep all previous segmentation results in the same mask.
    * @returnParam largest value in the segmentation mask
    */
    unsigned int AdjustIteratorMask();

    /* Sets parameters needed for adjusting the iterator mask
    * @param iteratorMaskForFineSegmentation pointer to the image containing the complete segmentation result of one
    * leaf (inclusively leakage-segmentation)
    * @param adjLowerTh lower threshold value of the segmentation without leakage-segmentation
    * @param adjLowerTh upper threshold value of the segmentation without leakage-segmentation
    * @param discardLeafSegmentation flag if the last segmentation preview ended with a leakage already in the first
    * step
    */
    void SetParameterForFineSegmentation(TOutputImage *iteratorMaskForFineSegmentation,
                                         unsigned int adjLowerTh,
                                         unsigned int adjUpperTh,
                                         itk::Index<3> seedPoint,
                                         bool discardLeafSegmentation);

    TOutputImage *GetResultImage();

  protected:
    ConnectedAdaptiveThresholdImageFilter();
    ~ConnectedAdaptiveThresholdImageFilter() override{};

    void GenerateData() override;

  private:
    OutputImagePointer m_OutoutImageMaskFineSegmentation;
    bool m_GrowingDirectionIsUpwards;
    PixelType m_SeedpointValue;
    PixelType m_DetectedLeakagePoint;
    PixelType m_InitValue;
    unsigned int m_AdjLowerTh;
    unsigned int m_AdjUpperTh;
    itk::Index<3> m_SeedPointIndex;

    /* Flag for switching between raw segmentation and fine segmentation (Bronchial tree segmentation) */
    bool m_FineDetectionMode;

    bool m_DiscardLastPreview;
    bool m_SegmentationCancelled;
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkConnectedAdaptiveThresholdImageFilter.txx"
#endif

#endif
