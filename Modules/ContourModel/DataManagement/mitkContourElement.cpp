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
#include <vtkMath.h>
#include <algorithm>

mitk::ContourElement::ContourElement()
{
  this->m_Vertices = new VertexListType();
  this->m_IsClosed = false;
}



mitk::ContourElement::ContourElement(const mitk::ContourElement &other) :
  itk::LightObject(),
  m_Vertices(other.m_Vertices),
  m_IsClosed(other.m_IsClosed)
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
  if(index >= 0 && this->GetSize() > index)
  {
    auto _where = this->m_Vertices->begin();
    _where += index;
    this->m_Vertices->insert(_where, new VertexType(vertex, isControlPoint));
  }
}



void mitk::ContourElement::SetVertexAt(int pointId, const Point3D &point)
{
  if(pointId >= 0 && this->GetSize() > pointId)
  {
    this->m_Vertices->at(pointId)->Coordinates = point;
  }
}



void mitk::ContourElement::SetVertexAt(int pointId, const VertexType* vertex)
{
  if(pointId >= 0 && this->GetSize() > pointId)
  {
    this->m_Vertices->at(pointId)->Coordinates = vertex->Coordinates;
    this->m_Vertices->at(pointId)->IsControlPoint = vertex->IsControlPoint;
  }
}



mitk::ContourElement::VertexType* mitk::ContourElement::GetVertexAt(int index)
{
  return this->m_Vertices->at(index);
}

bool mitk::ContourElement::IsEmpty()
{
  return this->m_Vertices->empty();
}


mitk::ContourElement::VertexType* mitk::ContourElement::GetVertexAt(const mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should some kind of an octree with spatial query*/

  if(eps > 0)
  {
    //currently no method with better performance is available
    return BruteForceGetVertexAt(point, eps);
  }//if eps < 0
  return nullptr;
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
      auto it = nearestlist.begin();
      auto end = nearestlist.end();
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
  return nullptr;
}



/*mitk::ContourElement::VertexType* mitk::ContourElement::OptimizedGetVertexAt(const mitk::Point3D &point, float eps)
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
*/



mitk::ContourElement::VertexListType* mitk::ContourElement::GetVertexList()
{
  return this->m_Vertices;
}



bool mitk::ContourElement::IsClosed()
{
  return this->m_IsClosed;
}

bool mitk::ContourElement::IsNearContour(const mitk::Point3D &point, float eps)
{
    ConstVertexIterator it1 = this->m_Vertices->begin();
    ConstVertexIterator it2 = this->m_Vertices->begin();
    it2 ++; // it2 runs one position ahead

    ConstVertexIterator end = this->m_Vertices->end();

    int counter = 0;

    for (; it1 != end; it1++, it2++, counter++)
    {
      if (it2 == end)
          it2 = this->m_Vertices->begin();

      mitk::Point3D v1 = (*it1)->Coordinates;
      mitk::Point3D v2 = (*it2)->Coordinates;

      const float l2 = v1.SquaredEuclideanDistanceTo(v2);

      mitk::Vector3D p_v1 = point - v1;
      mitk::Vector3D v2_v1 = v2 - v1;

      double tc = (p_v1 * v2_v1) / l2;

      // take into account we have line segments and not (infinite) lines
      if (tc < 0.0) tc = 0.0;
      if (tc > 1.0) tc = 1.0;

      mitk::Point3D crossPoint = v1 + v2_v1 * tc;

      double distance = point.SquaredEuclideanDistanceTo(crossPoint);

      if (distance < eps)
      {
          return true;
      }
    }

    return false;
}


void mitk::ContourElement::Close()
{
  this->m_IsClosed = true;
}



void mitk::ContourElement::Open()
{
  this->m_IsClosed = false;
}



void mitk::ContourElement::SetClosed( bool isClosed)
{
  isClosed ? this->Close() : this->Open();
}

mitk::ContourElement::VertexListType*
mitk::ContourElement::GetControlVertices()
{
    auto   newVertices = new VertexListType();

    auto it = this->m_Vertices->begin();
    auto end = this->m_Vertices->end();

    while(it != end)
    {
        if((*it)->IsControlPoint)
        {
           newVertices->push_back((*it));
        }
        it++;
    }

    return newVertices;
}

void mitk::ContourElement::Concatenate(mitk::ContourElement* other, bool check)
{
  if( other->GetSize() > 0)
  {
    ConstVertexIterator otherIt =  other->m_Vertices->begin();
    ConstVertexIterator otherEnd =  other->m_Vertices->end();
    while(otherIt != otherEnd)
    {
      if (check)
      {
          ConstVertexIterator thisIt =  this->m_Vertices->begin();
          ConstVertexIterator thisEnd =  this->m_Vertices->end();

          bool found = false;
          while(thisIt != thisEnd)
          {
              if ( (*thisIt)->Coordinates == (*otherIt)->Coordinates )
              {
                  found = true;
                  break;
              }

              thisIt++;
          }
          if (!found)
              this->m_Vertices->push_back(*otherIt);
      }
      else
      {
        this->m_Vertices->push_back(*otherIt);
      }
      otherIt++;
    }
  }
}

bool mitk::ContourElement::RemoveVertex(const VertexType *vertex)
{
  auto it = this->m_Vertices->begin();

  auto end = this->m_Vertices->end();

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



int mitk::ContourElement::GetIndex(const VertexType *vertex)
{
  auto it = this->m_Vertices->begin();

  auto end = this->m_Vertices->end();

  int index = 0;

  //search for vertex
  while(it != end)
  {
    if((*it) == vertex)
    {
      return index;
    }

    it++;
    ++index;
  }

  return -1;//not found
}



bool mitk::ContourElement::RemoveVertexAt(int index)
{
  if( index >= 0 && static_cast<VertexListType::size_type>(index) < this->m_Vertices->size() )
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
    auto it = this->m_Vertices->begin();

    auto end = this->m_Vertices->end();

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
//----------------------------------------------------------------------
void mitk::ContourElement::RedistributeControlVertices(const VertexType* selected, int period)
{
    int counter = 0;
    auto _where = this->m_Vertices->begin();

    if (selected != nullptr)
    {
        while (_where != this->m_Vertices->end())
        {
            if ((*_where) == selected)
            {
                break;
            }
            _where++;
        }
    }

    auto _iter = _where;
    while (_iter != this->m_Vertices->end())
    {
        div_t divresult;
        divresult = div (counter,period);
        (*_iter)->IsControlPoint = (divresult.rem == 0);
        counter++;
        _iter++;
    }

    _iter = _where;
    counter = 0;
    while (_iter != this->m_Vertices->begin())
    {
        div_t divresult;
        divresult = div (counter,period);
        (*_iter)->IsControlPoint = (divresult.rem == 0);
        counter++;
        _iter--;
    }
}
