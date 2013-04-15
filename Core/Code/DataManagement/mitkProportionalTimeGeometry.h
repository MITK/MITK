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

//MITK
#include <mitkTimeGeometry.h>
#include <mitkBaseGeometry.h>
#include <mitkCommon.h>
#include <MitkExports.h>
#include "mitkOperationActor.h"
#include "mitkVector.h"

// To be replaced
#include <mitkSlicedGeometry3D.h>

// STL
#include <vector>

//ITK
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include <itkObject.h>

namespace mitk {

//  typedef itk::BoundingBox<unsigned long, 3, double>   BoundingBox;
//  typedef itk::FixedArray<ScalarType,2>                TimeBounds;

  class MITK_CORE_EXPORT ProportionalTimeGeometry : public TimeGeometry
  {
  public:

    ProportionalTimeGeometry();
//    mitkClassMacro(ProportionalTimeGeometry, TimeGeometry);
    typedef ProportionalTimeGeometry self;
    itkNewMacro(self);

    virtual TimeStepType     GetNumberOfTimeSteps();
    virtual TimePointType    GetMinimumTimePoint ();
    virtual TimePointType    GetMaximumTimePoint ();

    //##Documentation
    //## @brief Get the time bounds (in ms)
    virtual TimeBounds GetTimeBounds( );

    virtual bool IsValidTimePoint (TimePointType& timePoint);
    virtual bool IsValidTimeStep  (TimeStepType& timeStep);
    virtual TimePointType  TimeStepToTimePoint (TimeStepType& timeStep);
    virtual TimeStepType   TimePointToTimeStep (TimePointType& timePoint);

    virtual BaseGeometry* GetGeometryForTimePoint ( TimePointType timePoint);
    virtual BaseGeometry* GetGeometryForTimeStep  ( TimeStepType timeStep);

    virtual bool IsValid ();

    virtual void Initialize();

    virtual void ExecuteOperation(Operation *);

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
    std::vector<BaseGeometry::Pointer> m_GeometryVector;
    TimePointType m_FirstTimePoint;
    TimePointType m_StepDuration;

  }; // end class ProportialTimeGeometry

} // end namespace MITK
#endif // ProportialTimeGeometry_h