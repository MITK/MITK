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

//ITK
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include <itkObject.h>
//MITK
#include <mitkBaseGeometry.h>
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

  /**
  * \brief Manages the geometries of a data object for each time step
  *
  * For each time step a geometry object is kept, which defines
  * the position and transformation of the BasicObject.
  */
  class MITK_CORE_EXPORT TimeGeometry : public itk::Object, public OperationActor
  {

  public:
    mitkClassMacro(TimeGeometry, itk::Object);

    typedef unsigned long TimePointType;
    typedef std::size_t   TimeStepType;

    /**
    * \brief Returns the number of time steps.
    */
    virtual TimeStepType     GetNumberOfTimeSteps() = 0;
    /**
    * \brief Returns the first time point for which the object is valid.
    */
    virtual TimePointType    GetMinimumTimePoint () = 0;
    /**
    * \brief Returns the last time point for which the object is valid
    */
    virtual TimePointType    GetMaximumTimePoint () = 0;

    /**
    * \brief Get the time bounds (in ms)
    */
    virtual TimeBounds GetTimeBounds( ) = 0;
    /**
    * \brief Tests if a given time point is covered by this object
    */
    virtual bool IsValidTimePoint (TimePointType& timePoint) = 0;
    /**
    * \brief Test for the given time step if a geometry is availible
    */
    virtual bool IsValidTimeStep  (TimeStepType& timeStep) = 0;

    /**
    * \brief Converts a time step to a time point
    */
    virtual TimePointType  TimeStepToTimePoint (TimeStepType& timeStep) = 0;
    /**
    * \brief Converts a time point to the corresponding time step
    */
    virtual TimeStepType   TimePointToTimeStep (TimePointType& timePoint) = 0;

    /**
    * \brief Returns the geometry of a specific time point
    */
    virtual BaseGeometry* GetGeometryForTimePoint ( TimePointType timePoint) = 0;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    */
    virtual BaseGeometry* GetGeometryForTimeStep  ( TimeStepType timeStep) = 0;

    /**
    * \brief Applies the given transformation to all time
    */
    void ApplyTransformMatrixToAllTimeSteps (Transform3D& transformation);

    /**
    * \brief Tests if all necessary informations are set and the object is valid
    */
    virtual bool IsValid () = 0;
    /**
    * \brief Get the position of the corner number \a id (in world coordinates)
    *
    * See SetImageGeometry for how a corner is defined on images.
    */
    Point3D GetCornerPointInWorldSpace(int id) const;

    /**
    * \brief Get the position of a corner (in world coordinates)
    *
    * See SetImageGeometry for how a corner is defined on images.
    */
    Point3D GetCornerPointInWorldSpace(bool xFront=true, bool yFront=true, bool zFront=true) const;

    /**
    * \brief Get the center of the bounding-box in mm
    */
    Point3D GetCenterInWorldSpace() const;

    /**
    * \brief Get the squared length of the diagonal of the bounding-box in mm
    */
    double GetDiagonalLength2InWorldSpace() const;

    /**
    * \brief Get the length of the diagonal of the bounding-box in mm
    */
    double GetDiagonalLengthinWorldSpace() const;

    /**
    * \brief Test whether the point \a p (world coordinates in mm) is inside the bounding box
    */
    bool IsWorldPointInside(const mitk::Point3D& p) const;

    /**
    * \brief Updates the bounding box to cover the area used in all time steps
    *
    * The bounding box is updated by this method. The new bounding box
    * covers an area which includes all bounding boxes during
    * all times steps.
    */
    void UpdateBoundingBox();

    /**
    * \brief Initializes the TimeGeometry
    */
    virtual void Initialize();
  protected:
    TimeGeometry();
    virtual ~TimeGeometry();

    /**
    * \brief Contains a bounding box which includes all time steps
    */
    BoundingBox::Pointer m_BoundingBox;

  }; // end class TimeGeometry

} // end namespace MITK
#endif // TimeGeometry_h