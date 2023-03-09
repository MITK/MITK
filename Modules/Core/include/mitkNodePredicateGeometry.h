/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateGeometry_h
#define mitkNodePredicateGeometry_h

#include "mitkNodePredicateBase.h"
#include "mitkBaseGeometry.h"
#include "mitkTimeGeometry.h"

namespace mitk
{
  class BaseData;

  /**Documentation
   @brief Predicate that evaluates if the given DataNode's data object
   has the same geometry (in terms of spacing, origin, orientation) like
   the reference geometry.

   One can either check the whole time geometry of
   the date node by defining a reference time geometry or check against one given
   reference base geometry. If the predicate should check against a base geometry,
   you can specify the timepoint of the data's time geometry that should be checked.
   If no timepoint is defined the predicate will evaluate the data geometry in
   the first timestep.
   Evaluates to "false" for unsupported or undefined data objects/geometries.

   One can specify the tolerance/precision of the check via SetCheckPrecision(),
   SetCheckCoordinatePrecision() or SetCheckDirectionPrecision().
   @remark The default tolerance for coordinate checks is defined by
   NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION. The default tolerance
   for direction checks is defined by
   NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION.
   Both are not as strict as mitk::eps. The reason is,
   that, for the typical use of the node predicate, mitk::eps would be to pedantic, as we
   encounter often rounding differences/errors in real world data sets. For more details,
   see the documentation of the aforementioned constants.
   We have introduced two different precision values because differences are less
   impactful for coordinates than for direction values. Therefore we can relax coordinate checks
   more then direction checks.
   @ingroup DataStorage */
  class MITKCORE_EXPORT NodePredicateGeometry : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateGeometry, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateGeometry, const TimeGeometry*);
    mitkNewMacro1Param(NodePredicateGeometry, const BaseGeometry*);
    mitkNewMacro2Param(NodePredicateGeometry, const BaseGeometry*, TimePointType);

    /** Sets CheckCoordinatePrecision and CheckDirectionPrecision to the passed value.*/
    void SetCheckPrecision(mitk::ScalarType precision);

    itkSetMacro(CheckCoordinatePrecision, mitk::ScalarType);
    itkGetMacro(CheckCoordinatePrecision, mitk::ScalarType);

    itkSetMacro(CheckDirectionPrecision, mitk::ScalarType);
    itkGetMacro(CheckDirectionPrecision, mitk::ScalarType);

    ~NodePredicateGeometry() override;

    bool CheckNode(const mitk::DataNode *node) const override;

  protected:

    /**Constructor that is used configures the predicate to check the reference geometry against the first data timepoint.*/
    NodePredicateGeometry(const BaseGeometry* refGeometry);
    /**Constructor allows to define the timepoint that should be evaluated against the reference.*/
    NodePredicateGeometry(const BaseGeometry* refGeometry, TimePointType relevantTimePoint);
    /**Constructor that is used configures the predicate to check against the whole time geometry.*/
    NodePredicateGeometry(const TimeGeometry* refGeometry);

    BaseGeometry::ConstPointer m_RefGeometry;
    TimeGeometry::ConstPointer m_RefTimeGeometry;
    TimePointType m_TimePoint;
    /**Indicates if m_TimePoint should be regarded or always the first timestep should be used.*/
    bool m_UseTimePoint;
    /**Precision that should be used for the equal coordinate checks.*/
    mitk::ScalarType m_CheckCoordinatePrecision;
    /**Precision that should be used for the equal direction checks.*/
    mitk::ScalarType m_CheckDirectionPrecision;
  };

  /** The default tolerance for the comparison of spatial/world coordinate equality.
    This tolerance is as strict as mitk::eps. The reason is,
    that, for the typical use of the node predicate, mitk::eps would be to pedantic. We
    often encounter floating point differences and practically it makes no difference e.g.
    if two images differ something like 0.0001 mm in size or spacing or origin.*/
  constexpr double NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION = 1e-4;
  /** The default tolerance for the comparison of direction matrix equality.
    This tolerance is as strict as mitk::eps. The reason is,
    that, for the typical use of the node predicate, mitk::eps would be to pedantic. We
    often encounter floating point differences and practically it makes no difference e.g.
    if the elements of the direction/orientation matrix differ something like 0.000001.*/
  constexpr double NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION = 1e-6;

} // namespace mitk

#endif
