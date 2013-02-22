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
#include <mitkContourElement.h>

mitk::ContourElement::ContourElement()
{
  this->m_Vertices = new VertexListType();
  this->m_IsClosed = false;
}



mitk::ContourElement::ContourElement(const mitk::ContourElement &other) :
  m_Vertices(other.m_Vertices), m_IsClosed(other.m_IsClosed)
{
}



mitk::ContourElement::~ContourElement()
{
  delete this->m_Vertices;
}



void mitk::ContourElement::AddVertex(mitk::Point3D &vertex, bool isControlPoint)
{
  this->m_Vertices->push_back(new VertexType(vertex, isControlPoint));
}



void mitk::ContourElement::AddVertex(VertexType &vertex)
{
  this->m_Vertices->push_back(&vertex);
}



void mitk::ContourElement::AddVertexAtFront(mitk::Point3D &vertex, bool isControlPoint)
{
  this->m_Vertices->push_front(new VertexType(vertex, isControlPoint));
}



void mitk::ContourElement::AddVertexAtFront(VertexType &vertex)
{
  this->m_Vertices->push_front(&vertex);
}



void mitk::ContourElement::InsertVertexAtIndex(mitk::Point3D &vertex, bool isControlPoint, int index)
{
  if(index > 0 && this->GetSize() > index)
  {
    VertexIterator _where = this->m_Vertices->begin();
    _where += index;
    this->m_Vertices->insert(_where, new VertexType(vertex, isControlPoint));
  }
}



mitk::ContourElement::VertexType* mitk::ContourElement::GetVertexAt(int index)
{
  return this->m_Vertices->at(index);
}



mitk::ContourElement::VertexType* mitk::ContourElement::GetVertexAt(const mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should some kind of an octree with spatial query*/

  if(eps > 0)
  {
    if(true ) //currently no method with better performance is available
    {
      return BruteForceGetVertexAt(point, eps);
    }
    else
    {
      return OptimizedGetVertexAt(point, eps);
    }
  }//if eps < 0
  return NULL;
}



mitk::ContourElement::VertexType* mitk::ContourElement::BruteForceGetVertexAt(const mitk::Point3D &point, float eps)
{
  if(eps > 0)
  {
    std::deque< std::pair<double, VertexType*> > nearestlist;

    ConstVertexIterator it = this->m_Vertices->begin();

    ConstVertexIterator end = this->m_Vertices->end();

    while(it != end)
    {
      mitk::Point3D currentPoint = (*it)->Coordinates;

      double distance = currentPoint.EuclideanDistanceTo(point);
      if(distance < eps)
      {
        //if list is emtpy, add point to list
        if(nearestlist.size() < 1)
        {
          nearestlist.push_front(std::pair<double, VertexType*>( (*it)->Coordinates.EuclideanDistanceTo(point), (*it) ));
        }
        //found an approximate point - check if current is closer then first in nearestlist
        else if( distance < nearestlist.front().first )
        {
          //found even closer vertex
          nearestlist.push_front(std::pair<double, VertexType*>( (*it)->Coordinates.EuclideanDistanceTo(point), (*it) ));
        }
      }//if distance > eps

      it++;
    }//while
    if(nearestlist.size() > 0)
    {
      /*++++++++++++++++++++ return the nearest active point if one was found++++++++++++++++++*/
      std::deque< std::pair<double, VertexType*> >::iterator it = nearestlist.begin();
      std::deque< std::pair<double, VertexType*> >::iterator end = nearestlist.end();
      while(it != end)
      {
        if( (*it).second->IsControlPoint )
        {
          return (*it).second;
        }
        it++;
      }
      /*---------------------------------------------------------------------------------------*/

      //return closest point
      return nearestlist.front().second;
    }
  }
  return NULL;
}



mitk::ContourElement::VertexType* mitk::ContourElement::OptimizedGetVertexAt(const mitk::Point3D &point, float eps)
{
  if( (eps > 0) && (this->m_Vertices->size()>0) )
  {
      int k = 1;
      int dim = 3;
      int nPoints = this->m_Vertices->size();
      ANNpointArray pointsArray;
      ANNpoint queryPoint;
      ANNidxArray indexArray;
      ANNdistArray distanceArray;
      ANNkd_tree* kdTree;

      queryPoint = annAllocPt(dim);
      pointsArray = annAllocPts(nPoints, dim);
      indexArray = new ANNidx[k];
      distanceArray = new ANNdist[k];


       int i = 0;

      //fill points array with our control points
      for(VertexIterator it = this->m_Vertices->begin(); it != this->m_Vertices->end(); it++, i++)
      {
        mitk::Point3D cur = (*it)->Coordinates;
        pointsArray[i][0]= cur[0];
        pointsArray[i][1]= cur[1];
        pointsArray[i][2]= cur[2];
      }

      //create the kd tree
      kdTree = new ANNkd_tree(pointsArray,nPoints, dim);

      //fill mitk::Point3D into ANN query point
      queryPoint[0] = point[0];
      queryPoint[1] = point[1];
      queryPoint[2] = point[2];

      //k nearest neighbour search
      kdTree->annkSearch(queryPoint, k, indexArray, distanceArray, eps);

      VertexType* ret = NULL;

      try
      {
        ret = this->m_Vertices->at(indexArray[0]);
      }
      catch(std::out_of_range ex)
      {
        //ret stays NULL
        return ret;
      }

      //clean up ANN
      delete [] indexArray;
      delete [] distanceArray;
      delete kdTree;
      annClose();

      return ret;
  }
  return NULL;
}



mitk::ContourElement::VertexListType* mitk::ContourElement::GetVertexList()
{
  return this->m_Vertices;
}



bool mitk::ContourElement::IsClosed()
{
  return this->m_IsClosed;
}



void mitk::ContourElement::Close()
{
  this->m_IsClosed = true;
}



void mitk::ContourElement::Open()
{
  this->m_IsClosed = false;
}



void mitk::ContourElement::SetIsClosed( bool isClosed)
{
  isClosed ? this->Close() : this->Open();
}



void mitk::ContourElement::Concatenate(mitk::ContourElement* other)
{
  if( other->GetSize() > 0)
  {
    ConstVertexIterator it =  other->m_Vertices->begin();
    ConstVertexIterator end =  other->m_Vertices->end();
    //add all vertices of other after last vertex
    while(it != end)
    {
      this->m_Vertices->push_back(*it);
      it++;
    }
  }
}



bool mitk::ContourElement::RemoveVertex(mitk::ContourElement::VertexType* vertex)
{
  VertexIterator it = this->m_Vertices->begin();

  VertexIterator end = this->m_Vertices->end();

  //search for vertex and remove it if exists
  while(it != end)
  {
    if((*it) == vertex)
    {
      this->m_Vertices->erase(it);
      return true;
    }

    it++;
  }

  return false;
}



bool mitk::ContourElement::RemoveVertexAt(int index)
{
  if( index >= 0 && index < this->m_Vertices->size() )
  {
    this->m_Vertices->erase(this->m_Vertices->begin()+index);
    return true;
  }
  else
  {
    return false;
  }
}



bool mitk::ContourElement::RemoveVertexAt(mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should be some kind of an octree with spatial query*/

  if(eps > 0){
    VertexIterator it = this->m_Vertices->begin();

    VertexIterator end = this->m_Vertices->end();

    while(it != end)
    {
      mitk::Point3D currentPoint = (*it)->Coordinates;

      if(currentPoint.EuclideanDistanceTo(point) < eps)
      {
        //approximate point found
        //now erase it
        this->m_Vertices->erase(it);
        return true;
      }

      it++;
    }
  }
  return false;
}



void mitk::ContourElement::Clear()
{
  this->m_Vertices->clear();
}
