/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <algorithm>
#include <mitkContourElement.h>
#include <vtkMath.h>

bool mitk::ContourElement::ContourModelVertex::operator ==(const ContourModelVertex& other) const
{
  return this->Coordinates == other.Coordinates && this->IsControlPoint == other.IsControlPoint;
}

mitk::ContourElement::ConstVertexIterator mitk::ContourElement::ConstIteratorBegin() const
{
  return this->begin();
}

mitk::ContourElement::ConstVertexIterator mitk::ContourElement::ConstIteratorEnd() const
{
  return this->end();
}

mitk::ContourElement::VertexIterator mitk::ContourElement::IteratorBegin()
{
  return this->begin();
}

mitk::ContourElement::VertexIterator mitk::ContourElement::IteratorEnd()
{
  return this->end();
}

mitk::ContourElement::ConstVertexIterator mitk::ContourElement::begin() const
{
  return this->m_Vertices.begin();
}

mitk::ContourElement::ConstVertexIterator mitk::ContourElement::end() const
{
  return this->m_Vertices.end();
}

mitk::ContourElement::VertexIterator mitk::ContourElement::begin()
{
  return this->m_Vertices.begin();
}

mitk::ContourElement::VertexIterator mitk::ContourElement::end()
{
  return this->m_Vertices.end();
}

mitk::ContourElement::ContourElement(const mitk::ContourElement &other)
  : itk::LightObject(), m_IsClosed(other.m_IsClosed)
{
  for (const auto& v : other.m_Vertices)
  {
    m_Vertices.push_back(new ContourModelVertex(*v));
  }
}

mitk::ContourElement& mitk::ContourElement::operator = (const ContourElement& other)
{
  if (this != &other)
  {
    this->Clear();
    for (const auto& v : other.m_Vertices)
    {
      m_Vertices.push_back(new ContourModelVertex(*v));
    }
  }

  this->m_IsClosed = other.m_IsClosed;
  return *this;
}

mitk::ContourElement::~ContourElement()
{
  this->Clear();
}

mitk::ContourElement::VertexSizeType mitk::ContourElement::GetSize() const
{
  return this->m_Vertices.size();
}

void mitk::ContourElement::AddVertex(const mitk::Point3D &vertex, bool isControlPoint)
{
  this->m_Vertices.push_back(new VertexType(vertex, isControlPoint));
}

void mitk::ContourElement::AddVertexAtFront(const mitk::Point3D &vertex, bool isControlPoint)
{
  this->m_Vertices.push_front(new VertexType(vertex, isControlPoint));
}

void mitk::ContourElement::InsertVertexAtIndex(const mitk::Point3D &vertex, bool isControlPoint, VertexSizeType index)
{
  if (index >= 0 && this->GetSize() > index)
  {
    auto _where = this->m_Vertices.begin();
    _where += index;
    this->m_Vertices.insert(_where, new VertexType(vertex, isControlPoint));
  }
}

void mitk::ContourElement::SetVertexAt(VertexSizeType pointId, const Point3D &point)
{
  if (pointId >= 0 && this->GetSize() > pointId)
  {
    this->m_Vertices[pointId]->Coordinates = point;
  }
}

void mitk::ContourElement::SetVertexAt(VertexSizeType pointId, const VertexType *vertex)
{
  if (nullptr == vertex)
  {
    mitkThrow() << "Cannot set vertex. Passed vertex instance is invalid. Index to set: " << pointId;
  }

  if (pointId >= 0 && this->GetSize() > pointId)
  {
    this->m_Vertices[pointId]->Coordinates = vertex->Coordinates;
    this->m_Vertices[pointId]->IsControlPoint = vertex->IsControlPoint;
  }
}

mitk::ContourElement::VertexType *mitk::ContourElement::GetVertexAt(VertexSizeType index)
{
  return this->m_Vertices.at(index);
}

const mitk::ContourElement::VertexType* mitk::ContourElement::GetVertexAt(VertexSizeType index) const
{
  return this->m_Vertices.at(index);
}

bool mitk::ContourElement::IsEmpty() const
{
  return this->m_Vertices.empty();
}

mitk::ContourElement::VertexType *mitk::ContourElement::GetVertexAt(const mitk::Point3D &point, float eps)
{
  /* current version iterates over the whole deque - should some kind of an octree with spatial query*/

  if (eps > 0)
  {
    // currently no method with better performance is available
    return BruteForceGetVertexAt(point, eps);
  } // if eps < 0
  return nullptr;
}

mitk::ContourElement::VertexType *mitk::ContourElement::BruteForceGetVertexAt(const mitk::Point3D &point, double eps)
{
  if (eps > 0)
  {
    std::deque<std::pair<double, VertexType *>> nearestlist;

    ConstVertexIterator it = this->m_Vertices.begin();

    ConstVertexIterator end = this->m_Vertices.end();

    while (it != end)
    {
      mitk::Point3D currentPoint = (*it)->Coordinates;

      double distance = currentPoint.EuclideanDistanceTo(point);
      if (distance < eps)
      {
        // if list is emtpy, add point to list
        if (nearestlist.size() < 1)
        {
          nearestlist.push_front(std::pair<double, VertexType *>((*it)->Coordinates.EuclideanDistanceTo(point), (*it)));
        }
        // found an approximate point - check if current is closer then first in nearestlist
        else if (distance < nearestlist.front().first)
        {
          // found even closer vertex
          nearestlist.push_front(std::pair<double, VertexType *>((*it)->Coordinates.EuclideanDistanceTo(point), (*it)));
        }
      } // if distance > eps

      it++;
    } // while
    if (nearestlist.size() > 0)
    {
      /*++++++++++++++++++++ return the nearest active point if one was found++++++++++++++++++*/
      auto it = nearestlist.begin();
      auto end = nearestlist.end();
      while (it != end)
      {
        if ((*it).second->IsControlPoint)
        {
          return (*it).second;
        }
        it++;
      }
      /*---------------------------------------------------------------------------------------*/

      // return closest point
      return nearestlist.front().second;
    }
  }
  return nullptr;
}

const mitk::ContourElement::VertexListType *mitk::ContourElement::GetVertexList() const
{
  return &(this->m_Vertices);
}

bool mitk::ContourElement::IsClosed() const
{
  return this->m_IsClosed;
}

bool mitk::ContourElement::IsNearContour(const mitk::Point3D &point, float eps) const
{
  ConstVertexIterator it1 = this->m_Vertices.begin();
  ConstVertexIterator it2 = this->m_Vertices.begin();
  it2++; // it2 runs one position ahead

  ConstVertexIterator end = this->m_Vertices.end();

  int counter = 0;

  for (; it1 != end; it1++, it2++, counter++)
  {
    if (it2 == end)
      it2 = this->m_Vertices.begin();

    mitk::Point3D v1 = (*it1)->Coordinates;
    mitk::Point3D v2 = (*it2)->Coordinates;

    const float l2 = v1.SquaredEuclideanDistanceTo(v2);

    mitk::Vector3D p_v1 = point - v1;
    mitk::Vector3D v2_v1 = v2 - v1;

    double tc = (p_v1 * v2_v1) / l2;

    // take into account we have line segments and not (infinite) lines
    if (tc < 0.0)
      tc = 0.0;
    if (tc > 1.0)
      tc = 1.0;

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

void mitk::ContourElement::SetClosed(bool isClosed)
{
  isClosed ? this->Close() : this->Open();
}

mitk::ContourElement::VertexListType mitk::ContourElement::GetControlVertices() const
{
  VertexListType controlVertices;

  std::copy_if(this->m_Vertices.begin(), this->m_Vertices.end(), std::back_inserter(controlVertices), [](const VertexType* v) {return v->IsControlPoint; });

  return controlVertices;
}

void mitk::ContourElement::Concatenate(const mitk::ContourElement *other, bool check)
{
  if (other->GetSize() > 0)
  {
    for (const auto& sourceVertex : other->m_Vertices)
    {
      if (check)
      {
        auto finding = std::find_if(this->m_Vertices.begin(), this->m_Vertices.end(), [sourceVertex](const VertexType* v) {return sourceVertex->Coordinates == v->Coordinates; });

        if (finding == this->m_Vertices.end())
        {
          this->m_Vertices.push_back(new ContourModelVertex(*sourceVertex));
        }
      }
      else
      {
        this->m_Vertices.push_back(new ContourModelVertex(*sourceVertex));
      }
    }
  }
}

mitk::ContourElement::VertexSizeType mitk::ContourElement::GetIndex(const VertexType* vertex) const
{
  VertexSizeType result = NPOS;

  auto finding = std::find(this->m_Vertices.begin(), this->m_Vertices.end(), vertex);

  if (finding != this->m_Vertices.end())
  {
    result = finding - this->m_Vertices.begin();
  }

  return result;
}

bool mitk::ContourElement::RemoveVertex(const VertexType *vertex)
{
  auto finding = std::find(this->m_Vertices.begin(), this->m_Vertices.end(), vertex);

  return RemoveVertexByIterator(finding);
}

bool mitk::ContourElement::RemoveVertexAt(VertexSizeType index)
{
  if (index >= 0 && index < this->m_Vertices.size())
  {
    auto delIter = this->m_Vertices.begin() + index;
    return RemoveVertexByIterator(delIter);
  }

  return false;
}

bool mitk::ContourElement::RemoveVertexAt(const mitk::Point3D &point, double eps)
{
  if (eps > 0)
  {
    auto finding = std::find_if(this->m_Vertices.begin(), this->m_Vertices.end(), [point, eps](const VertexType* v) {return v->Coordinates.EuclideanDistanceTo(point) < eps; });

    return RemoveVertexByIterator(finding);
  }
  return false;
}

bool mitk::ContourElement::RemoveVertexByIterator(VertexListType::iterator& iter)
{
  if (iter != this->m_Vertices.end())
  {
    delete* iter;
    this->m_Vertices.erase(iter);
    return true;
  }

  return false;
}

void mitk::ContourElement::Clear()
{
  for (auto vertex : m_Vertices)
  {
    delete vertex;
  }
  this->m_Vertices.clear();
}

//----------------------------------------------------------------------
void mitk::ContourElement::RedistributeControlVertices(const VertexType *selected, int period)
{
  int counter = 0;
  auto _where = this->m_Vertices.begin();

  if (selected != nullptr)
  {
    auto finding = std::find(this->m_Vertices.begin(), this->m_Vertices.end(), selected);

    if (finding != this->m_Vertices.end())
    {
      _where = finding;
    }
  }

  auto _iter = _where;
  while (_iter != this->m_Vertices.end())
  {
    div_t divresult;
    divresult = div(counter, period);
    (*_iter)->IsControlPoint = (divresult.rem == 0);
    counter++;
    _iter++;
  }

  _iter = _where;
  counter = 0;
  while (_iter != this->m_Vertices.begin())
  {
    div_t divresult;
    divresult = div(counter, period);
    (*_iter)->IsControlPoint = (divresult.rem == 0);
    counter++;
    _iter--;
  }
}
