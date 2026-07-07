/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkAdaptiveThresholdIterator_h
#define itkAdaptiveThresholdIterator_h

#include <itkConditionalConstIterator.h>
#include <itkImage.h>
#include <itkIndex.h>
#include <itkSize.h>

#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace itk
{
  /**
  * \class AdaptiveThresholdIterator
  * \brief Iterates over an image using a variable image function,
  *  which threshold can be varied during the iteration process.
  */
  template <class TImage, class TFunction>
  class ITK_EXPORT AdaptiveThresholdIterator : public ConditionalConstIterator<TImage>
  {
  public:
    /** Standard class typedefs. */
    typedef AdaptiveThresholdIterator Self;

    typedef ConditionalConstIterator<TImage> Superclass;

    typedef TImage ImageType;

    // A temporary image used for storing info about all indices
    typedef TImage TTempImage;

    typename TTempImage::Pointer tempPtr;
    //[!] isn't really used?!

    /** Type of function */
    typedef TFunction FunctionType;

    /** Type of vector used to store location info in the spatial function */
    typedef typename TFunction::InputType FunctionInputType;

    /** Size typedef support. */
    typedef typename TImage::SizeType SizeType;

    /** Region typedef support */
    typedef typename TImage::RegionType RegionType;

    typedef typename TImage::IndexType IndexType;

    /** Internal Pixel Type */
    typedef typename TImage::InternalPixelType InternalPixelType;

    /** External Pixel Type */
    typedef typename TImage::PixelType PixelType;

    /** Queue containing indices representing a voxel position */
    typedef std::queue<IndexType> IndexQueueType;

    /** Map used to generate the output result */
    typedef std::map<unsigned int, IndexQueueType> QueueMapType;

    /** Dimension of the image the iterator walks.  This constant is needed so
    * that functions that are templated over image iterator type (as opposed to
    * being templated over pixel type and dimension) can have compile time
    * access to the dimension of the image that the iterator walks. */
    itkStaticConstMacro(NDimensions, unsigned int, TImage::ImageDimension);

    /** Constructor establishes an iterator to walk a particular image and a
    * particular region of that image. This version of the constructor uses
    * an explicit seed pixel for the flood fill, the "startIndex" */
    AdaptiveThresholdIterator(ImageType *imagePtr, FunctionType *fnPtr, IndexType startIndex);

    /** Constructor establishes an iterator to walk a particular image and a
    * particular region of that image. This version of the constructor uses
    * an explicit list of seed pixels for the flood fill, the "startIndex" */
    AdaptiveThresholdIterator(ImageType *imagePtr, FunctionType *fnPtr, std::vector<IndexType> &startIndex);

    /** Constructor establishes an iterator to walk a particular image and a
    * particular region of that image. This version of the constructor
    * should be used when the seed pixel is unknown */
    AdaptiveThresholdIterator(ImageType *imagePtr, FunctionType *fnPtr);

    /** \brief Default Destructor. */
    ~AdaptiveThresholdIterator() override{};

    /** \brief Initialize the iterator, called from constructor. */
    void InitializeIterator();

    /** \brief Advance the iterator by one step of the extended flood fill. */
    void DoExtendedFloodStep();

    /** \brief Set the expansion direction for adaptive thresholding.
      \param upwards If true, threshold expands upwards; otherwise downwards.
    */
    void SetExpansionDirection(bool upwards);

    /** \brief Initialize the region growing state machine. */
    void InitRegionGrowingState();

    /** \brief Set the minimum threshold value.
      \param min The minimum threshold.
    */
    void SetMinTH(int min);

    /** \brief Set the maximum threshold value.
      \param max The maximum threshold.
    */
    void SetMaxTH(int max);

    /** \brief Get the pixel value at the seed point. */
    int GetSeedPointValue(void);

    /** switch between fine and raw leakage detection */
    void SetFineDetectionMode(bool fine = false)
    {
      m_FineDetectionMode = fine;
      m_DetectionStop = false;
    }

    /** Get the index. This provides a read only reference to the index.
    * This causes the index to be calculated from pointer arithmetic and is
    * therefore an expensive operation.
    * \sa SetIndex */
    const IndexType GetIndex() override
    {
      return (*m_QueueMap.find(m_RegionGrowingState)).second.front();
    } // [!] is never called?!

    /** \brief Get the pixel value at the current iterator position. */
    const PixelType Get(void) const override
    {
      return const_cast<ImageType *>(this->m_Image.GetPointer())
        ->GetPixel((*m_QueueMap.find(m_RegionGrowingState)).second.front());
    }

    /** \brief Set the pixel value at the current iterator position. */
    void Set(const PixelType &value)
    {
      const_cast<ImageType *>(this->m_Image.GetPointer())
        ->GetPixel((*m_QueueMap.find(m_RegionGrowingState)).second.front()) = value;
    }

    /** \brief Reset the iterator to the beginning. */
    void GoToBegin();

    /** \brief Check whether the iterator has reached the end of the region. */
    bool IsAtEnd() const override { return this->m_IsAtEnd; };

    /** \brief Walk forward one index by performing an extended flood step. */
    void operator++() override { this->DoExtendedFloodStep(); }

    /** \brief Get the function used for threshold evaluation. */
    virtual SmartPointer<FunctionType> GetFunction() const { return m_Function; }
    /** operator= is provided to make sure the handle to the image is properly
    * reference counted. */
    Self &operator=(const Self &it)
    {
      this->m_Image = it.m_Image; // copy the smart pointer
      this->m_Region = it.m_Region;
      this->m_InitializeValue = it.m_InitializeValue;
      this->m_RegionGrowingState = it.m_RegionGrowingState;
      this->m_MinTH = it.m_MinTH;
      this->m_MaxTH = it.m_MaxTH;
      this->m_SeedPointValue = it.m_SeedPointValue;
      this->m_VoxelCounter = it.m_VoxelCounter;
      this->m_LastVoxelNumber = it.m_LastVoxelNumber;
      this->m_DetectedLeakagePoint = it.m_DetectedLeakagePoint;
      this->m_CurrentLeakageRatio = it.m_CurrentLeakageRatio;
      return *this;
    }

    /** \brief Compute whether the pixel at the given index should be included in the flood fill. */
    bool IsPixelIncluded(const IndexType &index) const override;

    /** \brief Calculate the initialization value for the output image based on threshold range.
      \param lower The lower threshold.
      \param upper The upper threshold.
      \return The initialization value.
    */
    static int CalculateInitializeValue(int lower, int upper) { return ((upper - lower) + 1) * (-1); };

    /** \brief Get the detected leakage point in the region growing state space. */
    int GetLeakagePoint(void) { return m_DetectedLeakagePoint; }
  protected:
    /** \brief Pointer to the output image to which the result shall be written. */
    SmartPointer<ImageType> m_OutputImage;

    SmartPointer<FunctionType> m_Function;

    /** A list of locations to start the recursive fill */
    std::vector<IndexType> m_StartIndices;

    /** The origin of the source image */
    typename ImageType::PointType m_ImageOrigin;

    /** The spacing of the source image */
    typename ImageType::SpacingType m_ImageSpacing;

    /** Region of the source image */
    RegionType m_ImageRegion;

    bool m_UpwardsExpansion;

    int m_InitializeValue;

    /** \brief Expand the threshold range upwards by one step. */
    void ExpandThresholdUpwards();

    /** \brief Expand the threshold range downwards by one step. */
    void ExpandThresholdDownwards();

    /** \brief Increment the region growing state counter. */
    void IncrementRegionGrowingState();

    /** \brief Estimate the distance (in expansion steps) of a voxel from the current state. */
    int EstimateDistance(IndexType);

    /** \brief Calculate the maximum number of region growing expansion steps. */
    unsigned int CalculateMaxRGS();

  private:
    void InsertIndexTypeIntoQueueMap(unsigned int key, IndexType index);

    int m_RegionGrowingState;

    QueueMapType m_QueueMap;

    int m_MinTH;

    int m_MaxTH;

    int m_SeedPointValue;

    unsigned int m_VoxelCounter;
    unsigned int m_LastVoxelNumber;

    int m_DetectedLeakagePoint;
    float m_CurrentLeakageRatio;

    void CheckSeedPointValue();

    /* flag for switching between raw leakage detection (bigger bronchial vessels)
    * and fine leakage detection (smaller bronchial vessels [starting from leaves])
    */
    bool m_FineDetectionMode;
    bool m_DetectionStop;
  };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkAdaptiveThresholdIterator.tpp>
#endif

#endif
