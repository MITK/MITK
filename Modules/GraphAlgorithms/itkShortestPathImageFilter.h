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
#ifndef __itkShortestPathImageFilter_h
#define __itkShortestPathImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkShortestPathCostFunction.h"
#include "itkShortestPathNode.h"
#include <itkImageRegionIteratorWithIndex.h>

#include <itkMacro.h>

// ------- INFORMATION ----------
/// SET FUNCTIONS
// void SetInput( ItkImage ) // Compulsory
// void SetStartIndex (const IndexType & StartIndex); // Compulsory
// void SetEndIndex(const IndexType & EndIndex); // Compulsory
// void SetFullNeighborsMode(bool) // Optional (default=false), if false N4, if true N26
// void SetActivateTimeOut(bool) // Optional (default=false), for debug issues: after 30s algorithms terminates. You can
// have a look at the VectorOrderImage to see how far it came
// void SetMakeOutputImage(bool) // Optional (default=true), Generate an outputimage of the path. You can also get the
// path directoy with GetVectorPath()
// void SetCalcAllDistances(bool) // Optional (default=false), Calculate Distances over the whole image. CAREFUL,
// algorithm time extends a lot. Necessary for GetDistanceImage
// void SetStoreVectorOrder(bool) // Optional (default=false), Stores in which order the pixels were checked. Necessary
// for GetVectorOrderImage
// void AddEndIndex(const IndexType & EndIndex) //Optional. By calling this function you can add several endpoints! The
// algorithm will look for several shortest Pathes. From Start to all Endpoints.
//
/// GET FUNCTIONS
// std::vector< itk::Index<3> > GetVectorPath(); // returns the shortest path as vector
// std::vector< std::vector< itk::Index<3> > GetMultipleVectorPathe(); // returns a vector of shortest Pathes (which are
// vectors of points)
// GetDistanceImage // Returns the distance image
// GetVectorOrderIMage // Returns the Vector Order image
//
// EXAMPLE USE
// pleae see qmitkmitralvalvesegmentation4dtee bundle

namespace itk
{
  template <class TInputImageType, class TOutputImageType>
  class ShortestPathImageFilter : public ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:
    // Standard Typedefs
    typedef ShortestPathImageFilter Self;
    typedef ImageToImageFilter<TInputImageType, TOutputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    // Typdefs for metric
    typedef ShortestPathCostFunction<TInputImageType> CostFunctionType;
    typedef typename CostFunctionType::Pointer CostFunctionTypePointer;

    // More typdefs for convenience
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
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      // Run-time type information
      itkTypeMacro(ShortestPathImageFilter, ImageToImageFilter)

      // Display
      void PrintSelf(std::ostream &os, Indent indent) const override;

    // Compare function for A_STAR
    struct CompareNodeStar
    {
      bool operator()(ShortestPathNode *a, ShortestPathNode *b) { return (a->distAndEst > b->distAndEst); }
    };

    // \brief Set Starpoint for ShortestPath Calculation
    void SetStartIndex(const IndexType &StartIndex);

    // \brief Adds Endpoint for multiple ShortestPath Calculation
    void AddEndIndex(const IndexType &index);

    // \brief Set Endpoint for ShortestPath Calculation
    void SetEndIndex(const IndexType &EndIndex);

    // \brief Set FullNeighborsMode. false = no diagonal neighbors, in 2D this means N4 Neigborhood. true = would be N8
    // in 2D
    itkSetMacro(FullNeighborsMode, bool);
    itkGetMacro(FullNeighborsMode, bool);

    // \brief Set Graph_fullNeighbors. false = no diagonal neighbors, in 2D this means N4 Neigborhood. true = would be
    // N8 in 2D
    itkSetMacro(Graph_fullNeighbors, bool)

      // \brief (default=true), Produce output image, which shows the shortest path. But you can also get the shortest
      // Path directly as vector with the function GetVectorPath
      itkSetMacro(MakeOutputImage, bool);
    itkGetMacro(MakeOutputImage, bool);

    // \brief (default=false), Store an Vector of Order, so you can call getVectorOrderImage after update
    itkSetMacro(StoreVectorOrder, bool);
    itkGetMacro(StoreVectorOrder, bool);

    // \brief (default=false), // Calculate all Distances to all pixels, so you can call getDistanceImage after update
    // (warning algo will take a long time)
    itkSetMacro(CalcAllDistances, bool);
    itkGetMacro(CalcAllDistances, bool);

    // \brief (default=false), for debug issues: after 30s algorithms terminates. You can have a look at the
    // VectorOrderImage to see how far it came
    itkSetMacro(ActivateTimeOut, bool);
    itkGetMacro(ActivateTimeOut, bool);

    // \brief returns shortest Path as vector
    std::vector<IndexType> GetVectorPath();

    // \brief returns Multiple shortest Paths. You can call this function, when u performed a multiple shortest path
    // search (one start, several ends)
    std::vector<std::vector<IndexType>> GetMultipleVectorPaths();

    // \brief returns the vector order image. It shows in which order the pixels were checked. good for debugging. Be
    // sure to have m_StoreVectorOrder=true
    OutputImagePointer GetVectorOrderImage();

    // \brief returns the distance image. It shows the distances from the startpoint to all other pixels. Be sure to
    // have m_CalcAllDistances=true
    OutputImagePointer GetDistanceImage();

    // \brief Fill m_VectorPath
    void MakeShortestPathVector();

    // \brief cleans up the filter
    void CleanUp();

    itkSetObjectMacro(CostFunction,
                      CostFunctionType); // itkSetObjectMacro = set function that uses pointer as parameter
    itkGetObjectMacro(CostFunction, CostFunctionType);

  protected:
    std::vector<IndexType>
      m_endPoints; // if you fill this vector, the algo will not rest until all endPoints have been reached
    std::vector<IndexType> m_endPointsClosed;

    ShortestPathNode *m_Nodes; // main list that contains all nodes
    NodeNumType m_Graph_NumberOfNodes;
    NodeNumType m_Graph_StartNode;
    NodeNumType m_Graph_EndNode;
    bool m_Graph_fullNeighbors;
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

#include "itkShortestPathImageFilter.txx"

#endif
