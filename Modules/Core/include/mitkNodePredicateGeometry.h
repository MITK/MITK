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

#ifndef MITKNODEPREDICATEGEOMETRY_H_HEADER_INCLUDED_
#define MITKNODEPREDICATEGEOMETRY_H_HEADER_INCLUDED_

#include "mitkNodePredicateBase.h"
#include "mitkBaseGeometry.h"
#include "mitkTimeGeometry.h"

namespace mitk
{
  class BaseData;

  /**Documentation
   @brief Predicate that evaluates if the given DataNode's data object
   has the same geometry (in terms of spacing, origin, orientation) like
   the reference geometry. One can either check the whole time geometry of
   the date node by defining a referenc time geometry or check against one given
   reference base geometry. If the predicate should check against a base geometry, 
   you can specify the timepoint of the data's time geometry that should be checked.
   If no timepoint is defined the predicate will evaluate the data geometry in
   the first timestep.
   Evaluates to "false" for unsupported or undefined data objects/geometries.
   @ingroup DataStorage */
  class MITKCORE_EXPORT NodePredicateGeometry : public NodePredicateBase
  {
  public:
    mitkClassMacro(NodePredicateGeometry, NodePredicateBase);
    mitkNewMacro1Param(NodePredicateGeometry, const TimeGeometry*);
    mitkNewMacro1Param(NodePredicateGeometry, const BaseGeometry*);
    mitkNewMacro2Param(NodePredicateGeometry, const BaseGeometry*, TimePointType);

    itkSetMacro(CheckPrecision, mitk::ScalarType);
    itkGetMacro(CheckPrecision, mitk::ScalarType);

    ~NodePredicateGeometry() override;

    bool CheckNode(const mitk::DataNode *node) const override;

  protected:

    /**Constructor that is used configures the predicate to check against the the refernce geometry against the first data timepoint.*/
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
    /**Precision that should be used for the equal checks.*/
    mitk::ScalarType m_CheckPrecision;
  };
} // namespace mitk

#endif /* MITKNodePredicateGeometry_H_HEADER_INCLUDED_ */
