/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkShortestPathCostFunctionLiveWire_h
#define itkShortestPathCostFunctionLiveWire_h

#include <itkShortestPathCostFunction.h>

#include <itkImageRegionConstIterator.h>

namespace itk
{
  /**
   * \brief Cost function for LiveWire interactive segmentation.
   *
   * This cost function computes the cumulative link cost between two
   * neighboring pixels using a weighted combination of three image features:
   *
   * - **Gradient Magnitude**: Mapped linearly (or dynamically via a cost map)
   *   to values between 0 (strong edge, low cost) and 1 (weak edge, high cost).
   * - **Gradient Direction**: The angular difference between gradient vectors at
   *   the two pixels, normalized to [0, 1].
   * - **Laplacian Zero Crossing** (via Canny edge detection): Binary cost of 0
   *   at edge pixels and 1 elsewhere.
   *
   * By default, gradient magnitude is mapped linearly to cost values. A dynamic
   * cost map can be set via SetDynamicCostMap() to use a non-linear mapping
   * based on an inverted histogram of the gradient magnitude image.
   *
   * The cost is scaled by the Euclidean distance between the two pixels
   * (1.0 for horizontal/vertical neighbors, sqrt(2) for diagonal neighbors).
   *
   * Repulsive points can be added to force the path away from certain pixels.
   *
   * \tparam TInputImageType The ITK image type used as input (typically 2D).
   *
   * \sa ShortestPathCostFunction
   * \sa ShortestPathImageFilter
   */
  template <class TInputImageType>
  class ITK_EXPORT ShortestPathCostFunctionLiveWire : public ShortestPathCostFunction<TInputImageType>
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunctionLiveWire Self;
    typedef ShortestPathCostFunction<TInputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef itk::ImageRegionConstIterator<TInputImageType> ConstIteratorType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(ShortestPathCostFunctionLiveWire, ShortestPathCostFunction);

    typedef itk::Image<unsigned char, 2> UnsignedCharImageType;
    typedef itk::Image<float, 2> FloatImageType;

    typedef float ComponentType;
    typedef itk::CovariantVector<ComponentType, 2> OutputPixelType;
    typedef itk::Image<OutputPixelType, 2> VectorOutputImageType;

    typedef typename TInputImageType::IndexType IndexType;
    typedef TInputImageType ImageType;
    typedef itk::ImageRegion<2> RegionType;

    /**
     * \brief Calculate the weighted cost of transitioning from p1 to p2.
     *
     * The cost is a weighted sum of Laplacian zero-crossing cost,
     * gradient magnitude cost, and gradient direction cost, scaled
     * by the Euclidean distance between the two pixels.
     *
     * \param[in] p1 The source pixel index.
     * \param[in] p2 The destination pixel index.
     * \return The combined weighted cost. Returns 1000 if either pixel is a repulsive point.
     */
    double GetCost(IndexType p1, IndexType p2) override;

    /**
     * \brief Return the minimal possible cost (needed for the A* heuristic).
     * \return The stored minimum cost value (0 by default, yielding Dijkstra behavior).
     */
    double GetMinCost() override;

    /**
     * \brief Initialize the cost function by computing derived images.
     *
     * Computes gradient magnitude, gradient direction, and Canny edge
     * detection images from the input image. Also initializes the mask
     * image for repulsive points. Only performs computation on the first
     * call; subsequent calls only update start/end pixel values.
     *
     * \pre The input image must have been set via SetImage().
     */
    void Initialize() override;

    /**
     * \brief Mark a pixel as repulsive in the cost map.
     *
     * Pixels marked as repulsive receive a very high cost (1000) to
     * discourage the shortest path from passing through them.
     *
     * \param[in] index The pixel index to mark as repulsive.
     *
     * \sa RemoveRepulsivePoint
     * \sa ClearRepulsivePoints
     */
    virtual void AddRepulsivePoint(const IndexType &index);

    /**
     * \brief Remove a repulsive point from the cost map.
     *
     * \param[in] index The pixel index to unmark.
     *
     * \sa AddRepulsivePoint
     * \sa ClearRepulsivePoints
     */
    virtual void RemoveRepulsivePoint(const IndexType &index);

    /**
     * \brief Clear all repulsive points from the cost function.
     *
     * Resets the mask image to zero and disables repulsive point checking.
     *
     * \sa AddRepulsivePoint
     * \sa RemoveRepulsivePoint
     */
    virtual void ClearRepulsivePoints();

    /** \brief Set the requested image region for processing. */
    itkSetMacro(RequestedRegion, RegionType);
    /** \brief Get the requested image region. */
    itkGetMacro(RequestedRegion, RegionType);

    /**
     * \brief Set the input image and initialize the mask image.
     *
     * Overrides the base class to additionally allocate and initialize
     * the repulsive-point mask image to match the input image's geometry.
     *
     * \param[in] _arg The input image.
     */
    void SetImage(const TInputImageType *_arg) override;

    /**
     * \brief Set a dynamic (non-linear) cost map for gradient magnitude mapping.
     *
     * The map keys are integer-scaled gradient magnitude values, and the
     * map values represent corresponding histogram-derived weights. When
     * set, a Gaussian interpolation is used instead of linear mapping.
     *
     * \param[in,out] costMap The cost map to use. Stored by copy.
     *
     * \sa SetUseCostMap
     * \sa SetCostMapMaximum
     */
    void SetDynamicCostMap(std::map<int, int> &costMap)
    {
      this->m_CostMap = costMap;
      this->m_UseCostMap = true;
      this->m_MaxMapCosts = -1;
      this->Modified();
    }

    /**
     * \brief Enable or disable the dynamic cost map.
     *
     * \param[in] useCostMap If true, use the dynamic cost map; if false, use linear mapping.
     */
    void SetUseCostMap(bool useCostMap) { this->m_UseCostMap = useCostMap; }

    /**
     * \brief Set the maximum value of the dynamic cost map.
     *
     * Used for normalization when the dynamic cost map is active. Setting
     * this avoids recomputing the maximum from the map each time.
     *
     * \param[in] max The maximum cost map value.
     */
    void SetCostMapMaximum(double max) { this->m_MaxMapCosts = max; }

    /**
     * \brief Constants used internally for cost map scaling.
     */
    enum Constants
    {
      MAPSCALEFACTOR = 10 ///< Scale factor applied to gradient magnitude for cost map lookup.
    };

    /**
     * \brief Evaluate a Gaussian function at position x.
     *
     * Computes the Gaussian approximation:
     * \code
     * f(x) = yOfGaussian * exp( -0.5 * (x - xOfGaussian)^2 )
     * \endcode
     *
     * Used for interpolating between cost map entries.
     *
     * \param[in] x The position at which to evaluate.
     * \param[in] xOfGaussian The center (mean) of the Gaussian.
     * \param[in] yOfGaussian The amplitude (peak value) of the Gaussian.
     * \return The Gaussian value at position x.
     */
    static double Gaussian(double x, double xOfGaussian, double yOfGaussian);

    /**
     * \brief Get the repulsive-point mask image.
     * \return Pointer to the mask image (pixel value 255 = repulsive, 0 = normal).
     */
    const UnsignedCharImageType *GetMaskImage() { return this->m_MaskImage.GetPointer(); };

    /**
     * \brief Get the computed gradient magnitude image.
     * \return Pointer to the gradient magnitude image.
     */
    const FloatImageType *GetGradientMagnitudeImage() { return this->m_GradientMagnitudeImage.GetPointer(); };

    /**
     * \brief Get the computed edge image (Canny edge detection result).
     * \return Pointer to the edge image.
     */
    const FloatImageType *GetEdgeImage() { return this->m_EdgeImage.GetPointer(); };

    /**
     * \brief Get the computed gradient vector image.
     * \return Pointer to the 2D gradient vector image.
     */
    const VectorOutputImageType *GetGradientImage() { return this->m_GradientImage.GetPointer(); };
  protected:
    ShortestPathCostFunctionLiveWire();

    ~ShortestPathCostFunctionLiveWire() override{};

    FloatImageType::Pointer m_GradientMagnitudeImage;
    FloatImageType::Pointer m_EdgeImage;
    UnsignedCharImageType::Pointer m_MaskImage;
    VectorOutputImageType::Pointer m_GradientImage;

    double m_MinCosts;

    bool m_UseRepulsivePoints;

    typename Superclass::PixelType val;

    typename Superclass::PixelType startValue;
    typename Superclass::PixelType endValue;

    double m_GradientMax;

    RegionType m_RequestedRegion;

    bool m_Initialized;

    std::map<int, int> m_CostMap;

    bool m_UseCostMap;

    double m_MaxMapCosts;

  private:
    double SigmoidFunction(double I, double max, double min, double alpha, double beta);
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkShortestPathCostFunctionLiveWire.tpp>
#endif

#endif /* itkShortestPathCostFunctionLiveWire_h */
