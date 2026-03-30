/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkShortestPathCostFunction_h
#define itkShortestPathCostFunction_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkShapedNeighborhoodIterator.h>

namespace itk
{
  /**
   * \brief Abstract base class for cost functions used by ShortestPathImageFilter.
   *
   * This pure virtual class defines the interface for cost functions that
   * compute the transition cost between two neighboring pixels in an image.
   * Subclasses must implement GetCost(), GetMinCost(), and Initialize() to
   * provide application-specific cost metrics (e.g., gradient-based costs
   * for LiveWire segmentation, or TBSS-based costs for tract definition).
   *
   * The cost function operates on images of type \p TInputImageType and is
   * used by ShortestPathImageFilter to find the cheapest path between a
   * start and end index.
   *
   * \tparam TInputImageType The ITK image type used as input. Must provide
   *         PixelType and IndexType typedefs.
   *
   * \sa ShortestPathImageFilter
   * \sa ShortestPathCostFunctionLiveWire
   * \sa ShortestPathCostFunctionTbss
   */
  template <class TInputImageType>
  class ShortestPathCostFunction : public Object
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunction Self;
    typedef Object Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef ShapedNeighborhoodIterator<TInputImageType> ShapedNeighborhoodIteratorType;

    /** Run-time type information (and related methods). */
    itkTypeMacro(ShortestPathCostFunction, Object);

    /** Type definition for the input image. */
    typedef TInputImageType ImageType;

    // More typedefs for convenience
    typedef typename TInputImageType::Pointer ImagePointer;
    typedef typename TInputImageType::ConstPointer ImageConstPointer;
    typedef typename TInputImageType::PixelType PixelType;
    typedef typename TInputImageType::IndexType IndexType;

    /** \brief Set the input image on which costs are computed. */
    itkSetConstObjectMacro(Image, TInputImageType);

    /**
     * \brief Calculate the cost of transitioning from pixel p1 to pixel p2.
     *
     * \param[in] p1 The index of the source pixel.
     * \param[in] p2 The index of the destination pixel (a neighbor of p1).
     * \return The cost of moving from p1 to p2. Lower values indicate cheaper transitions.
     */
    virtual double GetCost(IndexType p1, IndexType p2) = 0;

    /**
     * \brief Return the minimal possible cost for any transition.
     *
     * This lower bound is required by the A* search heuristic. If the
     * return value is 0, the algorithm degrades to Dijkstra's algorithm.
     *
     * \return The minimum possible transition cost.
     */
    virtual double GetMinCost() = 0;

    /**
     * \brief Initialize the cost function.
     *
     * Called before the shortest path search begins. Subclasses should
     * perform any required precomputation (e.g., computing gradient images).
     *
     * \pre The input image must have been set via SetImage().
     */
    virtual void Initialize() = 0;

    /**
     * \brief Set the starting index of the path.
     *
     * \param[in] index The image index where the path begins.
     */
    void SetStartIndex(const IndexType &index);

    /**
     * \brief Set the ending index of the path.
     *
     * \param[in] index The image index where the path ends.
     */
    void SetEndIndex(const IndexType &index);

  protected:
    ShortestPathCostFunction(){};
    ~ShortestPathCostFunction() override{};
    void PrintSelf(std::ostream &os, Indent indent) const override;
    ImageConstPointer m_Image;  ///< The input image used for cost computation.
    IndexType m_StartIndex, m_EndIndex; ///< Start and end indices defining the path endpoints.

  private:
    ShortestPathCostFunction(const Self &); // purposely not implemented
    void operator=(const Self &);           // purposely not implemented
  };

} // end namespace itk

#include <itkShortestPathCostFunction.tpp>

#endif /* itkShortestPathCostFunction_h */
