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
#ifndef __itkShortestPathImageFilter_txx
#define __itkShortestPathImageFilter_txx

#include "itkShortestPathImageFilter.h"

#include "mitkMemoryUtilities.h"
#include <ctime>
#include <algorithm>
#include <iostream>
#include <vector>

namespace itk
{
  // Constructor  (initialize standard values)
  template <class TInputImageType, class TOutputImageType>
  ShortestPathImageFilter<TInputImageType, TOutputImageType>::ShortestPathImageFilter()
    : m_Nodes(nullptr),
      m_Graph_NumberOfNodes(0),
      m_Graph_fullNeighbors(false),
      m_FullNeighborsMode(false),
      m_MakeOutputImage(true),
      m_StoreVectorOrder(false),
      m_CalcAllDistances(false),
      multipleEndPoints(false),
      m_ActivateTimeOut(false),
      m_Initialized(false)
  {
    m_endPoints.clear();
    m_endPointsClosed.clear();

    if (m_MakeOutputImage)
    {
      this->SetNumberOfRequiredOutputs(1);
      this->SetNthOutput(0, OutputImageType::New());
    }
  }

  template <class TInputImageType, class TOutputImageType>
  ShortestPathImageFilter<TInputImageType, TOutputImageType>::~ShortestPathImageFilter()
  {
    delete[] m_Nodes;
  }

  template <class TInputImageType, class TOutputImageType>
  inline typename ShortestPathImageFilter<TInputImageType, TOutputImageType>::IndexType
    ShortestPathImageFilter<TInputImageType, TOutputImageType>::NodeToCoord(NodeNumType node)
  {
    const InputImageSizeType &size = this->GetInput()->GetRequestedRegion().GetSize();
    int dim = InputImageType::ImageDimension;
    IndexType coord;
    if (dim == 2)
    {
      coord[1] = node / size[0];
      coord[0] = node % size[0];
      if (((unsigned long)coord[0] >= size[0]) || ((unsigned long)coord[1] >= size[1]))
      {
        coord[0] = 0;
        coord[1] = 0;
      }
    }
    if (dim == 3)
    {
      coord[2] = node / (size[0] * size[1]);
      coord[1] = (node % (size[0] * size[1])) / size[0];
      coord[0] = (node % (size[0] * size[1])) % size[0];
      if (((unsigned long)coord[0] >= size[0]) || ((unsigned long)coord[1] >= size[1]) ||
          ((unsigned long)coord[2] >= size[2]))
      {
        coord[0] = 0;
        coord[1] = 0;
        coord[2] = 0;
      }
    }

    return coord;
  }

  template <class TInputImageType, class TOutputImageType>
  inline typename itk::NodeNumType ShortestPathImageFilter<TInputImageType, TOutputImageType>::CoordToNode(
    IndexType coord)
  {
    const InputImageSizeType &size = this->GetInput()->GetRequestedRegion().GetSize();
    int dim = InputImageType::ImageDimension;
    NodeNumType node = 0;
    if (dim == 2)
    {
      node = (coord[1] * size[0]) + coord[0];
    }
    if (dim == 3)
    {
      node = (coord[2] * size[0] * size[1]) + (coord[1] * size[0]) + coord[0];
    }
    if ((m_Graph_NumberOfNodes > 0) && (node >= m_Graph_NumberOfNodes))
    {
      /*MITK_INFO << "WARNING! Coordinates outside image!";
      MITK_INFO << "Coords = " << coord ;
      MITK_INFO << "ImageDim = " << dim ;
      MITK_INFO << "RequestedRegionSize = " << size ;*/
      node = 0;
    }

    return node;
  }

  template <class TInputImageType, class TOutputImageType>
  inline bool ShortestPathImageFilter<TInputImageType, TOutputImageType>::CoordIsInBounds(IndexType coord)
  {
    const InputImageSizeType &size = this->GetInput()->GetRequestedRegion().GetSize();
    int dim = InputImageType::ImageDimension;

    if (dim == 2)
    {
      if ((coord[0] >= 0) && ((unsigned long)coord[0] < size[0]) && (coord[1] >= 0) &&
          ((unsigned long)coord[1] < size[1]))
      {
        return true;
      }
    }
    if (dim == 3)
    {
      if ((coord[0] >= 0) && ((unsigned long)coord[0] < size[0]) && (coord[1] >= 0) &&
          ((unsigned long)coord[1] < size[1]) && (coord[2] >= 0) && ((unsigned long)coord[2] < size[2]))
      {
        return true;
      }
    }
    return false;
  }

  template <class TInputImageType, class TOutputImageType>
  inline std::vector<ShortestPathNode *> ShortestPathImageFilter<TInputImageType, TOutputImageType>::GetNeighbors(
    unsigned int nodeNum, bool FullNeighbors)
  {
    // returns a vector of nodepointers.. these nodes are the neighbors
    int dim = InputImageType::ImageDimension;
    IndexType Coord = NodeToCoord(nodeNum);
    IndexType NeighborCoord;
    std::vector<ShortestPathNode *> nodeList;

    int neighborDistance = 1; // if i increase that, i might not hit the endnote

    // maybe use itkNeighborhoodIterator here, might be faster

    if (dim == 2)
    {
      // N4
      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1] - neighborDistance;
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0] + neighborDistance;
      NeighborCoord[1] = Coord[1];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1] + neighborDistance;
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0] - neighborDistance;
      NeighborCoord[1] = Coord[1];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      if (FullNeighbors)
      {
        // N8
        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);
      }
    }
    if (dim == 3)
    {
      // N6
      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1] - neighborDistance;
      NeighborCoord[2] = Coord[2];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0] + neighborDistance;
      NeighborCoord[1] = Coord[1];
      NeighborCoord[2] = Coord[2];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1] + neighborDistance;
      NeighborCoord[2] = Coord[2];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0] - neighborDistance;
      NeighborCoord[1] = Coord[1];
      NeighborCoord[2] = Coord[2];
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1];
      NeighborCoord[2] = Coord[2] + neighborDistance;
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      NeighborCoord[0] = Coord[0];
      NeighborCoord[1] = Coord[1];
      NeighborCoord[2] = Coord[2] - neighborDistance;
      if (CoordIsInBounds(NeighborCoord))
        nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

      if (FullNeighbors)
      {
        // N26
        // Middle Slice
        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2];
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2];
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2];
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2];
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        // BackSlice (Diagonal)
        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        // BackSlice (Non-Diag)
        NeighborCoord[0] = Coord[0];
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1];
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0];
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1];
        NeighborCoord[2] = Coord[2] - neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        // FrontSlice (Diagonal)
        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        // FrontSlice(Non-Diag)
        NeighborCoord[0] = Coord[0];
        NeighborCoord[1] = Coord[1] - neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] + neighborDistance;
        NeighborCoord[1] = Coord[1];
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0];
        NeighborCoord[1] = Coord[1] + neighborDistance;
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);

        NeighborCoord[0] = Coord[0] - neighborDistance;
        NeighborCoord[1] = Coord[1];
        NeighborCoord[2] = Coord[2] + neighborDistance;
        if (CoordIsInBounds(NeighborCoord))
          nodeList.push_back(&m_Nodes[CoordToNode(NeighborCoord)]);
      }
    }
    return nodeList;
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::SetStartIndex(
    const typename TInputImageType::IndexType &StartIndex)
  {
    for (unsigned int i = 0; i < TInputImageType::ImageDimension; ++i)
    {
      m_StartIndex[i] = StartIndex[i];
    }
    m_Graph_StartNode = CoordToNode(m_StartIndex);
    // MITK_INFO << "StartIndex = " << StartIndex;
    // MITK_INFO << "StartNode = " << m_Graph_StartNode;
    m_Initialized = false;
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::SetEndIndex(
    const typename TInputImageType::IndexType &EndIndex)
  {
    for (unsigned int i = 0; i < TInputImageType::ImageDimension; ++i)
    {
      m_EndIndex[i] = EndIndex[i];
    }
    m_Graph_EndNode = CoordToNode(m_EndIndex);
    // MITK_INFO << "EndNode = " << m_Graph_EndNode;
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::AddEndIndex(
    const typename TInputImageType::IndexType &index)
  {
    // ONLY FOR MULTIPLE END POINTS SEARCH
    IndexType newEndIndex;
    for (unsigned int i = 0; i < TInputImageType::ImageDimension; ++i)
    {
      newEndIndex[i] = index[i];
    }
    m_endPoints.push_back(newEndIndex);
    SetEndIndex(m_endPoints[0]);
    multipleEndPoints = true;
  }

  template <class TInputImageType, class TOutputImageType>
  inline double ShortestPathImageFilter<TInputImageType, TOutputImageType>::getEstimatedCostsToTarget(
    const typename TInputImageType::IndexType &a)
  {
    // Returns the minimal possible costs for a path from "a" to targetnode.
    itk::Vector<float, 3> v;
    v[0] = m_EndIndex[0] - a[0];
    v[1] = m_EndIndex[1] - a[1];
    v[2] = m_EndIndex[2] - a[2];

    return m_CostFunction->GetMinCost() * v.GetNorm();
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::InitGraph()
  {
    if (!m_Initialized)
    {
      // Clean up previous stuff
      CleanUp();

      // Calc Number of nodes
      auto imageDimensions = TInputImageType::ImageDimension;
      const InputImageSizeType &size = this->GetInput()->GetRequestedRegion().GetSize();
      m_Graph_NumberOfNodes = 1;
      for (NodeNumType i = 0; i < imageDimensions; ++i)
        m_Graph_NumberOfNodes = m_Graph_NumberOfNodes * size[i];

      // Initialize mainNodeList with that number
      m_Nodes = new ShortestPathNode[m_Graph_NumberOfNodes];

      // Initialize each node in nodelist
      for (NodeNumType i = 0; i < m_Graph_NumberOfNodes; i++)
      {
        m_Nodes[i].distAndEst = -1;
        m_Nodes[i].distance = -1;
        m_Nodes[i].prevNode = -1;
        m_Nodes[i].mainListIndex = i;
        m_Nodes[i].closed = false;
      }

      m_Initialized = true;
    }

    // In the beginning, the Startnode needs a distance of 0
    m_Nodes[m_Graph_StartNode].distance = 0;
    m_Nodes[m_Graph_StartNode].distAndEst = 0;

    // initalize cost function
    m_CostFunction->Initialize();
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::StartShortestPathSearch()
  {
    // Setup Timer
    clock_t startAll = clock();
    clock_t stopAll = clock();

    // init variables
    double durationAll = 0;
    bool timeout = false;
    NodeNumType mainNodeListIndex = 0;
    DistanceType curNodeDistance = 0;
    NodeNumType numberOfNodesChecked = 0;

    // Create Multimap (tree structure for fast searching)
    std::multimap<double, ShortestPathNode *> myMap;
    std::pair<std::multimap<double, ShortestPathNode *>::iterator, std::multimap<double, ShortestPathNode *>::iterator>
      ret;
    std::multimap<double, ShortestPathNode *>::iterator it;

    // At first, only startNote is discovered.
    myMap.insert(
      std::pair<double, ShortestPathNode *>(m_Nodes[m_Graph_StartNode].distAndEst, &m_Nodes[m_Graph_StartNode]));

    // While there are discovered Nodes, pick the one with lowest distance,
    // update its neighbors and eventually delete it from the discovered Nodes list.
    while (!myMap.empty())
    {
      numberOfNodesChecked++;
      /*if ( (numberOfNodesChecked % (m_Graph_NumberOfNodes/100)) == 0)
      {
        MITK_INFO << "Checked " << ( numberOfNodesChecked / (m_Graph_NumberOfNodes/100) ) << "% List: " << myMap.size()
      << "\n";
      }*/

      // Get element with lowest score
      mainNodeListIndex = myMap.begin()->second->mainListIndex;
      curNodeDistance = myMap.begin()->second->distance;
      myMap.begin()->second->closed = true; // close it

      // Debug:
      // MITK_INFO << "INFO: size " << myMap.size();
      /*
      for (it = myMap.begin(); it != myMap.end(); ++it)
      {
      MITK_INFO << "(1) " << it->first << "|" << it->second->distAndEst << "|"<<it->second->mainListIndex;
      }
      */

      // Kicks out element with lowest score
      myMap.erase(myMap.begin());

      // if wanted, store vector order
      if (m_StoreVectorOrder)
      {
        m_VectorOrder.push_back(mainNodeListIndex);
      }

      // Check neighbors
      std::vector<ShortestPathNode *> neighborNodes = GetNeighbors(mainNodeListIndex, m_Graph_fullNeighbors);
      for (NodeNumType i = 0; i < neighborNodes.size(); i++)
      {
        if (neighborNodes[i]->closed)
          continue; // this nodes is already closed, go to next neighbor

        IndexType coordCurNode = NodeToCoord(mainNodeListIndex);
        IndexType coordNeighborNode = NodeToCoord(neighborNodes[i]->mainListIndex);

        // calculate the new Distance to the current neighbor
        double newDistance = curNodeDistance + (m_CostFunction->GetCost(coordCurNode, coordNeighborNode));

        // if it is shorter than any yet known path to this neighbor, than the current path is better. Save that!
        if ((newDistance < neighborNodes[i]->distance) || (neighborNodes[i]->distance == -1))
        {
          // if that neighbornode is not in discoverednodeList yet, Push it there and update
          if (neighborNodes[i]->distance == -1)
          {
            neighborNodes[i]->distance = newDistance;
            neighborNodes[i]->distAndEst = newDistance + getEstimatedCostsToTarget(coordNeighborNode);
            neighborNodes[i]->prevNode = mainNodeListIndex;
            myMap.insert(std::pair<double, ShortestPathNode *>(m_Nodes[neighborNodes[i]->mainListIndex].distAndEst,
                                                               &m_Nodes[neighborNodes[i]->mainListIndex]));
            /*
            MITK_INFO << "Inserted: " << m_Nodes[neighborNodes[i]->mainListIndex].distAndEst << "|" <<
            m_Nodes[neighborNodes[i]->mainListIndex].mainListIndex;
            MITK_INFO << "INFO: size " << myMap.size();
            for (it = myMap.begin(); it != myMap.end(); ++it)
            {
            MITK_INFO << "(1) " << it->first << "|" << it->second->distAndEst << "|"<<it->second->mainListIndex;
            }
            */
          }
          // or if is already in discoverednodelist, update
          else
          {
            /*
            it = myMap.find(neighborNodes[i]->distAndEst);
            if (it == myMap.end() )
            {
            MITK_INFO << "Nothing!";
            // look further
            for (it = myMap.begin(); it != myMap.end(); ++it)
            {
            if ((*it).second->mainListIndex == lookForId)
            {
            MITK_INFO << "But it is there!!!";
            MITK_INFO << "Searched for: " << lookFor << "   but had: " << (*it).second->distAndEst;
            }

            }
            }
            */

            // 1st : find and delete old element
            bool found = false;
            ret = myMap.equal_range(neighborNodes[i]->distAndEst);

            if ((ret.first == ret.second))
            {
              /*+++++++++++++ no exact match +++++++++++++*/
              // MITK_INFO << "No exact match!"; // if this happens, you are screwed
              /*
              MITK_INFO << "Was looking for: " << lookFor << "  ID: " << lookForId;
              if (ret.first != myMap.end() )
              {
              it = ret.first;
              MITK_INFO << "Found: " << it->first << "  ID: " << it->second->mainListIndex;
              ++it;
              MITK_INFO << "Found: " << it->first << "  ID: " << it->second->mainListIndex;
              --it;
              --it;
              MITK_INFO << "Found: " << it->first << "  ID: " << it->second->mainListIndex;
              }

              // look if that ID is found in the map
              for (it = myMap.begin(); it != myMap.end(); ++it)
              {
              if ((*it).second->mainListIndex == lookForId)
              {
              MITK_INFO << "But it is there!!!";
              MITK_INFO << "Searched dist: " << lookFor << "  found dist: " << (*it).second->distAndEst;
              }

              }
              */
            }
            else
            {
              for (it = ret.first; it != ret.second; ++it)
              {
                if (it->second->mainListIndex == neighborNodes[i]->mainListIndex)
                {
                  found = true;
                  myMap.erase(it);
                  /*
                  MITK_INFO << "INFO: size " << myMap.size();
                  MITK_INFO << "Erase: " << it->first << "|" << it->second->mainListIndex;

                  MITK_INFO << "INFO: size " << myMap.size();
                  for (it = myMap.begin(); it != myMap.end(); ++it)
                  {
                  MITK_INFO << "(1) " << it->first << "|" << it->second->distAndEst << "|"<<it->second->mainListIndex;
                  }
                  */
                  break;
                }
              }
            }

            if (!found)
            {
              // MITK_INFO << "Could not find it! :(";
              continue;
            }

            // 2nd: update and insert new element
            neighborNodes[i]->distance = newDistance;
            neighborNodes[i]->distAndEst = newDistance + getEstimatedCostsToTarget(coordNeighborNode);
            neighborNodes[i]->prevNode = mainNodeListIndex;
            // myMap.insert( std::pair<double,ShortestPathNode*> (neighborNodes[i]->distAndEst, neighborNodes[i]));
            myMap.insert(std::pair<double, ShortestPathNode *>(m_Nodes[neighborNodes[i]->mainListIndex].distAndEst,
                                                               &m_Nodes[neighborNodes[i]->mainListIndex]));

            // MITK_INFO << "Re-Inserted: " << m_Nodes[neighborNodes[i]->mainListIndex].distAndEst << "|" <<
            // m_Nodes[neighborNodes[i]->mainListIndex].mainListIndex;
            // MITK_INFO << "INFO: size " << myMap.size();
            /*for (it = myMap.begin(); it != myMap.end(); ++it)
            {
            MITK_INFO << "(1) " << it->first << "|" << it->second->distAndEst << "|"<<it->second->mainListIndex;
            }*/
          }
        }
      }
      // finished with checking all neighbors.

      // Check Timeout, if activated
      if (m_ActivateTimeOut)
      {
        stopAll = clock();
        durationAll = (double)(stopAll - startAll) / CLOCKS_PER_SEC;
        if (durationAll >= 30)
        {
          // MITK_INFO << "TIMEOUT!! Search took over 30 seconds";
          timeout = true;
        }
      }

      // Check end criteria:
      // For multiple points
      if (multipleEndPoints)
      {
        // super slow, make it faster
        for (unsigned int i = 0; i < m_endPoints.size(); i++)
        {
          if (CoordToNode(m_endPoints[i]) == mainNodeListIndex)
          {
            m_endPointsClosed.push_back(NodeToCoord(mainNodeListIndex));
            m_endPoints.erase(m_endPoints.begin() + i);
            if (m_endPoints.empty())
            {
              // Finished! break
              return;
            }
            if (m_Graph_EndNode == mainNodeListIndex)
            {
              // set new end
              SetEndIndex(m_endPoints[0]);
            }
          }
        }
      }
      // if single end point, then end, if this one is reached or timeout happened.
      else if ((mainNodeListIndex == m_Graph_EndNode || timeout) && !m_CalcAllDistances)
      {
        /*if (m_StoreVectorOrder)
          MITK_INFO << "Number of Nodes checked: " << m_VectorOrder.size() ;*/
        return;
      }
    }
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::MakeOutputs()
  {
    // MITK_INFO << "Make Output";

    if (m_MakeOutputImage)
    {
      OutputImagePointer output0 = this->GetOutput(0);
      output0->SetRegions(this->GetInput()->GetLargestPossibleRegion());
      output0->Allocate();
      OutputImageIteratorType shortestPathImageIt(output0, output0->GetRequestedRegion());
      // Create ShortestPathImage (Output 0)
      for (shortestPathImageIt.GoToBegin(); !shortestPathImageIt.IsAtEnd(); ++shortestPathImageIt)
      {
        // First intialize with background color
        shortestPathImageIt.Set(BACKGROUND);
      }

      if (!multipleEndPoints) // Only one path was calculated
      {
        for (unsigned int i = 0; i < m_VectorPath.size(); i++)
        {
          shortestPathImageIt.SetIndex(m_VectorPath[i]);
          shortestPathImageIt.Set(FOREGROUND);
        }
      }
      else // multiple pathes has been calculated, draw all
      {
        for (unsigned int i = 0; i < m_MultipleVectorPaths.size(); i++)
        {
          for (unsigned int j = 0; j < m_MultipleVectorPaths[i].size(); j++)
          {
            shortestPathImageIt.SetIndex(m_MultipleVectorPaths[i][j]);
            shortestPathImageIt.Set(FOREGROUND);
          }
        }
      }
    }
  }

  template <class TInputImageType, class TOutputImageType>
  typename ShortestPathImageFilter<TInputImageType, TOutputImageType>::OutputImagePointer
    ShortestPathImageFilter<TInputImageType, TOutputImageType>::GetVectorOrderImage()
  {
    // Create Vector Order Image
    // Return it

    OutputImagePointer image = OutputImageType::New();
    image->SetRegions(this->GetInput()->GetLargestPossibleRegion());
    image->Allocate();
    OutputImageIteratorType vectorOrderImageIt(image, image->GetRequestedRegion());

    // MITK_INFO << "GetVectorOrderImage";
    for (vectorOrderImageIt.GoToBegin(); !vectorOrderImageIt.IsAtEnd(); ++vectorOrderImageIt)
    {
      // First intialize with background color
      vectorOrderImageIt.Value() = BACKGROUND;
    }
    for (int i = 0; i < m_VectorOrder.size(); i++)
    {
      vectorOrderImageIt.SetIndex(NodeToCoord(m_VectorOrder[i]));
      vectorOrderImageIt.Set(BACKGROUND + i);
    }
    return image;
  }

  template <class TInputImageType, class TOutputImageType>
  typename ShortestPathImageFilter<TInputImageType, TOutputImageType>::OutputImagePointer
    ShortestPathImageFilter<TInputImageType, TOutputImageType>::GetDistanceImage()
  {
    // Create Distance Image
    // Return it

    OutputImagePointer image = OutputImageType::New();
    image->SetRegions(this->GetInput()->GetLargestPossibleRegion());
    image->Allocate();
    ;
    OutputImageIteratorType distanceImageIt(image, image->GetRequestedRegion());
    // Create Distance Image (Output 1)
    NodeNumType myNodeNum;
    for (distanceImageIt.GoToBegin(); !distanceImageIt.IsAtEnd(); ++distanceImageIt)
    {
      IndexType index = distanceImageIt.GetIndex();
      myNodeNum = CoordToNode(index);
      double newVal = m_Nodes[myNodeNum].distance;
      distanceImageIt.Set(newVal);
    }
  }

  template <class TInputImageType, class TOutputImageType>
  std::vector<typename ShortestPathImageFilter<TInputImageType, TOutputImageType>::IndexType>
    ShortestPathImageFilter<TInputImageType, TOutputImageType>::GetVectorPath()
  {
    return m_VectorPath;
  }

  template <class TInputImageType, class TOutputImageType>
  std::vector<std::vector<typename ShortestPathImageFilter<TInputImageType, TOutputImageType>::IndexType>>
    ShortestPathImageFilter<TInputImageType, TOutputImageType>::GetMultipleVectorPaths()
  {
    return m_MultipleVectorPaths;
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::MakeShortestPathVector()
  {
    // MITK_INFO << "Make ShortestPath Vec";

    // single end point
    if (!multipleEndPoints)
    {
      // fill m_VectorPath with the Shortest Path
      m_VectorPath.clear();

      // Go backwards from endnote to startnode
      NodeNumType prevNode = m_Graph_EndNode;
      while (prevNode != m_Graph_StartNode)
      {
        m_VectorPath.push_back(NodeToCoord(prevNode));
        prevNode = m_Nodes[prevNode].prevNode;
      }
      m_VectorPath.push_back(NodeToCoord(prevNode));
      // reverse it
      std::reverse(m_VectorPath.begin(), m_VectorPath.end());
    }
    // Multiple end end points and pathes
    else
    {
      for (unsigned int i = 0; i < m_endPointsClosed.size(); i++)
      {
        m_VectorPath.clear();
        // Go backwards from endnote to startnode
        NodeNumType prevNode = CoordToNode(m_endPointsClosed[i]);
        while (prevNode != m_Graph_StartNode)
        {
          m_VectorPath.push_back(NodeToCoord(prevNode));
          prevNode = m_Nodes[prevNode].prevNode;
        }
        m_VectorPath.push_back(NodeToCoord(prevNode));

        // reverse it
        std::reverse(m_VectorPath.begin(), m_VectorPath.end());
        // push_back
        m_MultipleVectorPaths.push_back(m_VectorPath);
      }
    }
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::CleanUp()
  {
    m_VectorOrder.clear();
    m_VectorPath.clear();
    // TODO: if multiple Path, clear all multiple Paths

    if (m_Nodes)
      delete[] m_Nodes;
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::GenerateData()
  {
    // Build Graph
    InitGraph();

    // Calc Shortest Parth
    StartShortestPathSearch();

    // Fill Shortest Path
    MakeShortestPathVector();

    // Make Outputs
    MakeOutputs();
  }

  template <class TInputImageType, class TOutputImageType>
  void ShortestPathImageFilter<TInputImageType, TOutputImageType>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

} /* end namespace itk */

#endif // __itkShortestPathImageFilter_txx
