/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkShortestPathCostFunctionTbss_h
#define itkShortestPathCostFunctionTbss_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkShapedNeighborhoodIterator.h>
#include <itkShortestPathCostFunction.h> // Superclass of Metrics
#include <itkImageRegionConstIterator.h>
#include <itkMacro.h>

namespace itk
{
  /**
   * \brief Cost function for Tract-Based Spatial Statistics (TBSS) path finding.
   *
   * This cost function is designed for finding shortest paths along white
   * matter tracts in TBSS analysis. The cost of transitioning between two
   * pixels combines the Euclidean distance with a penalty inversely
   * proportional to the pixel intensity (tract weight).
   *
   * Pixels with intensity below the configured threshold are considered
   * impassable (infinite cost), preventing the path from leaving the
   * tract skeleton.
   *
   * The cost formula is:
   * \code
   * cost = sqrt(dx^2 + dy^2 + dz^2) + 1000 * (1 - weight)
   * \endcode
   * where \c weight is the pixel intensity at the destination pixel.
   *
   * \tparam TInputImageType The ITK image type (typically 3D float image).
   *
   * \sa ShortestPathCostFunction
   * \sa ShortestPathImageFilter
   */
  template <class TInputImageType>
  class ShortestPathCostFunctionTbss : public ShortestPathCostFunction<TInputImageType>
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunctionTbss Self;
    typedef ShortestPathCostFunction<TInputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef itk::ImageRegionConstIterator<TInputImageType> ConstIteratorType;
    typedef typename TInputImageType::IndexType IndexType;

    typedef itk::Image<float, 3> FloatImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(Self, Superclass);

    /**
     * \brief Calculate the cost of transitioning from pixel p1 to pixel p2.
     *
     * If the intensity at p2 is below the threshold, returns
     * std::numeric_limits<double>::max() (impassable). Otherwise, returns
     * the Euclidean distance plus a penalty of 1000 * (1 - weight).
     *
     * \param[in] p1 The source pixel index.
     * \param[in] p2 The destination pixel index.
     * \return The transition cost, or numeric max if below threshold.
     */
    double GetCost(IndexType p1, IndexType p2) override;

    /**
     * \brief Initialize the cost function.
     *
     * Currently a no-op for the TBSS cost function.
     */
    void Initialize() override;

    /**
     * \brief Return the minimal possible cost (needed for A*).
     * \return Always returns 1.0.
     */
    double GetMinCost() override;

    /**
     * \brief Set the intensity threshold below which pixels are impassable.
     *
     * Pixels with intensity below this value receive infinite cost,
     * effectively restricting paths to the tract skeleton.
     *
     * \param[in] t The threshold value.
     */
    void SetThreshold(double t) { m_Threshold = t; }

  protected:
    ShortestPathCostFunctionTbss();

    ~ShortestPathCostFunctionTbss() override{};

    double m_Threshold; ///< Intensity threshold below which pixels are impassable.

  private:
  };

} // end namespace itk

#include <itkShortestPathCostFunctionTbss.tpp>

#endif /* itkShortestPathCostFunctionTbss_h */
