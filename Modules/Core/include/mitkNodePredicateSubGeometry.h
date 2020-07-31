/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKNODEPREDICATESUBGEOMETRY_H_HEADER_INCLUDED_
#define MITKNODEPREDICATESUBGEOMETRY_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include "mitkBaseGeometry.h"
#include "mitkTimeGeometry.h"

namespace mitk
{
  class BaseData;

  /**Documentation
   @brief Predicate that evaluates if the given DataNode's data object
   has a geometry that is a sub geomety of the reference geometry.
   Sub geometry means that both geometries have the same voxel grid (same spacing, same axes,
   orgin is on voxel grid), but the bounding box of the checked geometry is contained or equal
   to the bounding box of the reference geometry.\n
   One can either check the whole time geometry of
   the data node by defining a referenc time geometry or check against one given2
   reference base geometry. If the predicate should check against a base geometry,
   you can specify the timepoint of the data's time geometry that should be checked.
   If no timepoint is defined the predicate will evaluate the data geometry in
   the first timestep.
   Evaluates to "false" for unsupported or undefined data objects/geometries.

   On can specify the tolerance/precision of the check via SetCheckPrecision().
   @remark The default tolerance is the same as for NodePredicateGeometry and therefore not as strict as mitk::eps. The reason is,
   that, for the typical use of the node predicate, mitk::eps would be to pedantic. We
   often encounter floating point differences and practically it makes no difference e.g.
   if two images differ something like 0.000001 mm in size or spacing.
   @ingroup DataStorage */
  class MITKCORE_EXPORT NodePredicateSubGeometry : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateSubGeometry, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateSubGeometry, const BaseGeometry*);
    mitkNewMacro2Param(NodePredicateSubGeometry, const BaseGeometry*, TimePointType);

    itkSetMacro(CheckPrecision, mitk::ScalarType);
    itkGetMacro(CheckPrecision, mitk::ScalarType);

    ~NodePredicateSubGeometry() override;

    bool CheckNode(const mitk::DataNode *node) const override;

  protected:

    /**Constructor that is used configures the predicate to check the reference geometry against the first data timepoint.*/
    NodePredicateSubGeometry(const BaseGeometry* refGeometry);
    /**Constructor allows to define the timepoint that should be evaluated against the reference.*/
    NodePredicateSubGeometry(const BaseGeometry* refGeometry, TimePointType relevantTimePoint);

    BaseGeometry::ConstPointer m_RefGeometry;
    TimePointType m_TimePoint;
    /**Indicates if m_TimePoint should be regarded or always the first timestep should be used.*/
    bool m_UseTimePoint;
    /**Precision that should be used for the equal checks.*/
    mitk::ScalarType m_CheckPrecision;
  };
} // namespace mitk

#endif
