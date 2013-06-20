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
#include "SegmentationExports.h"
#include <mitkVector.h>

#include <ANN/ANN.h>


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
  class Segmentation_EXPORT ContourElement : public itk::LightObject
  {

  public:

    mitkClassMacro(ContourElement, itk::LightObject);

    itkNewMacro(Self);

    mitkCloneMacro(Self);


// Data container representing vertices

    /** \brief Represents a single vertex of contour.
    */
    struct ContourModelVertex
    {
      ContourModelVertex(mitk::Point3D &point, bool active=false)
        : Coordinates(point), IsControlPoint(active)
      {

      };

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

    /** \brief Returns the vertex a given index
    \param index
    */
    virtual VertexType* GetVertexAt(int index);

    /** \brief Returns the approximate nearest vertex a given posoition in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    virtual VertexType* GetVertexAt(const mitk::Point3D &point, float eps);

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
    virtual void SetIsClosed(bool isClosed);

    /** \brief Concatenate the contuor with a another contour.
    All vertices of the other contour will be add after last vertex.
    */
    void Concatenate(mitk::ContourElement* other);

    /** \brief Removes intersected vertices between two contour elements
    \param other - a given contour element.
    */
    void RemoveIntersections(mitk::ContourElement* other);

    /** \brief Remove the given vertex from the container if exists.
    \param vertex - the vertex to be removed.
    */
    virtual bool RemoveVertex(VertexType* vertex);

    /** \brief Remove a vertex at given index within the container if exists.
    \param index - the index to be removed at.
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
    VertexType* OptimizedGetVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Generate and interpolated version of the contour element based on the
    active interpolation method
    */
    void Interpolate();

    VertexListType* GetControlVertices();

    /** \brief Uniformly redistribute control points with a given period (in number of vertices)
    \param period - number of vertices between control points.
    */
    void RedistributeControlVertices(const VertexType* selected, int period);

  protected:

    ContourElement();
    ContourElement(const mitk::ContourElement &other);
    virtual ~ContourElement();

    /** \brief Finds the 4-th order bezier curve between given indexes.
    Adapted from vtkBezierContourLineInterpolator
    \param idx1 - first index
    \param idx2 - second index
    */
    void DoBezierInterpolation( int idx1, int idx2, VertexListType* vertices );

    /** \brief Returns a pair with two iterators pointing to the vertices where
    the contour element intersects (i.e. has the same coordinates) with another given contour element
    \param other - a given contour element.
    */
    std::pair<mitk::ContourElement::VertexIterator, mitk::ContourElement::VertexIterator>
        FindFirstIntersection(mitk::ContourElement* other);

    /** \brief Calculates the slope at a given index
    \param index - the index to get the slope at.
    \param slope - splope at given index.
    */
    bool GetNthNodeSlope( int index, double slope[3]);

   void ComputeMidpoint( double p1[3], double p2[3], double mid[3] )
    {
      mid[0] = (p1[0] + p2[0])/2;
      mid[1] = (p1[1] + p2[1])/2;
      mid[2] = (p1[2] + p2[2])/2;
    }

    VertexListType* m_Vertices; //double ended queue with vertices
    bool m_IsClosed;

  };
} // namespace mitk

#endif // _mitkContourElement_H_
