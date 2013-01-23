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
#ifndef _MITK_ContourElement_H_
#define _MITK_ContourElement_H_

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


/*+++++++++++++++++++++ Data container representing vertices +++++++++++++++++*/


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
/*+++++++++++++++++++++ END Data container representing vertices ++++++++++++++*/


/*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef ContourModelVertex VertexType;
    typedef std::deque<VertexType*> VertexListType;
    typedef VertexListType::iterator VertexIterator;
    typedef VertexListType::const_iterator ConstVertexIterator;
/*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/



/*++++++++++++++++  inline methods  +++++++++++++++++++++++*/

    /** \brief Return a const iterator a the front. */
    virtual ConstVertexIterator ConstIteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    /** \brief Return a const iterator a the end. */
    virtual ConstVertexIterator ConstIteratorEnd()
    {
      return this->m_Vertices->end();
    }

    /** \brief Return an iterator a the front. */
    virtual VertexIterator IteratorBegin()
    {
      return this->m_Vertices->begin();
    }

    /** \brief Return an iterator a the end. */
    virtual VertexIterator IteratorEnd()
    {
      return this->m_Vertices->end();
    }

    /** \brief Returns the number of contained vertices. */
    virtual int GetSize()
    {
      return this->m_Vertices->size();
    }
/*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/



    /** \brief Add a vertex at the end of the contour
    \param vertex - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.*/
    virtual void AddVertex(mitk::Point3D &vertex, bool isControlPoint);

    /** \brief Add a vertex at the end of the contour
    \param vertex - a ContourModelVertex.
    */
    virtual void AddVertex(VertexType &vertex);

    /** \brief Add a vertex at the front of the contour
    \param vertex - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.*/
    virtual void AddVertexAtFront(mitk::Point3D &vertex, bool isControlPoint);

    /** \brief Add a vertex at the front of the contour
    \param vertex - a ContourModelVertex.
    */
    virtual void AddVertexAtFront(VertexType &vertex);

    /** \brief Add a vertex at a given index of the contour
    \param vertex - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.
    \param index - the index to be inserted at.
    */
    virtual void InsertVertexAtIndex(mitk::Point3D &vertex, bool isControlPoint, int index);

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

    /** \brief Returns if the conour is closed or not.
    */
    virtual bool IsClosed();

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


  protected:

    ContourElement();
    ContourElement(const mitk::ContourElement &other);
    virtual ~ContourElement();

    VertexListType* m_Vertices; //double ended queue with vertices
    bool m_IsClosed;

  };
}

#endif
