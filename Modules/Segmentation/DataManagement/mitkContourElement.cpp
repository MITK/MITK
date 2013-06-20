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



void mitk::ContourElement::SetIsClosed( bool isClosed)
{
  isClosed ? this->Close() : this->Open();
}

mitk::ContourElement::VertexListType*
mitk::ContourElement::GetControlVertices()
{
    VertexListType* newVertices = new VertexListType();

    VertexIterator it = this->m_Vertices->begin();
    VertexIterator end = this->m_Vertices->end();

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

std::pair<mitk::ContourElement::VertexIterator, mitk::ContourElement::VertexIterator>
mitk::ContourElement::FindFirstIntersection(mitk::ContourElement* other)
{
    VertexIterator thisIt =  this->m_Vertices->begin();
    VertexIterator thisEnd =  this->m_Vertices->end();

    VertexIterator otherEnd =  other->m_Vertices->end();

    std::pair<mitk::ContourElement::VertexIterator, mitk::ContourElement::VertexIterator> match(thisEnd,otherEnd);

    while (thisIt != thisEnd)
    {
        VertexIterator otherIt =  other->m_Vertices->begin();

        while (otherIt != otherEnd)
        {
            if ( (*thisIt)->Coordinates == (*otherIt)->Coordinates )
            {
                match.first = thisIt;
                match.second = otherIt;
                return match;
            }

            otherIt++;
        }

        thisIt++;
    }

    return match;
}

void mitk::ContourElement::RemoveIntersections(mitk::ContourElement* other)
{
  if( other->GetSize() > 0)
  {

   // VertexIterator _where = this->FindFirstIntersection(other);
    std::pair<mitk::ContourElement::VertexIterator, mitk::ContourElement::VertexIterator> match = this->FindFirstIntersection(other);

    this->m_Vertices->erase(match.first, this->m_Vertices->end());
    other->m_Vertices->erase(other->m_Vertices->begin(), match.second );
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
//----------------------------------------------------------------------
void mitk::ContourElement::Interpolate()
{
    VertexListType* newVertices = new VertexListType();

    int nverts = this->m_Vertices->size();
    for (int i=0; (i+1)<nverts; i++)
    {
        this->DoBezierInterpolation(i, i+1, newVertices);
    }

    if ( this->IsClosed() )
    {
        this->DoBezierInterpolation(nverts-1, 0, newVertices);
    }

    delete this->m_Vertices;

    this->m_Vertices = newVertices;
}
//----------------------------------------------------------------------
void mitk::ContourElement::RedistributeControlVertices(const VertexType* selected, int period)
{
    int counter = 0;
    VertexIterator _where = this->m_Vertices->begin();

    if (selected != NULL)
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

    VertexIterator _iter = _where;
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
//----------------------------------------------------------------------
void mitk::ContourElement::DoBezierInterpolation( int idx1, int idx2, VertexListType* vertices )
{
  mitk::Point3D mp1;
  mp1 = this->m_Vertices->at(idx1)->Coordinates;

  mitk::Point3D mp2;
  mp2 = this->m_Vertices->at(idx2)->Coordinates;

//  this->RemoveIntermediateVerticesAt(idx1);
//  this->AddIntermediateVertex(mp1, idx1);
  vertices->push_back(new VertexType(mp1, this->m_Vertices->at(idx1)->IsControlPoint));

  int maxRecursion = 0;
  int tmp = 3;

  const int MaximumCurveLineSegments = 100;
  const double MaximumCurveError = 0.005;

  while ( 2*tmp < MaximumCurveLineSegments )
    {
    tmp *= 2;
    maxRecursion++;
    }

  // There are four control points with 3 components each, plus one
  // value for the recursion depth of this point
  double *controlPointsStack = new double[(3*4+1)*(maxRecursion+1)];
  int stackCount = 0;

  double slope1[3];
  double slope2[3];

  this->GetNthNodeSlope( idx1, slope1 );
  this->GetNthNodeSlope( idx2, slope2 );

  controlPointsStack[0] = 0;
  double *p1 = controlPointsStack+1;
  double *p2 = controlPointsStack+4;
  double *p3 = controlPointsStack+7;
  double *p4 = controlPointsStack+10;

  const VertexType* vertex1 = this->m_Vertices->at(idx1);

  p1[0] = vertex1->Coordinates[0];
  p1[1] = vertex1->Coordinates[1];
  p1[2] = vertex1->Coordinates[2];

  const VertexType* vertex2 = this->m_Vertices->at(idx2);

  p4[0] = vertex2->Coordinates[0];
  p4[1] = vertex2->Coordinates[1];
  p4[2] = vertex2->Coordinates[2];

  double distance = sqrt( vtkMath::Distance2BetweenPoints( p1, p4 ) );

  p2[0] = p1[0] + .333*distance*slope1[0];
  p2[1] = p1[1] + .333*distance*slope1[1];
  p2[2] = p1[2] + .333*distance*slope1[2];

  p3[0] = p4[0] - .333*distance*slope2[0];
  p3[1] = p4[1] - .333*distance*slope2[1];
  p3[2] = p4[2] - .333*distance*slope2[2];

  stackCount++;

  while ( stackCount )
    {
    //process last point on stack
    int recursionLevel = static_cast<int>(controlPointsStack[13*(stackCount-1)]);

    p1 = controlPointsStack + 13*(stackCount-1)+1;
    p2 = controlPointsStack + 13*(stackCount-1)+4;
    p3 = controlPointsStack + 13*(stackCount-1)+7;
    p4 = controlPointsStack + 13*(stackCount-1)+10;

    double totalDist = 0;
    totalDist += sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
    totalDist += sqrt(vtkMath::Distance2BetweenPoints(p2,p3));
    totalDist += sqrt(vtkMath::Distance2BetweenPoints(p3,p4));

    distance = sqrt(vtkMath::Distance2BetweenPoints(p1,p4));

    if ( recursionLevel >= maxRecursion || distance == 0 ||
         (totalDist - distance)/distance < MaximumCurveError )
      {
          mitk::Point3D mp2;
          mp2[0] = p2[0]; mp2[1] = p2[1]; mp2[2] = p2[2];
          //this->InsertVertexAtIndex(mp2, false, idx1);
          vertices->push_back(new VertexType(mp2, false));

          mitk::Point3D mp3;
          mp3[0] = p3[0]; mp3[1] = p3[1]; mp3[2] = p3[2];
          //this->InsertVertexAtIndex(mp3, false, idx1);
          vertices->push_back(new VertexType(mp3, false));

          if ( stackCount > 1 )
            {
              mitk::Point3D mp4;
              mp4[0] = p4[0]; mp4[1] = p4[1]; mp4[2] = p4[2];
              //this->InsertVertexAtIndex(mp4, false, idx1);
              vertices->push_back(new VertexType(mp4, false));
            }
          stackCount--;
      }
    else
      {
      double p12[3], p23[3], p34[3], p123[3], p234[3], p1234[3];

      this->ComputeMidpoint( p1, p2, p12 );
      this->ComputeMidpoint( p2, p3, p23 );
      this->ComputeMidpoint( p3, p4, p34 );
      this->ComputeMidpoint( p12, p23, p123 );
      this->ComputeMidpoint( p23, p34, p234 );
      this->ComputeMidpoint( p123, p234, p1234 );

      // add these two points to the stack
      controlPointsStack[13*(stackCount-1)] = recursionLevel+1;
      controlPointsStack[13*(stackCount)] = recursionLevel+1;

      double *newp1 = controlPointsStack + 13*(stackCount)+1;
      double *newp2 = controlPointsStack + 13*(stackCount)+4;
      double *newp3 = controlPointsStack + 13*(stackCount)+7;
      double *newp4 = controlPointsStack + 13*(stackCount)+10;

      newp1[0] = p1[0];
      newp1[1] = p1[1];
      newp1[2] = p1[2];

      newp2[0] = p12[0];
      newp2[1] = p12[1];
      newp2[2] = p12[2];

      newp3[0] = p123[0];
      newp3[1] = p123[1];
      newp3[2] = p123[2];

      newp4[0] = p1234[0];
      newp4[1] = p1234[1];
      newp4[2] = p1234[2];

      p1[0] = p1234[0];
      p1[1] = p1234[1];
      p1[2] = p1234[2];

      p2[0] = p234[0];
      p2[1] = p234[1];
      p2[2] = p234[2];

      p3[0] = p34[0];
      p3[1] = p34[1];
      p3[2] = p34[2];

      stackCount++;
      }
    }

  delete [] controlPointsStack;
}

//----------------------------------------------------------------------
bool mitk::ContourElement::GetNthNodeSlope( int index, double slope[3])
{
  if ( index < 0 ||
    static_cast<unsigned int>(index) >= this->m_Vertices->size() )
    {
    return false;
    }

  int idx1, idx2;

  if ( index == 0 && !this->IsClosed() )
    {
    idx1 = 0;
    idx2 = 1;
    }
  else if ( index == this->m_Vertices->size()-1 && !this->IsClosed() )
    {
    idx1 = this->m_Vertices->size()-2;
    idx2 = idx1+1;
    }
  else
    {
    idx1 = index - 1;
    idx2 = index + 1;

    if ( idx1 < 0 )
      {
      idx1 += this->m_Vertices->size();
      }
    if ( idx2 >= this->m_Vertices->size() )
      {
      idx2 -= this->m_Vertices->size();
      }
    }

  slope[0] =
      this->m_Vertices->at(idx2)->Coordinates[0] -
      this->m_Vertices->at(idx1)->Coordinates[0];
  slope[1] =
      this->m_Vertices->at(idx2)->Coordinates[1] -
      this->m_Vertices->at(idx1)->Coordinates[1];
  slope[2] =
      this->m_Vertices->at(idx2)->Coordinates[2] -
      this->m_Vertices->at(idx1)->Coordinates[2];

  vtkMath::Normalize( slope );
  return true;
}
