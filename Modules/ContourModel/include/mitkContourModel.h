/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourModel_h
#define mitkContourModel_h

#include <mitkBaseData.h>
#include <mitkCommon.h>
#include <MitkContourModelExports.h>

#include <mitkContourElement.h>

namespace mitk
{
  /**
   * \brief A time-resolved contour representation consisting of linked vertices in 3D space.
   *
   * ContourModel stores a sequence of vertices (mitk::ContourElement) for each time step.
   * Line segments between vertices are implicitly defined. By default, consecutive control
   * points are connected by straight lines, but B-spline interpolation is also available.
   *
   * Vertices can be added at the front or end and accessed by index, 3D position, or
   * iterator. The contour may be open (a polyline) or closed (forming a Jordan curve).
   * Interaction operations such as shifting individual vertices or the entire contour
   * are provided directly in the API without requiring a separate interactor class.
   *
   * ContourModel inherits from mitk::BaseData and therefore integrates with the MITK
   * data management framework (DataStorage, DataNode, TimeGeometry).
   *
   * \section mitkContourModelDisplayOptions Display Options
   *
   * The default mappers are mitk::ContourModelMapper2D and mitk::ContourModelMapper3D.
   * See those classes for display properties that can be configured via DataNode properties.
   *
   * \sa ContourElement, ContourModelSet, ContourModelMapper2D, ContourModelMapper3D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModel : public BaseData
  {
  public:
    mitkClassMacro(ContourModel, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /*+++++++++++++++ typedefs +++++++++++++++++++++++++++++++*/
    typedef ContourElement::VertexType VertexType;
    typedef ContourElement::VertexListType VertexListType;
    typedef ContourElement::VertexIterator VertexIterator;
    typedef ContourElement::ConstVertexIterator ConstVertexIterator;
    typedef std::vector<ContourElement::Pointer> ContourModelSeries;
    /*+++++++++++++++ END typedefs ++++++++++++++++++++++++++++*/

    /** \brief Interpolation mode for line segments between control points.
     *
     * Determines how consecutive control points are connected visually.
     */
    enum LineSegmentInterpolation
    {
      LINEAR,   ///< Straight line segments between control points.
      B_SPLINE  ///< B-spline interpolated curve through control points.
    };

    /*++++++++++++++++  inline methods  +++++++++++++++++++++++*/

    /** \brief Get the currently selected vertex.
     * \return Pointer to the selected vertex, or nullptr if no vertex is selected.
     */
    VertexType *GetSelectedVertex() { return this->m_SelectedVertex; }

    /** \brief Clear the current vertex selection.
     * \post GetSelectedVertex() returns nullptr.
     */
    void Deselect() { this->m_SelectedVertex = nullptr; }

    /** \brief Set or clear the control point flag on the currently selected vertex.
     * \param[in] isControlPoint If true, marks the selected vertex as a control point;
     *            if false, removes the control point designation. Default is true.
     * \note Does nothing if no vertex is currently selected.
     */
    void SetSelectedVertexAsControlPoint(bool isControlPoint = true)
    {
      if (this->m_SelectedVertex)
      {
        m_SelectedVertex->IsControlPoint = isControlPoint;
        this->Modified();
      }
    }

    /** \brief Set the interpolation mode for line segments between control points.
     * \param[in] interpolation The desired interpolation mode.
     * \sa GetLineSegmentInterpolation
     */
    void SetLineSegmentInterpolation(LineSegmentInterpolation interpolation)
    {
      this->m_lineInterpolation = interpolation;
      this->Modified();
    }

    /** \brief Get the current interpolation mode for line segments between control points.
     * \return The current LineSegmentInterpolation mode.
     * \sa SetLineSegmentInterpolation
     */
    LineSegmentInterpolation GetLineSegmentInterpolation() { return this->m_lineInterpolation; }
    /*++++++++++++++++  END inline methods  +++++++++++++++++++++++*/

    /** \brief Add a vertex to the contour at given timestep.
    The vertex is added at the end of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(const Point3D &vertex, TimeStepType timestep = 0);

    /** \brief Add a vertex to the contour at given timestep.
    A copy of the passed vertex is added at the end of contour.
    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)
    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(const VertexType &vertex, TimeStepType timestep = 0);

    /** \brief Add a vertex to the contour.
    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)
    \param isControlPoint - specifies the vertex to be handled in a special way (e.g. control points
    will be rendered).
    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertex(const Point3D& vertex, bool isControlPoint, TimeStepType timestep = 0);

    /** Clears the contour of destinationTimeStep and copies
        the contour of the passed source model at the sourceTimeStep.
     @pre sourceModel must point to a valid instance
     @pre sourceTimePoint must be valid
     @note Updating a vertex to a timestep which exceeds the timebounds of the contour
      will not be added, the TimeGeometry will not be expanded.
    */
    void UpdateContour(const ContourModel* sourceModel, TimeStepType destinationTimeStep, TimeStepType sourceTimeStep);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.
    The vertex is added at the FRONT of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(const Point3D &vertex, TimeStepType timestep = 0);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.
    The vertex is added at the FRONT of contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)

    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(const VertexType &vertex, TimeStepType timestep = 0);

    /** \brief Add a vertex to the contour at given timestep AT THE FRONT of the contour.

    \param vertex - coordinate representation of a control point
    \param timestep - the timestep at which the vertex will be add ( default 0)
    \param isControlPoint - specifies the vertex to be handled in a special way (e.g. control points
    will be rendered).


    @note Adding a vertex to a timestep which exceeds the timebounds of the contour
    will not be added, the TimeGeometry will not be expanded.
    */
    void AddVertexAtFront(const Point3D &vertex, bool isControlPoint, TimeStepType timestep = 0);

    /** \brief Insert a vertex at given index.
    */
    void InsertVertexAtIndex(const Point3D &vertex, int index, bool isControlPoint = false, TimeStepType timestep = 0);

    /** \brief Set a coordinates for point at given index.
    */
    bool SetVertexAt(int pointId, const Point3D &point, TimeStepType timestep = 0);

    /** \brief Set a coordinates and control state for point at given index.
    */
    bool SetVertexAt(int pointId, const VertexType *vertex, TimeStepType timestep = 0);

    /** \brief Return if the contour is closed or not.
    */
    bool IsClosed(int timestep = 0) const;

    /** \brief Concatenate two contours.
    The starting control point of the other will be added at the end of the contour.
    \param other
    \param timestep - the timestep at which the vertex will be add ( default 0)
    \param check - check for intersections ( default false)
    */
    void Concatenate(ContourModel *other, TimeStepType timestep = 0, bool check = false);

    /** \brief Returns a const VertexIterator at the start element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator Begin(TimeStepType timestep = 0) const;

    /** \brief Returns a const VertexIterator at the start element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator IteratorBegin(TimeStepType timestep = 0) const;

    /** \brief Returns a const VertexIterator at the end element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator End(TimeStepType timestep = 0) const;

    /** \brief Returns a const VertexIterator at the end element of the contour.
    @throw mitk::Exception if the timestep is invalid.
    */
    VertexIterator IteratorEnd(TimeStepType timestep = 0) const;

    /** \brief Close the contour.
    The last control point will be linked with the first point.
    */
    virtual void Close(TimeStepType timestep = 0);

    /** \brief Set isClosed to false contour.
    The link between the last control point the first point will be removed.
    */
    virtual void Open(TimeStepType timestep = 0);

    /** \brief Set closed property to given boolean.

    false - The link between the last control point the first point will be removed.
    true - The last control point will be linked with the first point.
    */
    virtual void SetClosed(bool isClosed, TimeStepType timestep = 0);

    /** \brief Returns the number of vertices at a given timestep.
    \param timestep - default = 0
    */
    int GetNumberOfVertices(TimeStepType timestep = 0) const;

    /** \brief Returns whether the contour model is empty at a given timestep.
    \param timestep - default = 0
    */
    virtual bool IsEmpty(TimeStepType timestep) const;

    /** \brief Returns whether the contour model is empty.
    */
    bool IsEmpty() const override;

    /** \brief Returns the vertex at the index position within the container.
    * If the index or timestep is invalid a nullptr will be returned.
    */
    virtual const VertexType *GetVertexAt(int index, TimeStepType timestep = 0) const;

    /** \brief Find the nearest vertex to a given 3D position at a specific time step.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for the search.
     * \param[in] timestep Time step to query.
     * \return Const pointer to the nearest vertex, or nullptr if the timestep is invalid or no vertex is within eps.
     */
    const VertexType *GetVertexAt(mitk::Point3D &point, float eps, TimeStepType timestep) const;

    /** \brief Return the next control vertex after the nearest vertex to a given 3D position.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for the search.
     * \param[in] timestep Time step to query.
     * \return Const pointer to the next control vertex, or nullptr if the timestep is invalid or no vertex is found.
     */
    virtual const VertexType *GetNextControlVertexAt(mitk::Point3D &point, float eps, TimeStepType timestep) const;

    /** \brief Return the previous control vertex before the nearest vertex to a given 3D position.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum Euclidean distance for the search.
     * \param[in] timestep Time step to query.
     * \return Const pointer to the previous control vertex, or nullptr if the timestep is invalid or no vertex is found.
     */
    virtual const VertexType *GetPreviousControlVertexAt(mitk::Point3D &point, float eps, TimeStepType timestep) const;

    /** \brief Return the index of a given vertex within the contour at the specified time step.
     * \param[in] vertex Pointer to the vertex to find.
     * \param[in] timestep Time step to query. Default is 0.
     * \return Index of the vertex, or -1 if not found or the timestep is invalid.
     */
    int GetIndex(const VertexType *vertex, TimeStepType timestep = 0);

    /** \brief Check whether the given time step has no associated contour data.
     * \param[in] t Time step index.
     * \return True if \p t exceeds the number of available time steps.
     */
    bool IsEmptyTimeStep(unsigned int t) const override;

    /** \brief Check whether a given point lies near the contour at the specified time step.
     * \param[in] point Query position in 3D space.
     * \param[in] eps Maximum squared distance for the proximity test.
     * \param[in] timestep Time step to query.
     * \return True if the point is near the contour, false otherwise.
     */
    bool IsNearContour(Point3D &point, float eps, TimeStepType timestep) const;

    /** Function that searches for the line segment of the contour that is closest to the passed point
    and close enough (distance between point and line segment <= eps). If such an line segment exist,
    the starting vertex and closing vertex of the found segment are passed back.
    @return True indicates that a line segment was found. False indicates that no segment of the contour
    is close enough to the passed point.
    @remark previousVertex and nextVertex are only valid if return is true.*/
    bool GetLineSegmentForPoint(Point3D &point,
                            float eps,
                            TimeStepType timestep,
                            mitk::ContourElement::VertexType *previousVertex = nullptr,
                            mitk::ContourElement::VertexType *nextVertex = nullptr);

    /**Overloaded version that returns additional information (start and end vertex of the line
    closest to the passed point and the closest point on the contour).
    @remark segmentStart, segmentStop and closestContourPoint are only valid if the function returns true.
    */
    bool GetLineSegmentForPoint(const mitk::Point3D& point,
      float eps, TimeStepType timestep, ContourElement::VertexSizeType& segmentStartIndex,
      ContourElement::VertexSizeType& segmentEndIndex, mitk::Point3D& closestContourPoint,
      bool findClosest = true) const;

    /** \brief Mark a vertex at an index in the container as selected.
     */
    bool SelectVertexAt(int index, TimeStepType timestep = 0);

    /** \brief Mark a vertex at an index in the container as control point.
    */
    bool SetControlVertexAt(int index, TimeStepType timestep = 0);

    /** \brief Mark a control vertex at a given position in 3D space.

    \param point - query point in 3D space
    \param eps - radius for nearest neighbour search (error bound).
    \param timestep - search at this timestep

    @return true = vertex found;  false = no vertex found
    */
    bool SelectControlVertexAt(const Point3D &point, float eps, TimeStepType timestep = 0);

    /** \brief Mark a vertex at a given position in 3D space.

    \param point - query point in 3D space
    \param eps - radius for nearest neighbour search (error bound).
    \param timestep - search at this timestep

    @return true = vertex found;  false = no vertex found
    */
    bool SelectVertexAt(const Point3D &point, float eps, TimeStepType timestep = 0);

    /*
        \param point - query point in 3D space
        \param eps - radius for nearest neighbour search (error bound).
        \param timestep - search at this timestep

        @return true = vertex found;  false = no vertex found
        */
    bool SetControlVertexAt(Point3D &point, float eps, TimeStepType timestep = 0);

    /** \brief Remove a vertex at given index within the container.

    @return true = the vertex was successfully removed;  false = wrong index.
    */
    bool RemoveVertexAt(int index, TimeStepType timestep = 0);

    /** \brief Remove a vertex at given timestep within the container.

    @return true = the vertex was successfully removed.
    */
    bool RemoveVertex(const VertexType *vertex, TimeStepType timestep = 0);

    /** \brief Remove a vertex at a query position in 3D space.

    The vertex to be removed will be search by nearest neighbour search.
    Note that possibly no vertex at this position and eps is stored inside
    the contour.

    @return true = the vertex was successfully removed;  false = no vertex found.
    */
    bool RemoveVertexAt(Point3D &point, float eps, TimeStepType timestep = 0);

    /** \brief Shift the currently selected vertex by a translation vector.

    \param translate - the translation vector.
    */
    void ShiftSelectedVertex(Vector3D &translate);

    /** \brief Shift the whole contour by a translation vector at given timestep.

    \param translate - the translation vector.
    \param timestep - at this timestep the contour will be shifted.
    */
    void ShiftContour(Vector3D &translate, TimeStepType timestep = 0);

    /** \brief Clear the storage container at given timestep.

    All control points are removed at
    timestep.
    */
    virtual void Clear(TimeStepType timestep);

    /** \brief Initialize all data objects
    */
    void Initialize() override;

    /** \brief Initialize object with specs of other contour.
    Note: No data will be copied.
    */
    void Initialize(const ContourModel &other);

    /** \brief Returns a list pointing to all vertices that are indicated to be control points.
    */
    VertexListType GetControlVertices(TimeStepType timestep);

    /** \brief Returns the container of the vertices.
     */
    VertexListType GetVertexList(TimeStepType timestep);

    /*++++++++++++++++++ method inherit from base data +++++++++++++++++++++++++++*/
    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    bool VerifyRequestedRegion() override;

    /**
    \brief Inherit from base data - no region support available for contourModel objects.
    */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
    \brief Expand the contour model and its TimeGeometry to given number of timesteps.
    */
    void Expand(unsigned int timeSteps) override;

    /**
    \brief Update the OutputInformation of a ContourModel object

    The BoundingBox of the contour will be updated, if necessary.
    */
    void UpdateOutputInformation() override;

    /**
    \brief Clear the storage container.

    The object is set to initial state. All control points are removed and the number of
    timesteps are set to 1.
    */
    void Clear() override;

    /**
    \brief overwrite if the Data can be called by an Interactor (StateMachine).
    */
    void ExecuteOperation(Operation *operation) override;

    /** \brief Redistributes control vertices with a given period (as number of vertices)
    \param period - the number of vertices between control points.
    \param timestep - at this timestep all lines will be rebuilt.
    */
    virtual void RedistributeControlVertices(int period, TimeStepType timestep);

  protected:
    mitkCloneMacro(Self);

    ContourModel();
    ContourModel(const ContourModel &other);
    ~ContourModel() override;

    // inherit from BaseData. called by Clear()
    void ClearData() override;

    // inherit from BaseData. Initial state of a contour with no vertices and a single timestep.
    void InitializeEmpty() override;

    // Shift a vertex
    static void ShiftVertex(VertexType *vertex, Vector3D &vector);

    // Storage with time resolved support.
    ContourModelSeries m_ContourSeries;

    // The currently selected vertex.
    VertexType *m_SelectedVertex;

    // The interpolation of the line segment between control points.
    LineSegmentInterpolation m_lineInterpolation;

    // only update the bounding geometry if necessary
    bool m_UpdateBoundingBox;
  };

  itkEventMacroDeclaration(ContourModelEvent, itk::AnyEvent);
  itkEventMacroDeclaration(ContourModelShiftEvent, ContourModelEvent);
  itkEventMacroDeclaration(ContourModelSizeChangeEvent, ContourModelEvent);
  itkEventMacroDeclaration(ContourModelAddEvent, ContourModelSizeChangeEvent);
  itkEventMacroDeclaration(ContourModelRemoveEvent, ContourModelSizeChangeEvent);
  itkEventMacroDeclaration(ContourModelExpandTimeBoundsEvent, ContourModelEvent);
  itkEventMacroDeclaration(ContourModelClosedEvent, ContourModelEvent);
}
#endif
