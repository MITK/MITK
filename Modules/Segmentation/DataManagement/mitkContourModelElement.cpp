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
#include <mitkContourModelElement.h>

mitk::ContourModelElement::ContourModelElement()
{
  this->m_Vertices = new VertexListType();
  this->m_IsClosed = false;
}



mitk::ContourModelElement::ContourModelElement(const mitk::ContourModelElement &other) :
  m_Vertices(other.m_Vertices), m_IsClosed(other.m_IsClosed)
{
}



mitk::ContourModelElement::~ContourModelElement()
{
  delete this->m_Vertices;
}



void mitk::ContourModelElement::AddVertex(mitk::Point3D &vertex, bool isActive)
{
  this->m_Vertices->push_back(new VertexType(vertex, isActive));
}



void mitk::ContourModelElement::AddVertex(VertexType &vertex)
{
  this->m_Vertices->push_back(&vertex);
}



void mitk::ContourModelElement::InsertVertexAtIndex(mitk::Point3D &vertex, bool isActive, int index)
{
  if(index > 0 && this->GetSize() > index)
  {
    VertexIterator _where = this->m_Vertices->begin();
    _where += index;
    this->m_Vertices->insert(_where, new VertexType(vertex, isActive));
  }
}



mitk::ContourModelElement::VertexType* mitk::ContourModelElement::GetVertexAt(int index)
{
  return this->m_Vertices->at(index);
}



mitk::ContourModelElement::VertexType* mitk::ContourModelElement::GetVertexAt(const mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should some kind of an octree with spatial query*/

  if(eps > 0)
  {
    if(this->m_Vertices->size() == 1)
    {
      return this->m_Vertices->at(0);
    }

    if(this->m_Vertices->size() < 10000)
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



mitk::ContourModelElement::VertexType* mitk::ContourModelElement::BruteForceGetVertexAt(const mitk::Point3D &point, float eps)
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
      //return closest point
      return nearestlist.front().second;
    }
  }
  return NULL;
}



mitk::ContourModelElement::VertexType* mitk::ContourModelElement::OptimizedGetVertexAt(const mitk::Point3D &point, float eps)
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



mitk::ContourModelElement::VertexListType* mitk::ContourModelElement::GetVertexList()
{
  return this->m_Vertices;
}



bool mitk::ContourModelElement::IsClosed()
{
  return this->m_IsClosed;
}



void mitk::ContourModelElement::Close()
{
  this->m_IsClosed = true;
}



void mitk::ContourModelElement::Open()
{
  this->m_IsClosed = false;
}



void mitk::ContourModelElement::SetIsClosed( bool isClosed)
{
  isClosed ? this->Close() : this->Open();
}



void mitk::ContourModelElement::Concatenate(mitk::ContourModelElement* other)
{
  ConstVertexIterator it =  other->m_Vertices->begin();
  ConstVertexIterator end =  other->m_Vertices->end();
  while(it != end)
  {
    this->m_Vertices->push_back(*it);
    it++;
  }
}



void mitk::ContourModelElement::RemoveVertex(mitk::ContourModelElement::VertexType* vertex)
{
  this->RemoveVertexAt(vertex->Coordinates, 0.1);
}



void mitk::ContourModelElement::RemoveVertexAt(int index)
{
  this->m_Vertices->erase(this->m_Vertices->begin()+index);
}



bool mitk::ContourModelElement::RemoveVertexAt(mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should be some kind of an octree with spatial query*/

  if(eps > 0){
    ConstVertexIterator it = this->m_Vertices->begin();

    ConstVertexIterator end = this->m_Vertices->end();

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