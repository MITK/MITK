/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSet_h
#define mitkPointSet_h

#include <mitkBaseData.h>

#include <itkDefaultDynamicMeshTraits.h>
#include <itkMesh.h>

namespace mitk
{
  /**
   * \brief Data structure which stores a set of points.
   *
   * 3D points are grouped within a point set; for time resolved usage, one point
   * set is created and maintained per time step. A point entry consists of the
   * point coordinates and point data.
   *
   * The point data includes a point ID (unique identifier to address this point
   * within the point set), the selection state of the point and the type of
   * the point.
   *
   * For further information about different point types see
   * mitk::PointSpecificationType in mitkVector.h.
   *
   * Inserting a point is accompanied by an event, containing an index. The new
   * point is inserted into the list at the specified position. At the same time
   * an internal ID is generated and stored for the point. Points at specific time
   * steps are accessed by specifying the time step number (which defaults to 0).
   *
   * The points of itk::PointSet stores the points in a pointContainer
   * (MapContainer). The points are best accessed by using a ConstIterator (as
   * defined in MapContainer); avoid access via index.
   *
   * The class internally uses an itk::Mesh for each time step.
   *
   * \section mitkPointSetDisplayOptions
   *
   * The default mappers for this data structure are mitk::PointSetVtkMapper2D and
   * mitk::PointSetVtkMapper3D. See these classes for display options which can
   * can be set via properties.
   *
   * \section Events
   *
   * PointSet issues the following events, for which observers can register
   * (the below events are grouped into a class hierarchy as indicated by
   * indentation level; e.g. PointSetSizeChangeEvent comprises PointSetAddEvent
   * and PointSetRemoveEvent):
   *
   * <tt>
   * PointSetEvent <i>subsumes all PointSet events</i>
   *   PointSetMoveEvent <i>issued when a point of the PointSet is moved</i>
   *   PointSetSizeChangeEvent <i>subsumes add and remove events</i>
   *     PointSetAddEvent <i>issued when a point is added to the PointSet</i>
   *     PointSetRemoveEvent <i>issued when a point is removed from the PointSet</i>
   * </tt>
   * \ingroup PSIO
   * \ingroup Data
   */
  class MITKCORE_EXPORT PointSet : public BaseData
  {
  public:
    mitkClassMacro(PointSet, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Scalar type for point coordinates. */
    typedef mitk::ScalarType CoordinateType;

    /** \brief Scalar type for interpolation weights. */
    typedef mitk::ScalarType InterpolationWeightType;

    /** \brief Points are 3-dimensional. */
    static const unsigned int PointDimension = 3;

    /** \brief Maximum topological dimension of mesh cells. */
    static const unsigned int MaxTopologicalDimension = 3;

    /**
     * \brief Data associated with each point in the PointSet.
     *
     * Contains a unique ID, selection state, and a type specification
     * (see mitk::PointSpecificationType).
     */
    struct MITKCORE_EXPORT PointDataType
    {
      unsigned int id;                        /**< \brief Unique identifier for this point. */
      bool selected;                          /**< \brief Whether the point is selected. */
      mitk::PointSpecificationType pointSpec; /**< \brief Type specification of the point. */

      /**
       * \brief Equality comparison operator.
       * \param[in] other The PointDataType to compare with.
       * \return True if id, selected, and pointSpec are all equal.
       */
      bool operator==(const PointDataType &other) const;
    };

    /**
     * \brief Type for storing indices of selected lines between points.
     *
     * For example, given points A, B, and C: index 0 is the line between A
     * and B, index 1 is between B and C, and index 2 between C and A.
     */
    typedef std::vector<unsigned int> SelectedLinesType;

    /** \brief Iterator for SelectedLinesType. */
    typedef SelectedLinesType::iterator SelectedLinesIter;

    /**
     * \brief Data associated with a cell in the mesh.
     */
    struct CellDataType
    {
      bool selected;                   /**< \brief Whether the entire cell is selected. */
      SelectedLinesType selectedLines; /**< \brief Indices of selected lines within the cell. */
      bool closed;                     /**< \brief Whether the polygon is finished and closed. */
    };

    typedef itk::DefaultDynamicMeshTraits<PointDataType,
                                          PointDimension,
                                          MaxTopologicalDimension,
                                          CoordinateType,
                                          InterpolationWeightType,
                                          CellDataType>
      MeshTraits;
    typedef itk::Mesh<PointDataType, PointDimension, MeshTraits> MeshType;

    typedef MeshType DataType;
    typedef Point3D PointType;
    typedef DataType::PointIdentifier PointIdentifier;
    typedef DataType::PointsContainer PointsContainer;
    typedef DataType::PointsContainerIterator PointsIterator;
    typedef DataType::PointsContainer::ConstIterator PointsConstIterator;
    typedef DataType::PointDataContainer PointDataContainer;
    typedef DataType::PointDataContainerIterator PointDataIterator;
    typedef DataType::PointDataContainerIterator PointDataConstIterator;

    /**
     * \brief Expand the PointSet to the given number of time steps.
     *
     * Creates new empty point containers for the additional time steps.
     *
     * \param[in] timeSteps The desired number of time steps.
     */
    void Expand(unsigned int timeSteps) override;

    /**
     * \brief Execute an operation on this PointSet.
     *
     * Supports OpINSERT, OpMOVE, OpREMOVE, OpSELECTPOINT, OpDESELECTPOINT,
     * OpSETPOINTTYPE, OpMOVEPOINTUP, and OpMOVEPOINTDOWN.
     *
     * \param[in] operation The operation to execute. Must be a PointOperation.
     * \sa mitk::PointOperation, mitk::OperationActor
     */
    void ExecuteOperation(Operation *operation) override;

    /**
     * \brief Get the number of points at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return The number of points, or 0 if the time step is out of range.
     */
    virtual int GetSize(unsigned int t = 0) const;

    /**
     * \brief Get the number of time steps in the point set series.
     *
     * \return The number of time steps.
     */
    virtual unsigned int GetPointSetSeriesSize() const;

    /**
     * \brief Get the underlying itk::Mesh at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return Smart pointer to the mesh data, or nullptr if \a t is out of range.
     */
    virtual DataType::Pointer GetPointSet(int t = 0) const;

    /**
     * \brief Get an iterator pointing to the first point at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return A PointsIterator to the first point, or End() if the time step is
     *         out of range.
     */
    PointsIterator Begin(int t = 0);

    /** \copydoc Begin(int) */
    PointsConstIterator Begin(int t = 0) const;

    /**
     * \brief Get an iterator pointing past the last point at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return A PointsIterator past the last point.
     */
    PointsIterator End(int t = 0);

    /** \copydoc End(int) */
    PointsConstIterator End(int t = 0) const;

    /**
     * \brief Get an iterator to the element with the maximum ID at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return A PointsIterator to the max-ID element, or End() if the PointSet
     *         is empty or the time step is out of range.
     */
    PointsIterator GetMaxId(int t = 0);

    /**
     * \brief Get the point with the given ID in world coordinates.
     *
     * If the ID does not exist, returns (0, 0, 0).
     *
     * \param[in] id The point identifier.
     * \param[in] t Time step (default: 0).
     * \return The point in world coordinates.
     */
    PointType GetPoint(PointIdentifier id, int t = 0) const;

    /**
     * \brief Get the point with the given ID if it exists.
     *
     * If a point exists for the ID, it is written to \a point (in world
     * coordinates) and the method returns true. Otherwise returns false.
     *
     * \param[in] id The point identifier.
     * \param[out] point Receives the point coordinates on success.
     * \param[in] t Time step (default: 0).
     * \return True if the point was found, false otherwise.
     */
    bool GetPointIfExists(PointIdentifier id, PointType *point, int t = 0) const;

    /**
     * \brief Set (overwrite) the point with the given ID in world coordinates.
     *
     * If the time step does not exist, the PointSet is expanded. The point
     * data is initialized with default values (unselected, PTUNDEFINED).
     *
     * \param[in] id The point identifier.
     * \param[in] point The point in world coordinates.
     * \param[in] t Time step (default: 0).
     */
    void SetPoint(PointIdentifier id, PointType point, int t = 0);

    /**
     * \brief Set (overwrite) the point with the given ID and PointSpecificationType.
     *
     * \param[in] id The point identifier.
     * \param[in] point The point in world coordinates.
     * \param[in] spec The point specification type.
     * \param[in] t Time step (default: 0).
     */
    void SetPoint(PointIdentifier id, PointType point, PointSpecificationType spec, int t = 0);

    /**
     * \brief Insert a point with the given ID in world coordinates.
     *
     * Unlike SetPoint, this uses the container's InsertElement, which
     * preserves existing elements at other indices.
     *
     * \param[in] id The point identifier.
     * \param[in] point The point in world coordinates.
     * \param[in] t Time step (default: 0).
     */
    void InsertPoint(PointIdentifier id, PointType point, int t = 0);

    /**
     * \brief Insert a point with the given ID, world coordinates, and
     * PointSpecificationType.
     *
     * \param[in] id The point identifier.
     * \param[in] point The point in world coordinates.
     * \param[in] spec The point specification type.
     * \param[in] t Time step.
     */
    void InsertPoint(PointIdentifier id, PointType point, PointSpecificationType spec, int t);

    /**
     * \brief Insert a point with an automatically assigned ID (max ID + 1).
     *
     * \param[in] point The point in world coordinates.
     * \param[in] t Time step (default: 0).
     * \return The identifier assigned to the new point.
     */
    PointIdentifier InsertPoint(PointType point, int t = 0);

    /**
     * \brief Remove the point with the given ID at time step \a t, if it exists.
     *
     * \param[in] id The point identifier.
     * \param[in] t Time step (default: 0).
     * \return True if the point was found and removed, false otherwise.
     */
    bool RemovePointIfExists(PointIdentifier id, int t = 0);

    /**
     * \brief Remove the point with the maximum ID at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return An iterator to the new last point, or End() if the set is empty.
     */
    PointsIterator RemovePointAtEnd(int t = 0);

    /**
     * \brief Swap a point with its neighbor in the ordered container.
     *
     * If \a moveUpwards is true, the point at \a id is swapped with the point
     * at id-1; otherwise with the point at id+1.
     *
     * \param[in] id The point identifier.
     * \param[in] moveUpwards If true, swap with the preceding point; otherwise
     *            swap with the following point.
     * \param[in] t Time step (default: 0).
     * \return True if the swap was successful, false if the neighbor does not exist.
     */
    bool SwapPointPosition(PointIdentifier id, bool moveUpwards, int t = 0);

    /**
     * \brief Search for a selected point and return its ID.
     *
     * \param[in] t Time step (default: 0).
     * \return The ID of the first selected point, or -1 if none is found.
     */
    virtual int SearchSelectedPoint(int t = 0) const;

    /**
     * \brief Deselect all points in all time steps.
     */
    virtual void ClearSelection();

    /**
     * \brief Check whether a point exists at the given position (index).
     *
     * \param[in] position The point index to check.
     * \param[in] t Time step (default: 0).
     * \return True if a point exists at the given position.
     */
    virtual bool IndexExists(int position, int t = 0) const;

    /**
     * \brief Get the selection state of the point at the given position.
     *
     * \param[in] position The point index.
     * \param[in] t Time step (default: 0).
     * \return True if the point is selected, false otherwise.
     */
    virtual bool GetSelectInfo(int position, int t = 0) const;

    /**
     * \brief Set the selection state of the point at the given position.
     *
     * \param[in] position The point index.
     * \param[in] selected True to select, false to deselect.
     * \param[in] t Time step (default: 0).
     */
    virtual void SetSelectInfo(int position, bool selected, int t = 0);

    /**
     * \brief Get the PointSpecificationType of the point at the given position.
     *
     * \param[in] position The point index.
     * \param[in] t Time step.
     * \return The point specification type, or PTUNDEFINED if the point does not exist.
     */
    virtual PointSpecificationType GetSpecificationTypeInfo(int position, int t) const;

    /**
     * \brief Get the number of selected points at time step \a t.
     *
     * \param[in] t Time step (default: 0).
     * \return The number of selected points.
     */
    virtual int GetNumberOfSelected(int t = 0) const;

    /**
     * \brief Search for the closest point within a given distance.
     *
     * \param[in] point The search position in world coordinates.
     * \param[in] distance The search radius in mm.
     * \param[in] t Time step (default: 0).
     * \return The ID of the closest point within \a distance, or -1 if no
     *         point was found.
     */
    int SearchPoint(Point3D point, ScalarType distance, int t = 0) const;

    /**
     * \brief Check whether the point set is empty at time step \a t.
     *
     * \param[in] t The time step to check.
     * \return True if the PointSet is initialized and has zero points at time step \a t.
     */
    bool IsEmptyTimeStep(unsigned int t) const override;

    /**
     * \brief Update the output information (bounding box, geometry).
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Set the requested region to the largest possible region (no-op for PointSet).
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check whether the requested region is outside the buffered region.
     *
     * \return Always returns false for PointSet.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify that the requested region is valid.
     *
     * \return Always returns true for PointSet.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object (no-op for PointSet).
     *
     * \param[in] data The data object (unused).
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Hook method called when the point set content changes.
     *
     * Subclasses can override this to react to modifications.
     */
    virtual void OnPointSetChange(){};

  protected:
    mitkCloneMacro(Self);

    PointSet();
    PointSet(const PointSet &other);
    ~PointSet() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override; ///< print content of the object to os

    void ClearData() override;

    void InitializeEmpty() override;

    /** \brief swaps point coordinates and point data of the points with identifiers id1 and id2 */
    bool SwapPointContents(PointIdentifier id1, PointIdentifier id2, int t = 0);

    typedef std::vector<DataType::Pointer> PointSetSeries;

    PointSetSeries m_PointSetSeries;

    DataType::PointsContainer::Pointer m_EmptyPointsContainer;

    /**
    * @brief flag to indicate the right time to call SetBounds
    **/
    bool m_CalculateBoundingBox;
  };

  /**
   * @brief Equal A function comparing two pointsets for being identical.
   * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const
   * mitk::PointSet& p1, const mitk::PointSet& p2) instead.
   *
   * @ingroup MITKTestingAPI
   *
   * The function compares the Geometry, the size and all points element-wise.
   * The parameter eps is a tolarence value for all methods which are internally used for comparison.
   *
   * @param rightHandSide Compare this against leftHandSide.
   * @param leftHandSide Compare this against rightHandSide.
   * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
   * @param verbose Flag indicating if the user wants detailed console output or not.
   * @param checkGeometry if comparing point sets loaded from a file, the geometries might be different and must not be
   * compared. In all other cases, you should compare the geometries.
   * @return True, if all subsequent comparisons are true, false otherwise
   */
  DEPRECATED(MITKCORE_EXPORT bool Equal(const mitk::PointSet *leftHandSide,
                                        const mitk::PointSet *rightHandSide,
                                        mitk::ScalarType eps,
                                        bool verbose,
                                        bool checkGeometry = true));

  /**
   * @brief Equal A function comparing two pointsets for being identical.
   *
   * @ingroup MITKTestingAPI
   *
   * The function compares the Geometry, the size and all points element-wise.
   * The parameter eps is a tolarence value for all methods which are internally used for comparison.
   *
   * @param rightHandSide Compare this against leftHandSide.
   * @param leftHandSide Compare this against rightHandSide.
   * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
   * @param verbose Flag indicating if the user wants detailed console output or not.
   * @param checkGeometry if comparing point sets loaded from a file, the geometries might be different and must not be
   * compared. In all other cases, you should compare the geometries.
   * @return True, if all subsequent comparisons are true, false otherwise
   */
  MITKCORE_EXPORT bool Equal(const mitk::PointSet &leftHandSide,
                             const mitk::PointSet &rightHandSide,
                             mitk::ScalarType eps,
                             bool verbose,
                             bool checkGeometry = true);

  itkEventMacroDeclaration(PointSetEvent, itk::AnyEvent);
  itkEventMacroDeclaration(PointSetMoveEvent, PointSetEvent);
  itkEventMacroDeclaration(PointSetSizeChangeEvent, PointSetEvent);
  itkEventMacroDeclaration(PointSetAddEvent, PointSetSizeChangeEvent);
  itkEventMacroDeclaration(PointSetRemoveEvent, PointSetSizeChangeEvent);
  itkEventMacroDeclaration(PointSetExtendTimeRangeEvent, PointSetEvent);

} // namespace mitk

#endif
