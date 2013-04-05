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

#ifndef TimeGeometry_h
#define TimeGeometry_h

//MITK
#include <MitkExports.h>
#include <mitkCommon.h>
#include "mitkOperationActor.h"

// To be replaced
#include <mitkSlicedGeometry3D.h>

// STL
#include <vector>

//ITK
#include <itkObject.h>
#include <itkBoundingBox.h>

namespace mitk {

//  typedef itk::BoundingBox<unsigned long, 3, double>   BoundingBox;
//  typedef itk::FixedArray<ScalarType,2>                TimeBounds;

  class MITK_CORE_EXPORT TimeGeometry : public itk::Object, public OperationActor
  {
    typedef unsigned long TimePointType;
    typedef std::size_t   TimeStepType;

  public:
    virtual TimeStepType GetNumberOfTimeSteps() = 0;
    virtual TimePointType    GetMinimumTimePoint () = 0;
    virtual TimePointType    GetMaximumTimePoint () = 0;

    //##Documentation
    //## @brief Get the time bounds (in ms)
    virtual TimeBounds GetTimeBounds( );

    virtual bool IsValidTimePoint (TimePointType& timePoint) = 0;
    virtual bool IsValidTimeStep  (TimeStepType& timeStep) = 0;

    virtual TimePointType  TimeStepToTimePoint (TimeStepType& timeStep) = 0;
    virtual TimeStepType   TimePointToTimeStep (TimePointType& timePoint) = 0;

    // To be implemented by this class
    virtual void ApplyTransformMatrixToAllTimeSteps (Transform3D& transformation) = 0;

    virtual bool IsValid ()
    //##Documentation
    //## @brief Get the position of the corner number \a id (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    Point3D GetCornerPointInWorldSpace(int id) const;

    //##Documentation
    //## @brief Get the position of a corner (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    Point3D GetCornerPointInWorldSpace(bool xFront=true, bool yFront=true, bool zFront=true) const;

    //##Documentation
    //## @brief Get the center of the bounding-box in mm
    //##
    Point3D GetCenterInWorldSpace() const;

    //##Documentation
    //## @brief Get the squared length of the diagonal of the bounding-box in mm
    //##
    double GetDiagonalLength2InWorldSpace() const;

    //##Documentation
    //## @brief Get the length of the diagonal of the bounding-box in mm
    //##
    double GetDiagonalLengthinWorldSpace() const;

    //##Documentation
    //## @brief Test whether the point \a p (world coordinates in mm) is
    //## inside the bounding box
    bool IsWordlPointInside(const mitk::Point3D& p) const;

    //##Documentation
    //## @brief Updates the bounding box to cover the area used in all time steps
    //##
    //## The bounding box is updated by this method. The new bounding box
    //## covers an area which includes all bounding boxes during
    //## all times steps.
    virtual void UpdateBoundingBox();

    virtual void Initialize();
  protected:
    TimeGeometry();
    virtual ~TimeGeometry();

    BoundingBox::Pointer m_BoundingBox;
  }; // end class TimeGeometry

} // end namespace MITK
#endif // TimeGeometry_h