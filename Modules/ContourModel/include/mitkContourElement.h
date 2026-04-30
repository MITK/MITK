/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourElement_h
#define mitkContourElement_h

#include <mitkCommon.h>
#include <MitkContourModelExports.h>
#include <mitkNumericTypes.h>

#include <deque>

namespace mitk
{
  /** \brief Represents a contour in 3D space as a sequence of linked vertices.
   *
   * A ContourElement consists of linked vertices that implicitly define a contour.
   * The vertices are stored in a double-ended queue (std::deque), making it possible
   * to add vertices at the front and end of the contour and to iterate in both directions.
   * Individual vertices can be marked as control points to distinguish them from
   * regular interpolation points.
   *
   * This class owns all its vertices. When a vertex is added, ownership is transferred
   * to this ContourElement instance, which manages deletion.
   *
   * \note This class is designed as an internal component of ContourModel. It is strongly
   * recommended to use ContourModel rather than ContourElement directly when working
   * with contour representations in MITK.
   *
   * \sa ContourModel
   * \sa ContourModelSet
   */
  class MITKCONTOURMODEL_EXPORT ContourElement : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(ContourElement, itk::LightObject);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Represents a single vertex of a contour.
     *
     * Each vertex stores its 3D coordinates and a flag indicating whether it
     * is a control point (i.e., a point with special semantic meaning such as
     * being used for interpolation or rendering).
     */
    struct MITKCONTOURMODEL_EXPORT ContourModelVertex
    {
      /** \brief Construct a vertex at the given position.
       * \param[in] point Coordinates in 3D space.
       * \param[in] active If true, the vertex is marked as a control point. Default is false.
       */
      ContourModelVertex(const mitk::Point3D& point, bool active = false) : IsControlPoint(active), Coordinates(point) {};

      /** \brief Copy constructor.
       * \param[in] other The vertex to copy from.
       */
      ContourModelVertex(const ContourModelVertex& other)
        : IsControlPoint(other.IsControlPoint), Coordinates(other.Coordinates)
      {
      };

      /** \brief Flag indicating whether this vertex is a control point.
       *
       * Control points receive special treatment, e.g., they may be rendered
       * differently or used as anchors for interpolation schemes.
       */
      bool IsControlPoint;

      /** \brief The 3D world coordinates of this vertex. */
      mitk::Point3D Coordinates;

      /** \brief Equality comparison operator.
       * \param[in] other The vertex to compare against.
       * \return True if both coordinates and the control point flag are equal.
       */
      bool operator ==(const ContourModelVertex& other) const;
    };

    /** \brief Alias for the vertex type used in this contour. */
    using VertexType = ContourModelVertex;
    /** \brief Alias for the container type storing vertex pointers. */
    using VertexListType = std::deque<VertexType*>;
    /** \brief Mutable iterator for traversing vertices. */
    using VertexIterator = VertexListType::iterator;
    /** \brief Const iterator for traversing vertices. */
    using ConstVertexIterator = VertexListType::const_iterator;
    /** \brief Size type for indexing into the vertex list. */
    using VertexSizeType = VertexListType::size_type;

    /** \brief Sentinel value indicating an invalid vertex index.
     *
     * Equivalent to the maximum value of the unsigned size type.
     */
    static const VertexSizeType NPOS = -1;

    /** \brief Return a const iterator to the first vertex.
     * \return Const iterator pointing to the beginning of the vertex container.
     */
    ConstVertexIterator ConstIteratorBegin() const;

    /** \brief Return a const iterator past the last vertex.
     * \return Const iterator pointing past the end of the vertex container.
     */
    ConstVertexIterator ConstIteratorEnd() const;

    /** \brief Return a mutable iterator to the first vertex.
     * \return Iterator pointing to the beginning of the vertex container.
     */
    VertexIterator IteratorBegin();

    /** \brief Return a mutable iterator past the last vertex.
     * \return Iterator pointing past the end of the vertex container.
     */
    VertexIterator IteratorEnd();

    /** \brief Return a const iterator to the first vertex (STL-compatible).
     * \return Const iterator pointing to the beginning of the vertex container.
     */
    ConstVertexIterator begin() const;

    /** \brief Return a const iterator past the last vertex (STL-compatible).
     * \return Const iterator pointing past the end of the vertex container.
     */
    ConstVertexIterator end() const;

    /** \brief Return a mutable iterator to the first vertex (STL-compatible).
     * \return Iterator pointing to the beginning of the vertex container.
     */
    VertexIterator begin();

    /** \brief Return a mutable iterator past the last vertex (STL-compatible).
     * \return Iterator pointing past the end of the vertex container.
     */
    VertexIterator end();

    /** \brief Return the number of vertices in this contour element.
     * \return The number of contained vertices.
     */
    VertexSizeType GetSize() const;

    /** \brief Add a vertex at the end of the contour.
     * \param[in] point Coordinates of the new vertex in 3D space.
     * \param[in] isControlPoint If true, the vertex is marked as a control point.
     */
    void AddVertex(const mitk::Point3D &point, bool isControlPoint);

    /** \brief Add a vertex at the front of the contour.
     * \param[in] point Coordinates of the new vertex in 3D space.
     * \param[in] isControlPoint If true, the vertex is marked as a control point.
     */
    void AddVertexAtFront(const mitk::Point3D &point, bool isControlPoint);

    /** \brief Insert a vertex at the given index in the contour.
     *
     * If the index equals or exceeds the current size, the vertex is appended.
     *
     * \param[in] point Coordinates of the new vertex in 3D space.
     * \param[in] isControlPoint If true, the vertex is marked as a control point.
     * \param[in] index Position at which the vertex is inserted.
     */
    void InsertVertexAtIndex(const mitk::Point3D &point, bool isControlPoint, VertexSizeType index);

    /** \brief Set the coordinates of an existing vertex at the given index.
     * \param[in] pointId Index of the vertex to modify.
     * \param[in] point New 3D coordinates.
     */
    void SetVertexAt(VertexSizeType pointId, const mitk::Point3D &point);

    /** \brief Set the vertex data at a given index by copying from the provided vertex.
     * \param[in] pointId Index of the vertex to modify.
     * \param[in] vertex Source vertex whose coordinates and control point flag are copied.
     * \pre \p vertex must be a valid (non-null) pointer.
     * \throw mitk::Exception if vertex is nullptr.
     */
    void SetVertexAt(VertexSizeType pointId, const VertexType* vertex);

    /** \brief Return the vertex at the given index.
     * \param[in] index Index of the vertex to retrieve.
     * \return Pointer to the vertex at the specified index.
     * \pre \p index must be a valid index (< GetSize()).
     * \throw std::out_of_range if the index is out of bounds.
     */
    VertexType* GetVertexAt(VertexSizeType index);

    /** \brief Return the vertex at the given index (const version).
     * \param[in] index Index of the vertex to retrieve.
     * \return Const pointer to the vertex at the specified index.
     * \pre \p index must be a valid index (< GetSize()).
     * \throw std::out_of_range if the index is out of bounds.
     */
    const VertexType* GetVertexAt(VertexSizeType index) const;

    /** \brief Find the nearest vertex to a given 3D position within a search radius.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for a vertex to be considered a match.
     * \return Pointer to the nearest vertex, or nullptr if none is within eps.
     */
    VertexType *GetVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Return the next control vertex after the nearest vertex to a given 3D position.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for the initial nearest vertex search.
     * \return Pointer to the next control vertex, or nullptr if none is found.
     */
    VertexType *GetNextControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Return the previous control vertex before the nearest vertex to a given 3D position.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for the initial nearest vertex search.
     * \return Pointer to the previous control vertex, or nullptr if none is found.
     */
    VertexType *GetPreviousControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Find the nearest control vertex to a given 3D position within a search radius.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for a control vertex to be considered a match.
     * \return Pointer to the nearest control vertex, or nullptr if none is within eps.
     */
    VertexType *GetControlVertexAt(const mitk::Point3D &point, float eps);

    /** \brief Return the index of a given vertex within the contour.
     * \param[in] vertex Pointer to the vertex to search for (by pointer identity).
     * \return Index of the vertex, or ContourElement::NPOS if not found.
     */
    VertexSizeType GetIndex(const VertexType *vertex) const;

    /** \brief Return a pointer to the internal vertex container.
     * \return Const pointer to the internal deque of vertex pointers.
     */
    const VertexListType *GetVertexList() const;

    /** \brief Check whether the contour element contains no vertices.
     * \return True if the contour has no vertices, false otherwise.
     */
    bool IsEmpty() const;

    /** \brief Check whether the contour is closed.
     * \return True if the contour is closed (first and last vertex are logically connected).
     */
    bool IsClosed() const;

    /** \brief Check whether a given point lies near the contour within a specified tolerance.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum squared distance for the point to be considered near the contour.
     * \return True if the point is within eps squared distance of any contour line segment.
     */
    bool IsNearContour(const mitk::Point3D &point, float eps) const;

    /** \brief Find the contour line segment closest to a given point.
     *
     * Searches for the line segment whose distance to \p point is at most \p eps.
     * If found, the start and end vertices of the segment are written to
     * \p previousVertex and \p nextVertex.
     *
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum squared distance for a segment to be considered close enough.
     * \param[out] previousVertex Receives the start vertex of the found segment.
     * \param[out] nextVertex Receives the end vertex of the found segment.
     * \return True if a matching segment was found, false otherwise.
     * \note \p previousVertex and \p nextVertex are only valid when the return value is true.
     */
    bool GetLineSegmentForPoint(const mitk::Point3D &point,
                            float eps,
                            mitk::ContourElement::VertexType *previousVertex,
                            mitk::ContourElement::VertexType *nextVertex) const;

    /** \brief Find the contour line segment closest to a given point (extended version).
     *
     * In addition to the basic version, this overload returns the indices of the segment
     * endpoints, the closest point on the segment, and supports choosing between
     * the first close-enough segment or the globally closest segment.
     *
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum squared distance for a segment to be considered close enough.
     * \param[out] segmentStartIndex Index of the segment's start vertex.
     * \param[out] segmentEndIndex Index of the segment's end vertex.
     * \param[out] closestContourPoint The point on the segment closest to \p point.
     * \param[in] findClosest If true, finds the globally closest segment; if false, returns
     *            the first segment within eps. Default is true.
     * \return True if a matching segment was found, false otherwise.
     * \note Output parameters are only valid when the return value is true.
     */
    bool GetLineSegmentForPoint(const mitk::Point3D& point,
      float eps, VertexSizeType& segmentStartIndex, VertexSizeType& segmentEndIndex, mitk::Point3D& closestContourPoint, bool findClosest = true) const;

    /** \brief Close the contour by logically connecting the last vertex to the first.
     * \post IsClosed() returns true.
     */
    void Close();

    /** \brief Open the contour by removing the logical connection between last and first vertex.
     * \post IsClosed() returns false.
     */
    void Open();

    /** \brief Set the closed state of the contour.
     * \param[in] isClosed If true, the contour is closed; if false, it is opened.
     */
    void SetClosed(bool isClosed);

    /** \brief Append all vertices of another contour to the end of this contour.
     *
     * All vertices from \p other are cloned and appended after the last vertex
     * of this contour.
     *
     * \param[in] other The contour whose vertices are to be appended.
     * \param[in] check If true, vertices whose coordinates already exist in this
     *            contour are skipped to avoid duplicates.
     */
    void Concatenate(const mitk::ContourElement *other, bool check);

    /** \brief Remove a specific vertex from the contour by pointer identity.
     * \param[in] vertex Pointer to the vertex to remove.
     * \return True if the vertex was found and removed, false otherwise.
     */
    bool RemoveVertex(const VertexType *vertex);

    /** \brief Remove the vertex at a given index.
     * \param[in] index Index of the vertex to remove.
     * \return True if the index was valid and the vertex was removed, false otherwise.
     */
    bool RemoveVertexAt(VertexSizeType index);

    /** \brief Remove the nearest vertex to a given 3D position within a search radius.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for a vertex to be considered a match.
     * \return True if a vertex was found and removed, false otherwise.
     */
    bool RemoveVertexAt(const mitk::Point3D &point, double eps);

    /** \brief Remove all vertices from the contour and free their memory.
     * \post GetSize() returns 0.
     */
    void Clear();

    /** \brief Find the nearest vertex to a given 3D position using brute-force search.
     *
     * Optionally filters for control points only and supports returning a vertex
     * at an offset from the nearest match (e.g., +1 for the next, -1 for the previous).
     * Wrapping is applied if the offset goes past either end of the list.
     *
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance (open boundary). Must be >= 0.
     * \param[in] isControlPoint If true, only control vertices are considered. Default is false.
     * \param[in] offset Index offset from the nearest vertex. Default is 0 (return the nearest).
     * \return Pointer to the matching vertex, or nullptr if no vertex is within eps.
     */
    VertexType *BruteForceGetVertexAt(const mitk::Point3D &point, double eps, bool isControlPoint = false, int offset = 0);

    /** \brief Find the index of the nearest vertex to a given 3D position using brute-force search.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance (open boundary). Must be >= 0.
     * \param[in] verticesList The vertex list to search (may be a subset, e.g., only control vertices).
     * \return Index of the nearest vertex within eps, or -1 if none is found.
     * \throw mitk::Exception if eps is negative.
     */
    int BruteForceGetVertexIndexAt(const mitk::Point3D &point,
                                                      double eps,
                                                      VertexListType verticesList);

    /** \brief Return a list of all vertices that are marked as control points.
     *
     * The returned list contains pointers to the internally stored vertices.
     * The ContourElement retains ownership of the vertices; callers must clone
     * any vertices they wish to manage independently.
     *
     * \return A VertexListType containing pointers to all control point vertices.
     */
    VertexListType GetControlVertices() const;

    /** \brief Uniformly redistribute control point markers among the vertices.
     *
     * Starting from \p vertex (or the first vertex if nullptr), marks every
     * \p period-th vertex as a control point and clears the flag on all others.
     *
     * \param[in] vertex The vertex around which redistribution is centered. May be nullptr.
     * \param[in] period Number of vertices between successive control points.
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
