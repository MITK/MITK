/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourElement_h
#define mitkContourElement_h

#include "mitkCommon.h"
#include <MitkContourModelExports.h>
#include <mitkNumericTypes.h>

#include <deque>

namespace mitk
{
  /** \brief Represents a contour in 3D space.
  A ContourElement is consisting of linked vertices implicitely defining the contour.
  They are stored in a double ended queue making it possible to add vertices at front and
  end of the contour and to iterate in both directions.
  To mark a vertex as a special one it can be set as a control point.

  \note This class assumes that it manages its vertices. So if a vertex instance is added to this
  class the ownership of the vertex is transfered to the ContourElement instance.
  The ContourElement instance takes care of deleting vertex instances if needed.
  It is highly not recommend to use this class directly as it is designed as a internal class of
  ContourModel. Therefore it is adviced to use ContourModel if contour representations are needed in
  MITK.
  */
  class MITKCONTOURMODEL_EXPORT ContourElement : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(ContourElement, itk::LightObject);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Represents a single vertex of a contour.
    */
    struct MITKCONTOURMODEL_EXPORT ContourModelVertex
    {
      ContourModelVertex(const mitk::Point3D& point, bool active = false) : IsControlPoint(active), Coordinates(point) {};
      ContourModelVertex(const ContourModelVertex& other)
        : IsControlPoint(other.IsControlPoint), Coordinates(other.Coordinates)
      {
      };

      /** \brief Treat point special. */
      bool IsControlPoint;

      /** \brief Coordinates in 3D space. */
      mitk::Point3D Coordinates;

      bool operator ==(const ContourModelVertex& other) const;
    };

    using VertexType = ContourModelVertex;
    using VertexListType = std::deque<VertexType*>;
    using VertexIterator = VertexListType::iterator;
    using ConstVertexIterator = VertexListType::const_iterator;
    using VertexSizeType = VertexListType::size_type;

    /**Indicates an invalid index.
     * It is always the maximum of the unsigned int type.*/
    static const VertexSizeType NPOS = -1;

    /** \brief Return a const iterator a the front.
    */
    ConstVertexIterator ConstIteratorBegin() const;
    /** \brief Return a const iterator a the end.
    */
    ConstVertexIterator ConstIteratorEnd() const;
    /** \brief Return an iterator a the front.
    */
    VertexIterator IteratorBegin();
    /** \brief Return an iterator a the end.
    */
    VertexIterator IteratorEnd();

    /** \brief Return a const iterator a the front.
    * For easier support of stl functionality.
    */
    ConstVertexIterator begin() const;
    /** \brief Return a const iterator a the end.
    * For easier support of stl functionality.
    */
    ConstVertexIterator end() const;
    /** \brief Return an iterator a the front.
    * For easier support of stl functionality.
    */
    VertexIterator begin();
    /** \brief Return an iterator a the end.
    * For easier support of stl functionality.
    */
    VertexIterator end();

    /** \brief Returns the number of contained vertices.
    */
    VertexSizeType GetSize() const;

    /** \brief Add a vertex at the end of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.
    */
    void AddVertex(const mitk::Point3D &point, bool isControlPoint);

    /** \brief Add a vertex at the front of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a control point.
    */
    void AddVertexAtFront(const mitk::Point3D &point, bool isControlPoint);

    /** \brief Add a vertex at a given index of the contour
    \param point - coordinates in 3D space.
    \param isControlPoint - is the vertex a special control point.
    \param index - the index to be inserted at.
    */
    void InsertVertexAtIndex(const mitk::Point3D &point, bool isControlPoint, VertexSizeType index);

    /** \brief Set coordinates a given index.
    \param pointId Index of vertex.
    \param point Coordinates.
    */
    void SetVertexAt(VertexSizeType pointId, const mitk::Point3D &point);

    /** \brief Set vertex a given index (by copying the values).
    \param pointId Index of vertex.
    \param vertex Vertex.
    \pre Passed vertex is a valid instance
    */
    void SetVertexAt(VertexSizeType pointId, const VertexType* vertex);

    /** \brief Returns the vertex a given index
    \param index
    \pre index must be valid.
    */
    VertexType* GetVertexAt(VertexSizeType index);
    const VertexType* GetVertexAt(VertexSizeType index) const;

    /** \brief Returns the approximate nearest vertex a given position in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    VertexType *GetVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the next vertex to the approximate nearest vertex of a given position in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    VertexType *GetNextControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the previous vertex to the approximate nearest vertex of a given position in 3D space
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    VertexType *GetPreviousControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the approximate nearest control vertex a given posoition in 3D space, if the clicked position is within a specific range.
      \param point - query position in 3D space.
      \param eps - the error bound for search algorithm.
      */
    VertexType *GetControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Returns the index of the given vertex within the contour.
    \param vertex - the vertex to be searched.
    \return index of vertex. Returns ContourElement::NPOS if not found.
    */
    VertexSizeType GetIndex(const VertexType *vertex) const;

    /** \brief Returns the container of the vertices.
     */
    const VertexListType *GetVertexList() const;

    /** \brief Returns whether the contour element is empty.
    */
    bool IsEmpty() const;

    /** \brief Returns if the conour is closed or not.
    */
    bool IsClosed() const;

    /** \brief Returns whether a given point is near a contour, according to eps.
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm.
    */
    bool IsNearContour(const mitk::Point3D &point, float eps) const;

    /** Function that searches for the line segment of the contour that is closest to the passed point
    and close enough (distance between point and line segment <= eps). If such an line segment exist,
    the starting vertex and closing vertex of the found segment are passed back.
    @return True indicates that a line segment was found. False indicates that no segment of the contour
    is close enough to the passed point.
    @remark previousVertex and nextVertex are only valid if return is true.*/
    bool GetLineSegmentForPoint(const mitk::Point3D &point,
                            float eps,
                            mitk::ContourElement::VertexType *previousVertex,
                            mitk::ContourElement::VertexType *nextVertex) const;
    /** Overloaded version that offers additional options when searching for the line segment.
    In contrast to the other version it returns the index of the segment start and end as well as the point
    on the line segment closest to the passed point. Further one can decide if the function should search for
    the first segment that is close enough (see eps) or for the segment that is really the closest (findClosest==true).
    @remark segmentStartIndex, segmentEndIndex and closestContourPoint are only valid if return is true.*/
    bool GetLineSegmentForPoint(const mitk::Point3D& point,
      float eps, VertexSizeType& segmentStartIndex, VertexSizeType& segmentEndIndex, mitk::Point3D& closestContourPoint, bool findClosest = true) const;

    /** \brief Close the contour.
    Connect first with last element.
    */
    void Close();

    /** \brief Open the contour.
    Disconnect first and last element.
    */
    void Open();

    /** \brief Set the contours IsClosed property.
    \param isClosed - true = closed; false = open;
    */
    void SetClosed(bool isClosed);

    /** \brief Concatenate the contuor with a another contour.
    All vertices of the other contour will be cloned and added after last vertex.
    \param other - the other contour
    \param check - set it true to avoid adding of vertices that are already in the source contour
    */
    void Concatenate(const mitk::ContourElement *other, bool check);

    /** \brief Remove the given vertex from the container if exists.
    \param vertex - the vertex to be removed.
    */
    bool RemoveVertex(const VertexType *vertex);

    /** \brief Remove a vertex at given index within the container if exists.
    \param index - the index where the vertex should be removed.
    */
    bool RemoveVertexAt(VertexSizeType index);

    /** \brief Remove the approximate nearest vertex at given position in 3D space if one exists.
    \param point - query point in 3D space.
    \param eps - error bound for search algorithm.
    */
    bool RemoveVertexAt(const mitk::Point3D &point, double eps);

    /** \brief Clear the storage container.
    */
    void Clear();

    /** \brief Returns the approximate nearest vertex a given position in 3D space. With the parameter 'isControlPoint', 
    one can decide if any vertex should be returned, or just control vertices.
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm. It is an open boundary.
    \param isControlPoint
    \param offset - a offset to the vertex, e.g. 1 if the next vertex should be returned or -1 for the previous vertex
    */
    VertexType *BruteForceGetVertexAt(const mitk::Point3D &point, double eps, bool isControlPoint = false, int offset = 0);

    /** \brief Returns the index of the approximate nearest vertex of a given position in 3D space.
    \param point - query position in 3D space.
    \param eps - the error bound for search algorithm. It is an open boundary.
    \param verticesList - the vertex list to search the index in, either only control vertices or all vertices
    */
    int BruteForceGetVertexIndexAt(const mitk::Point3D &point,
                                                      double eps,
                                                      VertexListType verticesList);

    /** Returns a list pointing to all vertices that are indicated to be control
     points.
     \remark It is important to note, that the vertex pointers in the returned
     list directly point to the vertices stored interanlly. So they are still
     owned by the ContourElement instance that returns the list. If one wants
     to take over ownership, one has to clone the vertex instances.
     */
    VertexListType GetControlVertices() const;

    /** \brief Uniformly redistribute control points with a given period (in number of vertices)
    \param vertex - the vertex around which the redistribution is done.
    \param period - number of vertices between control points.
    */
    void RedistributeControlVertices(const VertexType *vertex, int period);

  protected:
    mitkCloneMacro(Self);

    ContourElement() = default;
    ContourElement(const mitk::ContourElement &other);
    ~ContourElement();

    ContourElement& operator = (const ContourElement & other);

    /** Internal helper function to correctly remove the element indicated by the iterator
    from the list. After the call the iterator is invalid.
    Caller of the function must ensure that the iterator is valid!.
    \result Indicates if the element indicated by the iterator was removed. If iterator points to end it returns false.*/
    bool RemoveVertexByIterator(VertexListType::iterator& iter);

    VertexListType m_Vertices; // double ended queue with vertices
    bool m_IsClosed = false;
  };
} // namespace mitk

#endif
