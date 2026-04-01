/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkShortestPathImageFilter_h
#define itkShortestPathImageFilter_h

#include <itkImageToImageFilter.h>
#include <itkShortestPathCostFunction.h>
#include <itkShortestPathNode.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <itkMacro.h>

namespace itk
{
  /**
   * \brief ITK image filter that computes the shortest path between pixels using A*.
   *
   * This filter finds the optimal (minimum cost) path between a start pixel and
   * one or more end pixels in an image, using the A* graph search algorithm with
   * a pluggable cost function. The cost function (ShortestPathCostFunction)
   * determines the transition cost between neighboring pixels.
   *
   * **Required inputs:**
   * - Input image via SetInput()
   * - Start pixel via SetStartIndex()
   * - End pixel via SetEndIndex() (or multiple via AddEndIndex())
   *
   * **Optional settings:**
   * - SetFullNeighborsMode(): N4/N6 (false, default) or N8/N26 (true) neighborhood
   * - SetMakeOutputImage(): Generate a binary output image of the path (default: true)
   * - SetCalcAllDistances(): Compute distances to all pixels (default: false, slow)
   * - SetStoreVectorOrder(): Record pixel visit order for debugging (default: false)
   * - SetActivateTimeOut(): Abort after 30 seconds (default: false)
   * - SetCostFunction(): Plug in a custom cost function
   *
   * **Output retrieval:**
   * - GetVectorPath(): The shortest path as a vector of pixel indices
   * - GetMultipleVectorPaths(): Multiple paths when multiple endpoints are used
   * - GetDistanceImage(): Distance image (requires SetCalcAllDistances(true))
   * - GetVectorOrderImage(): Visit order image (requires SetStoreVectorOrder(true))
   *
   * \tparam TInputImageType The input image type (2D or 3D).
   * \tparam TOutputImageType The output image type (typically same dimensionality).
   *
   * \sa ShortestPathCostFunction
   * \sa ShortestPathCostFunctionLiveWire
   * \sa ShortestPathNode
   */
  template <class TInputImageType, class TOutputImageType>
  class ShortestPathImageFilter : public ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:
    // Standard Typedefs
    typedef ShortestPathImageFilter Self;
    typedef ImageToImageFilter<TInputImageType, TOutputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    // Typedefs for metric
    typedef ShortestPathCostFunction<TInputImageType> CostFunctionType;
    typedef typename CostFunctionType::Pointer CostFunctionTypePointer;

    // More typedefs for convenience
    typedef TInputImageType InputImageType;
    typedef typename TInputImageType::Pointer InputImagePointer;
    typedef typename TInputImageType::PixelType InputImagePixelType;
    typedef typename TInputImageType::SizeType InputImageSizeType;
    typedef typename TInputImageType::IndexType IndexType;
    typedef typename itk::ImageRegionIteratorWithIndex<InputImageType> InputImageIteratorType;

    typedef TOutputImageType OutputImageType;
    typedef typename TOutputImageType::Pointer OutputImagePointer;
    typedef typename TOutputImageType::PixelType OutputImagePixelType;
    typedef typename TOutputImageType::IndexType OutputImageIndexType;
    typedef ImageRegionIteratorWithIndex<OutputImageType> OutputImageIteratorType;
    typedef itk::ShapedNeighborhoodIterator<TInputImageType> itkShapedNeighborhoodIteratorType;

    // New Macro for smartpointer instantiation
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    // Run-time type information
    itkTypeMacro(ShortestPathImageFilter, ImageToImageFilter);

    /**
     * \brief Print object information to an output stream.
     * \param[in,out] os The output stream.
     * \param[in] indent The indentation level.
     */
    void PrintSelf(std::ostream &os, Indent indent) const override;

    /**
     * \brief Comparator for the A* priority queue.
     *
     * Orders ShortestPathNode pointers by their combined distance and
     * estimated remaining cost (distAndEst) in descending order, so
     * that the node with the lowest cost is at the top of the queue.
     */
    struct CompareNodeStar
    {
      bool operator()(ShortestPathNode *a, ShortestPathNode *b) { return (a->distAndEst > b->distAndEst); }
    };

    /**
     * \brief Set the start pixel index for the shortest path computation.
     *
     * \param[in] StartIndex The image index where the path begins.
     */
    void SetStartIndex(const IndexType &StartIndex);

    /**
     * \brief Add an additional endpoint for multi-target shortest path search.
     *
     * When multiple endpoints are added, the algorithm finds the shortest
     * path from the start to each endpoint. Results can be retrieved via
     * GetMultipleVectorPaths().
     *
     * \param[in] index The image index of the additional endpoint.
     *
     * \sa GetMultipleVectorPaths
     */
    void AddEndIndex(const IndexType &index);

    /**
     * \brief Set the (single) endpoint for the shortest path computation.
     *
     * \param[in] EndIndex The image index where the path ends.
     */
    void SetEndIndex(const IndexType &EndIndex);

    /**
     * \brief Set the neighborhood connectivity mode.
     *
     * When false (default), uses N4 (2D) or N6 (3D) neighborhood
     * (no diagonal neighbors). When true, uses N8 (2D) or N26 (3D).
     */
    itkSetMacro(FullNeighborsMode, bool);
    /** \brief Get the current neighborhood connectivity mode. */
    itkGetMacro(FullNeighborsMode, bool);

    /**
     * \brief Set graph-level full neighbors mode.
     *
     * Internal setting controlling whether the graph uses diagonal
     * neighbors. Typically set to match FullNeighborsMode.
     */
    itkSetMacro(Graph_fullNeighbors, bool);

    /**
     * \brief Set whether to produce a binary output image of the path.
     *
     * Default is true. When true, the output image shows FOREGROUND (255)
     * on path pixels and BACKGROUND (0) elsewhere. The path can also be
     * retrieved as a vector via GetVectorPath().
     */
    itkSetMacro(MakeOutputImage, bool);
    /** \brief Get whether output image generation is enabled. */
    itkGetMacro(MakeOutputImage, bool);

    /**
     * \brief Set whether to record the pixel visit order.
     *
     * Default is false. When true, the visit order is stored and can be
     * retrieved via GetVectorOrderImage(). Useful for debugging.
     */
    itkSetMacro(StoreVectorOrder, bool);
    /** \brief Get whether visit order recording is enabled. */
    itkGetMacro(StoreVectorOrder, bool);

    /**
     * \brief Set whether to compute distances to all pixels.
     *
     * Default is false. When true, the algorithm does not stop at the
     * endpoint but continues until all reachable pixels are visited.
     * Results can be retrieved via GetDistanceImage().
     *
     * \warning This significantly increases computation time.
     */
    itkSetMacro(CalcAllDistances, bool);
    /** \brief Get whether all-distances mode is enabled. */
    itkGetMacro(CalcAllDistances, bool);

    /**
     * \brief Set whether to enable a 30-second timeout.
     *
     * Default is false. When true, the algorithm terminates after 30
     * seconds. Useful for debugging; inspect GetVectorOrderImage() to
     * see how far the search progressed.
     */
    itkSetMacro(ActivateTimeOut, bool);
    /** \brief Get whether the timeout is enabled. */
    itkGetMacro(ActivateTimeOut, bool);

    /**
     * \brief Get the computed shortest path as a vector of pixel indices.
     *
     * \return A vector of IndexType representing the path from start to end.
     * \pre The filter must have been updated (Update() called).
     */
    std::vector<IndexType> GetVectorPath();

    /**
     * \brief Get multiple shortest paths for multi-endpoint searches.
     *
     * Returns one path per endpoint added via AddEndIndex().
     *
     * \return A vector of vectors, each containing a path as pixel indices.
     * \pre Multiple endpoints must have been added and the filter updated.
     *
     * \sa AddEndIndex
     */
    std::vector<std::vector<IndexType>> GetMultipleVectorPaths();

    /**
     * \brief Get an image showing the pixel visit order.
     *
     * Each pixel value represents the order in which it was visited
     * during the search. Useful for debugging and visualization.
     *
     * \return The visit order image.
     * \pre SetStoreVectorOrder(true) must have been called before Update().
     */
    OutputImagePointer GetVectorOrderImage();

    /**
     * \brief Get the distance image showing shortest distances from the start.
     *
     * Each pixel value represents the minimum cumulative cost from the
     * start pixel to that pixel.
     *
     * \return The distance image.
     * \pre SetCalcAllDistances(true) must have been called before Update().
     */
    OutputImagePointer GetDistanceImage();

    /**
     * \brief Trace back from end to start and fill m_VectorPath.
     *
     * Follows the prevNode links from the end node back to the start
     * node to construct the shortest path vector.
     */
    void MakeShortestPathVector();

    /**
     * \brief Clean up allocated resources.
     *
     * Deletes the node array and resets internal state. Called
     * automatically at the end of GenerateData().
     */
    void CleanUp();

    /** \brief Set the cost function used for computing transition costs. */
    itkSetObjectMacro(CostFunction, CostFunctionType);
    /** \brief Get the cost function used for computing transition costs. */
    itkGetObjectMacro(CostFunction, CostFunctionType);

    /**
     * \brief Enable or disable the custom cost function.
     *
     * When disabled, a default unit cost is used for all transitions.
     *
     * \param[in] doUseCostFunction true to use the set cost function, false for unit cost.
     */
    void SetUseCostFunction(bool doUseCostFunction) { m_useCostFunction = doUseCostFunction; };

    /**
     * \brief Query whether the custom cost function is enabled.
     * \return true if the custom cost function is in use.
     */
    bool GetUseCostFunction() { return m_useCostFunction; };

  protected:
    std::vector<IndexType>
      m_endPoints; // if you fill this vector, the algo will not rest until all endPoints have been reached
    std::vector<IndexType> m_endPointsClosed;

    ShortestPathNode *m_Nodes; // main list that contains all nodes
    NodeNumType m_Graph_NumberOfNodes;
    NodeNumType m_Graph_StartNode;
    NodeNumType m_Graph_EndNode;
    bool m_Graph_fullNeighbors;
    bool m_useCostFunction;
    std::vector<ShortestPathNode *> m_Graph_DiscoveredNodeList;
    ShortestPathImageFilter(Self &); // intentionally not implemented
    void operator=(const Self &);    // intentionally not implemented
    const static int BACKGROUND = 0;
    const static int FOREGROUND = 255;
    bool m_FullNeighborsMode;

    bool m_MakeOutputImage;
    bool m_StoreVectorOrder; // Store an Vector of Order, so you can call getVectorOrderImage after update
    bool m_CalcAllDistances; // Calculate all Distances, so you can call getDistanceImage after update (warning algo
                             // will take a long time)

    bool multipleEndPoints;

    bool m_ActivateTimeOut; // if true, then i search max. 30 secs. then abort

    bool m_Initialized;

    CostFunctionTypePointer m_CostFunction;
    IndexType m_StartIndex, m_EndIndex;
    std::vector<IndexType> m_VectorPath;
    std::vector<std::vector<IndexType>> m_MultipleVectorPaths;

    std::vector<NodeNumType> m_VectorOrder;

    ShortestPathImageFilter();

    ~ShortestPathImageFilter() override;

    // \brief Create all the outputs
    void MakeOutputs();

    // \brief Generate Data
    void GenerateData() override;

    // \brief gets the estimate costs from pixel a to target.
    double getEstimatedCostsToTarget(const IndexType &a);

    typename InputImageType::Pointer m_magnitudeImage;

    // \brief Convert a indexnumber of a node in m_Nodes to image coordinates
    typename TInputImageType::IndexType NodeToCoord(NodeNumType);

    // \brief Convert image coordinate to a indexnumber of a node in m_Nodes
    unsigned int CoordToNode(IndexType);

    // \brief Returns the neighbors of a node
    std::vector<ShortestPathNode *> GetNeighbors(NodeNumType nodeNum, bool FullNeighbors);

    // \brief Check if coords are in bounds of image
    bool CoordIsInBounds(IndexType);

    // \brief Initializes the graph
    void InitGraph();

    // \brief Start ShortestPathSearch
    void StartShortestPathSearch();
  };

} // end of namespace itk

#include <itkShortestPathImageFilter.tpp>

#endif
