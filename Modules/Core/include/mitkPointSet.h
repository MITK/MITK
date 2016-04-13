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


#ifndef MITKPointSet_H_HEADER_INCLUDED
#define MITKPointSet_H_HEADER_INCLUDED

#include "mitkBaseData.h"

#include <itkMesh.h>
#include <itkDefaultDynamicMeshTraits.h>


namespace mitk {

/**
 * \brief Data structure which stores a set of points. Superclass of
 * mitk::Mesh.
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
 * The class internally uses an itk::Mesh for each time step, because
 * mitk::Mesh is derived from mitk::PointSet and needs the itk::Mesh structure
 * which is also derived from itk::PointSet. Thus several typedefs which seem
 * to be in wrong place, are declared here (for example SelectedLinesType).
 *
 * \section mitkPointSetDisplayOptions
 *
 * The default mappers for this data structure are mitk::PointSetGLMapper2D and
 * mitk::PointSetVtkMapper3D. See these classes for display options which can
 * can be set via properties.
 *
 * \section Events
 *
 * PointSet issues the following events, for which observers can register
 * (the below events are grouped into a class hierarchy as indicated by
 * identation level; e.g. PointSetSizeChangeEvent comprises PointSetAddEvent
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

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef mitk::ScalarType CoordinateType;
  typedef mitk::ScalarType InterpolationWeightType;

  static const unsigned int PointDimension = 3;
  static const unsigned int MaxTopologicalDimension = 3;

  /**
   * \brief struct for data of a point
   */
  struct MITKCORE_EXPORT PointDataType
  {
    unsigned int id;  //to give the point a special ID
    bool selected;  //information about if the point is selected
    mitk::PointSpecificationType pointSpec;  //specifies the type of the point

    bool operator==(const PointDataType& other) const;
  };

  /**
   * \brief cellDataType, that stores all indexes of the lines, that are
   * selected e.g.: points A,B and C.Between A and B there is a line with
   * index 0. If vector of cellData contains 1 and 2, then the lines between
   * B and C and C and A is selected.
   */
  typedef std::vector<unsigned int> SelectedLinesType;
  typedef SelectedLinesType::iterator SelectedLinesIter;
  struct CellDataType
  {
    //used to set the whole cell on selected
    bool selected;

    //indexes of selected lines. 0 is between pointId 0 and 1
    SelectedLinesType selectedLines;

    //is the polygon already finished and closed
    bool closed;
  };

  typedef itk::DefaultDynamicMeshTraits<
    PointDataType, PointDimension, MaxTopologicalDimension,
    CoordinateType, InterpolationWeightType, CellDataType > MeshTraits;
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

  virtual void Expand( unsigned int timeSteps ) override;

  /** \brief executes the given Operation */
  virtual void ExecuteOperation(Operation* operation) override;

  /** \brief returns the current size of the point-list */
  virtual int GetSize( unsigned int t = 0 ) const;

  virtual unsigned int GetPointSetSeriesSize() const;

  /** \brief returns the pointset */
  virtual DataType::Pointer GetPointSet( int t = 0 ) const;

  PointsIterator Begin( int t = 0 );

  PointsConstIterator Begin( int t = 0 ) const;

  PointsIterator End( int t = 0 );

  PointsConstIterator End( int t = 0 ) const;

  /**
  * \brief Get an iterator to the max ID element if existent. Return End() otherwise.
  */
  PointsIterator GetMaxId( int t = 0 );

  /**
   * \brief Get the point with ID id in world coordinates
   *
   * check if the ID exists. If it doesn't exist, then return 0,0,0
   */
  PointType GetPoint( PointIdentifier id, int t = 0 ) const;

  /**
   * \brief Get the point with ID id in world coordinates
   *
   * If a point exists for the ID id, the point is returned in the parameter point
   * and the method returns true. If the ID does not exist, the method returns false
   */
  bool GetPointIfExists( PointIdentifier id, PointType* point, int t = 0 ) const;

  /**
   * \brief Set the given point in world coordinate system into the itkPointSet.
   */
  void SetPoint( PointIdentifier id, PointType point, int t = 0 );

  /**
  * \brief Set the given  point in world coordinate system with the given PointSpecificationType
  */
  void SetPoint( PointIdentifier id, PointType point, PointSpecificationType spec, int t = 0 );

  /**
   * \brief Set the given point in world coordinate system into the itkPointSet.
   */
  void InsertPoint( PointIdentifier id, PointType point, int t = 0 );

  /**
  * \brief Set the given point in world coordinate system with given PointSpecificationType
  */
  void InsertPoint( PointIdentifier id, PointType point, PointSpecificationType spec, int t );

  /**
  * \brief Insert the given point in world coordinate system with incremented max id at time step t.
  */
  PointIdentifier InsertPoint( PointType point, int t = 0 );

  /**
  * \brief Remove point with given id at timestep t, if existent
  */
  bool RemovePointIfExists( PointIdentifier id, int t = 0 );

  /**
  * \brief Remove max id point at timestep t and return iterator to precedent point
  */
  PointsIterator RemovePointAtEnd( int t = 0 );

  /**
  * \brief Swap a point at the given position (id) with the upper point (moveUpwards=true) or with the lower point (moveUpwards=false).
  * If upper or lower index does not exist false is returned, if swap was successful true.
  */
  bool SwapPointPosition( PointIdentifier id, bool moveUpwards, int t = 0 );


  /**
   * \brief searches a selected point and returns the id of that point.
   * If no point is found, then -1 is returned
   */
  virtual int SearchSelectedPoint( int t = 0 ) const;

  /** \brief returns true if a point exists at this position */
  virtual bool IndexExists( int position, int t = 0 ) const;

  /** \brief to get the state selected/unselected of the point on the
   * position
   */
  virtual bool GetSelectInfo( int position, int t = 0 ) const;

  virtual void SetSelectInfo( int position, bool selected, int t = 0 );

  /** \brief to get the type of the point at the position and the moment */
  virtual PointSpecificationType GetSpecificationTypeInfo( int position, int t ) const;

  /** \brief returns the number of selected points */
  virtual int GetNumberOfSelected( int t = 0 ) const;

  /**
   * \brief searches a point in the list == point +/- distance
   *
   * \param point is in world coordinates.
   * \param distance is in mm.
   * returns -1 if no point is found
   * or the position in the list of the first match
   */
  int SearchPoint( Point3D point, ScalarType distance, int t = 0 ) const;

  virtual bool IsEmptyTimeStep(unsigned int t) const override;

  //virtual methods, that need to be implemented
  virtual void UpdateOutputInformation() override;
  virtual void SetRequestedRegionToLargestPossibleRegion() override;
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
  virtual bool VerifyRequestedRegion() override;
  virtual void SetRequestedRegion(const itk::DataObject *data) override;

  //Method for subclasses
  virtual void OnPointSetChange(){};

protected:

  mitkCloneMacro(Self);

  PointSet();
  PointSet(const PointSet &other);
  virtual ~PointSet();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const override; ///< print content of the object to os

  virtual void ClearData() override;

  virtual void InitializeEmpty() override;

  /** \brief swaps point coordinates and point data of the points with identifiers id1 and id2 */
  bool SwapPointContents(PointIdentifier id1, PointIdentifier id2,  int t = 0 );

  typedef std::vector< DataType::Pointer > PointSetSeries;

  PointSetSeries m_PointSetSeries;

  DataType::PointsContainer::Pointer m_EmptyPointsContainer;

  /**
  * @brief flag to indicate the right time to call SetBounds
  **/
  bool m_CalculateBoundingBox;
};

/**
 * @brief Equal A function comparing two pointsets for beeing identical.
 * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const mitk::PointSet& p1, const mitk::PointSet& p2) instead.
 *
 * @ingroup MITKTestingAPI
 *
 * The function compares the Geometry, the size and all points element-wise.
 * The parameter eps is a tolarence value for all methods which are internally used for comparion.
 *
 * @param rightHandSide Compare this against leftHandSide.
 * @param leftHandSide Compare this against rightHandSide.
 * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
 * @param verbose Flag indicating if the user wants detailed console output or not.
 * @param checkGeometry if comparing point sets loaded from a file, the geometries might be different and must not be compared. In all other cases, you should compare the geometries.
 * @return True, if all subsequent comparisons are true, false otherwise
 */
DEPRECATED(MITKCORE_EXPORT bool Equal(const mitk::PointSet* leftHandSide, const mitk::PointSet* rightHandSide, mitk::ScalarType eps, bool verbose, bool checkGeometry = true));

/**
 * @brief Equal A function comparing two pointsets for beeing identical.
 *
 * @ingroup MITKTestingAPI
 *
 * The function compares the Geometry, the size and all points element-wise.
 * The parameter eps is a tolarence value for all methods which are internally used for comparion.
 *
 * @param rightHandSide Compare this against leftHandSide.
 * @param leftHandSide Compare this against rightHandSide.
 * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
 * @param verbose Flag indicating if the user wants detailed console output or not.
 * @param checkGeometry if comparing point sets loaded from a file, the geometries might be different and must not be compared. In all other cases, you should compare the geometries.
 * @return True, if all subsequent comparisons are true, false otherwise
 */
MITKCORE_EXPORT bool Equal(const mitk::PointSet& leftHandSide, const mitk::PointSet& rightHandSide, mitk::ScalarType eps, bool verbose, bool checkGeometry = true);

itkEventMacro( PointSetEvent, itk::AnyEvent );
itkEventMacro( PointSetMoveEvent, PointSetEvent );
itkEventMacro( PointSetSizeChangeEvent, PointSetEvent );
itkEventMacro( PointSetAddEvent, PointSetSizeChangeEvent );
itkEventMacro( PointSetRemoveEvent, PointSetSizeChangeEvent );
itkEventMacro( PointSetExtendTimeRangeEvent, PointSetEvent );

} // namespace mitk

#endif /* MITKPointSet_H_HEADER_INCLUDED */
