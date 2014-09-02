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
#ifndef _mitkContourElement_H_
#define _mitkContourElement_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include <mitkNumericTypes.h>

//#include <ANN/ANN.h>


#include <deque>

namespace mitk
{

  /** \brief Represents a contour in 3D space.
  A ContourElement is consisting of linked vertices implicitely defining the contour.
  They are stored in a double ended queue making it possible to add vertices at front and
  end of the contour and to iterate in both directions.
  To mark a vertex as a special one it can be set as a control point.

  \Note It is highly not recommend to use this class directly as no secure mechanism is used here.
  Use mitk::ContourModel instead providing some additional features.
  */
  class MitkContourModel_EXPORT ContourElement : public itk::LightObject
  {

  public:

    mitkClassMacro(ContourElement, itk::LightObject);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

// Data container representing vertices

    /** \brief Represents a single vertex of contour.
    */
    struct ContourModelVertex
    {
      ContourModelVertex(mitk::Point3D &point, bool active=false)
        : IsControlPoint(active), Coordinates(point)
      {

      }

      ContourModelVertex( const ContourModelVertex &other)
        : IsControlPoint(other.IsControlPoint), Coordinates(other.Coordinates)
      {
      }

      /** \brief Treat point special. */
      bool IsControlPoint;

      /** \brief Coordinates in 3D space. */
      mitk::Point3D Coordinates;
    };
// END Data container representing vertices

    typedef ContourModelVertex VertexType;
    typedef std::deque<VertexType*> VertexListType;
    typedef VertexListType::iterator VertexIterator;
    typedef VertexListType::const_iterator ConstVertexIterator;

    //  start of inline methods

    /** \brief Return a const iterator a the front.
    */
    virtual ConstVertexIterator ConstIteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    /** \brief Return a const iterator a the end.
    */
    virtual ConstVertexIterator ConstIteratorEnd()
    {
      return this->m_Vertices->end();
    }

    /** \brief Return an iterator a the front.
    */
    virtual VertexIterator IteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    /** \brief Return an iterator a the end.
    */
    virtual VertexIterator IteratorEnd()
    {
      return this->m_Vertices->end();
    }

    /** \brief Returns the number of contained vertices.
    */
    virtual int GetSize()
    {
      return this->m_Vertices->size();
    }
    //   end of inline methods

    /** \brief Add a vertex at the end of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.
    */
    virtual void AddVertex(mitk::Point3D &point, bool isControlPoint);

    /** \brief Add a vertex at the end of the contour
    \param vertex - a contour element vertex.
    */
    virtual void AddVertex(VertexType &vertex);

    /** \brief Add a vertex at the front of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a control point.
    */
    virtual void AddVertexAtFront(mitk::Point3D &point, bool isControlPoint);

    /** \brief Add a vertex at the front of the contour
    \param vertex - a contour element vertex.
    */
    virtual void AddVertexAtFront(VertexType &vertex);

    /** \brief Add a vertex at a given index of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.
    \param index - the index to be inserted at.
    */
    virtual void InsertVertexAtIndex(mitk::Point3D &point, bool isControlPoint, int index);

    /** \brief Set coordinates a given index.
    \param pointId Index of vertex.
    \param point Coordinates.
    */
    virtual void SetVertexAt(int pointId, const mitk::Point3D &point);

    /** \brief Set vertex a given index.
    \param pointId Index of vertex.
    \param vertex Vertex.
    */
    virtual void SetVertexAt(int pointId, const VertexType* vertex);

    /** \brief Returns the vertex a given index
    \param index
    */
    virtual VertexType* GetVertexAt(int index);

    /** \brief Returns the approximate nearest vertex a given posoition in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    virtual VertexType* GetVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the index of the given vertex within the contour.
    \param vertex - the vertex to be searched.
    \return index of vertex. -1 if not found.
    */
    virtual int GetIndex(const VertexType* vertex);

    /** \brief Returns the container of the vertices.
    */
    VertexListType* GetVertexList();

    /** \brief Returns whether the contour element is empty.
    */
    bool IsEmpty();

    /** \brief Returns if the conour is closed or not.
    */
    virtual bool IsClosed();

    /** \brief Returns whether a given point is near a contour, according to eps.
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    virtual bool IsNearContour(const mitk::Point3D &point, float eps);

    /** \brief Close the contour.
    Connect first with last element.
    */
    virtual void Close();

    /** \brief Open the contour.
    Disconnect first and last element.
    */
    virtual void Open();

    /** \brief Set the contours IsClosed property.
    \param isClosed - true = closed; false = open;
    */
    virtual void SetClosed(bool isClosed);

    /** \brief Concatenate the contuor with a another contour.
    All vertices of the other contour will be added after last vertex.
    \param other - the other contour
    \param check - set it true to avoid intersections
    */
    void Concatenate(mitk::ContourElement* other, bool check);

    /** \brief Remove the given vertex from the container if exists.
    \param vertex - the vertex to be removed.
    */
    virtual bool RemoveVertex(const VertexType* vertex);

    /** \brief Remove a vertex at given index within the container if exists.
    \param index - the index where the vertex should be removed.
    */
    virtual bool RemoveVertexAt(int index);

    /** \brief Remove the approximate nearest vertex at given position in 3D space if one exists.
    \param point - query point in 3D space.
    \param eps - error bound for search algorithm.
    */
    virtual bool RemoveVertexAt(mitk::Point3D &point, float eps);

    /** \brief Clear the storage container.
    */
    virtual void Clear();

    /** \brief Returns the approximate nearest vertex a given posoition in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    VertexType* BruteForceGetVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the approximate nearest vertex a given posoition in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    //VertexType* OptimizedGetVertexAt(const mitk::Point3D &point, float eps);

    VertexListType* GetControlVertices();

    /** \brief Uniformly redistribute control points with a given period (in number of vertices)
    \param vertex - the vertex around which the redistribution is done.
    \param period - number of vertices between control points.
    */
    void RedistributeControlVertices(const VertexType* vertex, int period);

  protected:
    mitkCloneMacro(Self);

    ContourElement();
    ContourElement(const mitk::ContourElement &other);
    virtual ~ContourElement();

    VertexListType* m_Vertices; //double ended queue with vertices
    bool m_IsClosed;

  };
} // namespace mitk

#endif // _mitkContourElement_H_
