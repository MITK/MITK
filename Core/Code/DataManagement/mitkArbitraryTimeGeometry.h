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

#ifndef ArbitraryTimeGeometry_h
#define ArbitraryTimeGeometry_h

//MITK
#include <mitkTimeGeometry.h>
#include <mitkCommon.h>
#include <MitkCoreExports.h>

namespace mitk {

  /**
  * \brief Organizes geometries over arbitrary defined time steps
  *
  * For this TimeGeometry implementation it is assumed that
  * the durations of the time steps are arbitrary and may differ.
  * The geometries of the time steps are independent,
  * and not linked to each other. Since the timeBounds of the
  * geometries are different for each time step it is not possible
  * to set the same geometry to different time steps. Instead
  * copies should be used.
  * @remark The lower time bound of a succeeding time step may not be smaller or equal
  * to the lower time bound of its predecessor. Thus the list of time points is
  * allways sorted by its lower time bounds.
  * @remark For the conversion between time step and time point the following assumption
  * is used.:\n
  * time step -> time point: time point is the lower time bound of the geometry indicated by step.\n
  * time point -> time step: associated time step is last step which lower time bound is smaller or equal then the time point.
  *
  * \addtogroup geometry
  */
  class MITK_CORE_EXPORT ArbitraryTimeGeometry : public TimeGeometry
  {
  public:
    mitkClassMacro(ArbitraryTimeGeometry, TimeGeometry);

    ArbitraryTimeGeometry();
    typedef ArbitraryTimeGeometry self;
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief Returns the number of time steps.
    *
    * Returns the number of time steps for which
    * geometries are saved. The number of time steps
    * is also the upper bound of the time steps. The
    * minimum time steps is always 0.
    */
    virtual TimeStepType     CountTimeSteps() const;
    /**
    * \brief Returns the first time point for which the object is valid.
    *
    * Returns the first valid time point for this geometry. It is the lower time bound of
    * the first step. The time point is given in ms.
    */
    virtual TimePointType    GetMinimumTimePoint () const;
    /**
    * \brief Returns the last time point for which the object is valid
    *
    * Gives the last time point for which a valid geometrie is saved in
    * this time geometry. It is the upper time bound of the last step.
    * The time point is given in ms.
    */
    virtual TimePointType    GetMaximumTimePoint () const;

    /**
    * \brief Get the time bounds (in ms)
    * it returns GetMinimumTimePoint() and GetMaximumTimePoint() results as bounds.
    */
    virtual TimeBounds GetTimeBounds( ) const;

    /**
    * \brief Tests if a given time point is covered by this object
    *
    * Returns true if a geometry can be returned for the given time
    * point (so it is within GetTimeBounds() and fails if not.
    * The time point must be given in ms.
    */
    virtual bool IsValidTimePoint (TimePointType timePoint) const;
    /**
    * \brief Test for the given time step if a geometry is availible
    *
    * Returns true if a geometry is defined for the given time step.
    * Otherwise false is returned.
    * The time step is defined as positiv number.
    */
    virtual bool IsValidTimeStep  (TimeStepType timeStep) const;
    /**
    * \brief Converts a time step to a time point
    *
    * Converts a time step to a time point by using the time steps lower
    * time bound.
    * If the original time steps does not point to a valid geometry,
    * a time point is calculated that also does not point to a valid
    * geometry, but no exception is raised.
    */
    virtual TimePointType  TimeStepToTimePoint (TimeStepType timeStep) const;
        /**
    * \brief Converts a time point to the corresponding time step
    *
    * Converts a time point to a time step in a way that
    * the new time step indicates the same geometry as the time point.
    * The associated time step is the last step which lower time bound
    * is smaller or equal then the time point.
    * If a negativ invalid time point is given always time step 0 is
    * returned. If an positiv invalid time step is given an invalid
    * time step will be returned.
    */
    virtual TimeStepType   TimePointToTimeStep (TimePointType timePoint) const;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns a clone of the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    */
    virtual Geometry3D::Pointer GetGeometryCloneForTimeStep( TimeStepType timeStep) const;

    /**
    * \brief Returns the geometry which corresponds to the given time point
    *
    * Returns the geometry which defines the given time point. If
    * the given time point is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    virtual Geometry3D::Pointer GetGeometryForTimePoint ( TimePointType timePoint) const;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    virtual Geometry3D::Pointer GetGeometryForTimeStep  ( TimeStepType timeStep) const;

    /**
    * \brief Tests if all necessary informations are set and the object is valid
    */
    virtual bool IsValid () const;

    /**
    * \brief Initilizes a new object with one time steps which contains an empty geometry.
    */
    virtual void Initialize();

    /**
    * \brief Expands the time geometry to the given number of time steps.
    *
    * Initializes the new time steps with empty geometries. This default geometries will behave like ProportionalTimeGeometry.
    * Shrinking is not supported.
    */
    virtual void Expand(TimeStepType size);
    /**
    * \brief Sets the geometry for the given time step
    *
    * If passed time step is not valid. Nothing will be changed.
    * Setting of time steps may change the time step geometry (count of time steps).
    * Alle other time steps will be removed that match one of the
    * conflict conditions:\n
    * 1. They are befor the set time step, but have larger lower bounds.\n
    * 2. They are after the set time step, but have lower lower bounds.
    */
    virtual void SetTimeStepGeometry(Geometry3D* geometry, TimeStepType timeStep);

    /**
    * \brief Makes a deep copy of the current object
    */
    virtual itk::LightObject::Pointer InternalClone () const;

    void ClearAllGeometries ();
    /** Append the passed geometry to the time geometry.
     * @pre The passed geometry pointer must be valid.
     * @pre The lower bound of the passed geometry must be larger then
     * the lower time bound of the currently last time step.*/
    void AppendTimeStep(Geometry3D* geometry);
    /** Same than AppendTimeStep. But clones geometry befor adding it.*/
    void AppendTimeStepClone(Geometry3D* geometry);
    void ReserveSpaceForGeometries (TimeStepType numberOfGeometries);

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  protected:
    virtual ~ArbitraryTimeGeometry();

    std::vector<Geometry3D::Pointer> m_GeometryVector;
    TimePointType m_MinimumTimePoint;
    TimePointType m_MaximumTimePoint;


  }; // end class ProportialTimeGeometry

} // end namespace MITK
#endif // ArbitraryTimeGeometry_h
