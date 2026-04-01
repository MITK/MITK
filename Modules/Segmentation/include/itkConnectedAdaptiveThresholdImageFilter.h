/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkConnectedAdaptiveThresholdImageFilter_h
#define itkConnectedAdaptiveThresholdImageFilter_h

#include <itkConnectedThresholdImageFilter.h>
#include <itkImage.h>

namespace itk
{
  /** \class ConnectedAdaptiveThresholdImageFilter
    \brief ImageFilter for region growing with an adaptive threshold iterator.

    Uses itkAdaptiveThresholdIterator to perform connected region growing
    with an adaptively expanding threshold range. Supports leakage detection
    in both raw and fine modes (e.g. for bronchial tree segmentation).

    \ingroup RegionGrowingSegmentation
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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods).  */
      itkTypeMacro(ConnectedAdaptiveThresholdImageFilter, ConnectedThresholdImageFilter);

    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;
    typedef typename OutputImageType::Pointer OutputImagePointer;
    typedef typename InputImageType::IndexType IndexType;
    typedef typename InputImageType::PixelType PixelType;

    /** \brief Set the growing direction for adaptive thresholding.
      \param upwards If true, threshold expands upwards; otherwise downwards.
    */
    void SetGrowingDirectionIsUpwards(bool upwards) { m_GrowingDirectionIsUpwards = upwards; }

    /** \brief Switch between fine and raw leakage detection mode.
      \param fine If true, fine detection mode is used (for smaller bronchial vessels).
    */
    void SetFineDetectionMode(bool fine)
    {
      m_FineDetectionMode = fine;
      m_DiscardLastPreview = false;
    }

    /** \brief Get the pixel value at the seed point. */
    int GetSeedpointValue(void) { return m_SeedpointValue; }

    /** \brief Get the detected leakage point value. */
    int GetLeakagePoint(void) { return m_DetectedLeakagePoint; }

    /** \brief Correct the seed point position if its value is outside the threshold range.

      Searches within a cubic volume of the given edge length for a voxel that
      lies within the threshold range.

      \param sizeOfVolume Edge length of the search volume.
      \param lowerTh Lower threshold bound.
      \param upperTh Upper threshold bound.
      \return The corrected seed point index.
    */
    IndexType CorrectSeedPointPosition(unsigned int sizeOfVolume, int lowerTh, int upperTh);

    /** \brief Set all voxels in a cubic region around the seed point to zero.
      \param croppingSize Edge length of the cubic cropping region.
    */
    void CropMask(unsigned int croppingSize);

    /** \brief Modify the iterator mask to accumulate previous segmentation results.
      \return The largest pixel value in the segmentation mask.
    */
    unsigned int AdjustIteratorMask();

    /** \brief Configure parameters for fine segmentation (bronchial tree leaf analysis).
      \param iteratorMaskForFineSegmentation Pointer to the image containing the complete
             segmentation result of one leaf (including leakage segmentation).
      \param adjLowerTh Lower threshold for the segmentation without leakage.
      \param adjUpperTh Upper threshold for the segmentation without leakage.
      \param seedPoint The seed point index for the fine segmentation.
      \param discardLeafSegmentation True if the last preview ended with leakage in the first step.
    */
    void SetParameterForFineSegmentation(TOutputImage *iteratorMaskForFineSegmentation,
                                         unsigned int adjLowerTh,
                                         unsigned int adjUpperTh,
                                         itk::Index<3> seedPoint,
                                         bool discardLeafSegmentation);

    /** \brief Get the result image after filter execution. */
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
#include <itkConnectedAdaptiveThresholdImageFilter.tpp>
#endif

#endif
