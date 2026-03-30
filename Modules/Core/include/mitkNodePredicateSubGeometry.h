/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodePredicateSubGeometry_h
#define mitkNodePredicateSubGeometry_h

#include <mitkNodePredicateBase.h>
#include <mitkBaseGeometry.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  class BaseData;

  /** \brief Predicate that evaluates if the given DataNode's data object
   * has a geometry that is a sub geometry of the reference geometry.
   *
   * Sub geometry means that both geometries have the same voxel grid (same spacing,
   * same axes, origin is on voxel grid), but the bounding box of the checked geometry
   * is contained or equal to the bounding box of the reference geometry.
   *
   * One can either check the whole time geometry of the data node by defining a
   * reference time geometry or check against one given reference base geometry.
   * If the predicate should check against a base geometry, you can specify the
   * timepoint of the data's time geometry that should be checked. If no timepoint
   * is defined the predicate will evaluate the data geometry in the first timestep.
   * Evaluates to "false" for unsupported or undefined data objects/geometries.
   *
   * One can specify the tolerance/precision of the check via SetCheckPrecision(),
   * SetCheckCoordinatePrecision() or SetCheckDirectionPrecision().
   *
   * \remark The default tolerance for coordinate checks is defined by
   * NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION. The default tolerance
   * for direction checks is defined by
   * NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION.
   * Both are not as strict as mitk::eps. The reason is that, for the typical use of
   * the node predicate, mitk::eps would be too pedantic, as we encounter often
   * rounding differences/errors in real world data sets. For more details,
   * see the documentation of the aforementioned constants.
   * We have introduced two different precision values because differences are less
   * impactful for coordinates than for direction values. Therefore we can relax
   * coordinate checks more than direction checks.
   *
   * \ingroup DataStorage
   */
  class MITKCORE_EXPORT NodePredicateSubGeometry : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateSubGeometry, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateSubGeometry, const BaseGeometry*);
    mitkNewMacro2Param(NodePredicateSubGeometry, const BaseGeometry*, TimePointType);

    /** \brief Sets both CheckCoordinatePrecision and CheckDirectionPrecision to the passed value.
     *
     * \param precision The precision value to use for both coordinate and direction checks.
     */
    void SetCheckPrecision(mitk::ScalarType precision);

    itkSetMacro(CheckCoordinatePrecision, mitk::ScalarType);
    itkGetMacro(CheckCoordinatePrecision, mitk::ScalarType);

    itkSetMacro(CheckDirectionPrecision, mitk::ScalarType);
    itkGetMacro(CheckDirectionPrecision, mitk::ScalarType);

    /** \brief Standard Destructor. */
    ~NodePredicateSubGeometry() override;

    /** \brief Checks if the node's data geometry is a sub geometry of the reference.
     *
     * \param node The DataNode to check.
     * \return True if the data geometry is a sub geometry of the reference within
     *         the configured precision.
     */
    bool CheckNode(const mitk::DataNode *node) const override;

  protected:

    /** \brief Constructor that checks the reference geometry against the first data timepoint.
     *
     * \param refGeometry The reference base geometry to compare against.
     */
    NodePredicateSubGeometry(const BaseGeometry* refGeometry);

    /** \brief Constructor that checks the reference geometry against a specific timepoint.
     *
     * \param refGeometry The reference base geometry to compare against.
     * \param relevantTimePoint The timepoint of the data's time geometry to evaluate.
     */
    NodePredicateSubGeometry(const BaseGeometry* refGeometry, TimePointType relevantTimePoint);

    BaseGeometry::ConstPointer m_RefGeometry;
    TimePointType m_TimePoint;
    /** \brief Indicates if m_TimePoint should be regarded or always the first timestep should be used. */
    bool m_UseTimePoint;
    /** \brief Precision that should be used for the equal coordinate checks. */
    mitk::ScalarType m_CheckCoordinatePrecision;
    /** \brief Precision that should be used for the equal direction checks. */
    mitk::ScalarType m_CheckDirectionPrecision;
  };
} // namespace mitk

#endif
