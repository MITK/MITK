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

#ifndef ProportialTimeGeometry_h
#define ProportialTimeGeometry_h

//ITK
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include <itkObject.h>
//MITK
#include <mitkTimeGeometry.h>
#include <mitkCommon.h>
#include <MitkExports.h>
#include "mitkOperationActor.h"
#include "mitkVector.h"

// To be replaced
#include <mitkSlicedGeometry3D.h>

// STL
#include <vector>

namespace mitk {

//  typedef itk::BoundingBox<unsigned long, 3, double>   BoundingBox;
//  typedef itk::FixedArray<ScalarType,2>                TimeBounds;

  class MITK_CORE_EXPORT ProportionalTimeGeometry : public TimeGeometry
  {
  public:
    mitkClassMacro(ProportionalTimeGeometry, TimeGeometry);

    ProportionalTimeGeometry();
    typedef ProportionalTimeGeometry self;
    itkNewMacro(self);

    virtual TimeStepType     GetNumberOfTimeSteps() const;
    virtual TimePointType    GetMinimumTimePoint () const;
    virtual TimePointType    GetMaximumTimePoint () const;

    //##Documentation
    //## @brief Get the time bounds (in ms)
    virtual TimeBounds GetTimeBounds( ) const;

    virtual bool IsValidTimePoint (TimePointType timePoint) const;
    virtual bool IsValidTimeStep  (TimeStepType timeStep) const;
    virtual TimePointType  TimeStepToTimePoint (TimeStepType timeStep) const;
    virtual TimeStepType   TimePointToTimeStep (TimePointType timePoint) const;
    virtual Geometry3D::Pointer GetGeometryCloneForTimeStep( TimeStepType timeStep) const;

    virtual Geometry3D* GetGeometryForTimePoint ( TimePointType timePoint) const;
    virtual Geometry3D* GetGeometryForTimeStep  ( TimeStepType timeStep) const;

    virtual bool IsValid ();

    virtual void Initialize();

    virtual void ExecuteOperation(Operation *);

    virtual void Expand(TimeStepType size);
    virtual void SetTimeStepGeometry(Geometry3D* geometry, TimeStepType timeStep);

    /**
    * \brief Makes a deep copy of the current object
    */
    virtual TimeGeometry::Pointer Clone () const;

    itkGetMacro(FirstTimePoint, TimePointType);
    itkSetMacro(FirstTimePoint, TimePointType);
    itkGetMacro(StepDuration, TimePointType);
    itkSetMacro(StepDuration, TimePointType);

//    void SetGeometryForTimeStep(TimeStepType timeStep, BaseGeometry& geometry);
    void ClearAllGeometries ();
//    void AddGeometry(BaseGeometry geometry);
    void ReserveSpaceForGeometries (TimeStepType numberOfGeometries);

  protected:
    virtual ~ProportionalTimeGeometry();

    BoundingBox::Pointer m_BoundingBox;
    std::vector<Geometry3D::Pointer> m_GeometryVector;
    TimePointType m_FirstTimePoint;
    TimePointType m_StepDuration;

  }; // end class ProportialTimeGeometry

} // end namespace MITK
#endif // ProportialTimeGeometry_h