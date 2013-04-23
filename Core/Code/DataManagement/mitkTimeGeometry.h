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
#include <mitkCommon.h>
#include <MitkExports.h>
#include "mitkOperationActor.h"
#include "mitkVector.h"

// To be replaced
#include <mitkGeometry3D.h>

// STL
#include <vector>


namespace mitk {

//  typedef itk::BoundingBox<unsigned long, 3, double>   BoundingBox;
//  typedef itk::FixedArray<ScalarType,2>                TimeBounds;


//  typedef unsigned long TimePointType;
  typedef float         TimePointType;
  typedef std::size_t   TimeStepType;

  /**
  * \brief Manages the geometries of a data object for each time step
  *
  * For each time step a geometry object is kept, which defines
  * the position and transformation of the BasicObject.
  */
  class MITK_CORE_EXPORT TimeGeometry : public itk::Object, public OperationActor
  {
  protected:
    TimeGeometry();
    virtual ~TimeGeometry();

    /**
    * \brief Contains a bounding box which includes all time steps
    */
    BoundingBox::Pointer m_BoundingBox;


  public:
    mitkClassMacro(TimeGeometry, itk::Object);


    /**
    * \brief Returns the number of time steps.
    */
    virtual TimeStepType     GetNumberOfTimeSteps() const = 0;
    /**
    * \brief Returns the first time point for which the object is valid.
    */
    virtual TimePointType    GetMinimumTimePoint () const = 0;
    /**
    * \brief Returns the last time point for which the object is valid
    */
    virtual TimePointType    GetMaximumTimePoint () const = 0;

    /**
    * \brief Get the time bounds (in ms)
    */
    virtual TimeBounds GetTimeBounds( ) const = 0;
    /**
    * \brief Tests if a given time point is covered by this object
    */
    virtual bool IsValidTimePoint (TimePointType timePoint) const = 0;
    /**
    * \brief Test for the given time step if a geometry is availible
    */
    virtual bool IsValidTimeStep  (TimeStepType timeStep) const = 0;

    /**
    * \brief Converts a time step to a time point
    */
    virtual TimePointType  TimeStepToTimePoint (TimeStepType timeStep) const = 0;
    /**
    * \brief Converts a time point to the corresponding time step
    */
    virtual TimeStepType   TimePointToTimeStep (TimePointType timePoint) const = 0;

    /**
    * \brief Returns the geometry of a specific time point
    */
    virtual Geometry3D* GetGeometryForTimePoint ( TimePointType timePoint) const = 0;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    */
    virtual Geometry3D* GetGeometryForTimeStep ( TimeStepType timeStep) const = 0;

    /**
    * \brief Returns a clone of the geometry of a specific time point
    */
    virtual Geometry3D::Pointer GetGeometryCloneForTimeStep( TimeStepType timeStep) const = 0;
    /**
    * \brief Sets the geometry for a given time step
    */
    virtual void SetTimeStepGeometry(Geometry3D* geometry, TimeStepType timeStep) = 0;

    /**
    * \brief Expands to the given number of time steps
    *
    * Expands to the given number of time steps. Each new created time
    * step is filled with an empty geometry.
    * Shrinking is not supported!
    */
    virtual void Expand(TimeStepType size) = 0;

    /**
    * \brief Tests if all necessary informations are set and the object is valid
    */
    virtual bool IsValid () = 0;
    /**
    * \brief Get the position of the corner number \a id (in world coordinates)
    *
    * See SetImageGeometry for how a corner is defined on images.
    */
    Point3D GetCornerPointInWorld(int id) const;

    /**
    * \brief Get the position of a corner (in world coordinates)
    *
    * See SetImageGeometry for how a corner is defined on images.
    */
    Point3D GetCornerPointInWorld(bool xFront=true, bool yFront=true, bool zFront=true) const;

    /**
    * \brief Get the center of the bounding-box in mm
    */
    Point3D GetCenterInWorld() const;

    /**
    * \brief Get the squared length of the diagonal of the bounding-box in mm
    */
    double GetDiagonalLength2InWorld() const;

    /**
    * \brief Get the length of the diagonal of the bounding-box in mm
    */
    double GetDiagonalLengthinWorld() const;

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
    * \brief Returns a bounding box that covers all time steps
    */
    BoundingBox* GetBoundingBoxInWorld() const
    {
      return m_BoundingBox;
    }

    /**
    * \brief Returns the world bounds of the object that cover all time steps
    */
    BoundingBox::BoundsArrayType GetBoundsInWorld() const
    {
      return m_BoundingBox->GetBounds();
    }

    /**
    * \brief Returns the Extend of the bounding in the given direction
    */
    ScalarType GetExtendInWorld (unsigned int direction) const;

    /**
    * \brief Makes a deep copy of the current object
    */
    virtual TimeGeometry::Pointer Clone () const = 0 ;

    /**
    * \brief Initializes the TimeGeometry
    */
    virtual void Initialize();

    /**
    * \brief Updates the geometry
    */
    void Update();

    /**
    * \brief Updates everything except the Bounding box
    *
    * This class should be overwritten by child classes.
    * The method is called when Update() is required.
    */
    virtual void UpdateWithoutBoundingBox()
    {};

    /**
    * \brief Executes the given operation on all time steps
    */
    virtual void ExecuteOperation(Operation *op);

  }; // end class TimeGeometry

} // end namespace MITK
#endif // TimeGeometry_h