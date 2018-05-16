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

namespace mitk
{
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
  * @remark The lower time bound of a succeeding time step may not be smaller
  * than the upper time bound of its predecessor. Thus the list of time points is
  * always sorted by its lower time bounds.
  * @remark For the conversion between time step and time point the following assumption
  * is used.:\n
  * time step -> time point: time point is the lower time bound of the geometry indicated by step.\n
  * time point -> time step: associated time step is last step which lower time bound is smaller or equal then the time
  * point.
  *
  * \addtogroup geometry
  */
  class MITKCORE_EXPORT ArbitraryTimeGeometry : public TimeGeometry
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
      TimeStepType CountTimeSteps() const override;
    /**
    * \brief Returns the first time point for which the time geometry instance is valid.
    *
    * Returns the first valid time point for this geometry. It is the lower time bound of
    * the first step. The time point is given in ms.
    */
    TimePointType GetMinimumTimePoint() const override;
    /**
    * \brief Returns the last time point for which the time geometry instance is valid
    *
    * Gives the last time point for which a valid geometry is saved in
    * this time geometry. It is the upper time bound of the last step.
    * The time point is given in ms.
    */
    TimePointType GetMaximumTimePoint() const override;

    /**
    * \brief Returns the first time point for which the time geometry instance is valid.
    *
    * Returns the first valid time point for the given TimeStep. The time point
    * is given in ms.
    */
    TimePointType GetMinimumTimePoint(TimeStepType step) const override;
    /**
    * \brief Returns the last time point for which the time geometry instance is valid
    *
    * Gives the last time point for the Geometry specified by the given TimeStep. The time point is given in ms.
    */
    TimePointType GetMaximumTimePoint(TimeStepType step) const override;

    /**
    * \brief Get the time bounds (in ms)
    * it returns GetMinimumTimePoint() and GetMaximumTimePoint() results as bounds.
    */
    TimeBounds GetTimeBounds() const override;

    /**
    * \brief Get the time bounds for the given TimeStep (in ms)
    */
    TimeBounds GetTimeBounds(TimeStepType step) const override;

    /**
    * \brief Tests if a given time point is covered by this time geometry instance
    *
    * Returns true if a geometry can be returned for the given time
    * point (so it is within GetTimeBounds() and fails if not.
    * The time point must be given in ms.
    */
    bool IsValidTimePoint(TimePointType timePoint) const override;
    /**
    * \brief Test for the given time step if a geometry is availible
    *
    * Returns true if a geometry is defined for the given time step.
    * Otherwise false is returned.
    * The time step is defined as positiv number.
    */
    bool IsValidTimeStep(TimeStepType timeStep) const override;
    /**
    * \brief Converts a time step to a time point
    *
    * Converts a time step to a time point by using the time steps lower
    * time bound.
    * If the original time steps does not point to a valid geometry,
    * a time point is calculated that also does not point to a valid
    * geometry, but no exception is raised.
    */
    TimePointType TimeStepToTimePoint(TimeStepType timeStep) const override;
    /**
  * \brief Converts a time point to the corresponding time step
  *
  * Converts a time point to a time step in a way that
  * the new time step indicates the same geometry as the time point.
  * The associated time step is the last step which lower time bound
  * is smaller or equal then the time point.
  * If a negative invalid time point is given always time step 0 is
  * returned. If an positive invalid time step is given an invalid
  * time step will be returned.
  */
    TimeStepType TimePointToTimeStep(TimePointType timePoint) const override;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns a clone of the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    */
    BaseGeometry::Pointer GetGeometryCloneForTimeStep(TimeStepType timeStep) const override;

    /**
    * \brief Returns the geometry which corresponds to the given time point
    *
    * Returns the geometry which defines the given time point. If
    * the given time point is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    BaseGeometry::Pointer GetGeometryForTimePoint(TimePointType timePoint) const override;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    BaseGeometry::Pointer GetGeometryForTimeStep(TimeStepType timeStep) const override;

    /**
    * \brief Tests if all necessary informations are set and the object is valid
    */
    bool IsValid() const override;

    /**
    * \brief Initializes a new object with one time steps which contains an empty geometry.
    */
    void Initialize() override;

    /**
    * \brief Expands the time geometry to the given number of time steps.
    *
    * Initializes the new time steps with empty geometries. This default geometries will behave like
    * ProportionalTimeGeometry.
    * Shrinking is not supported. The new steps will have the same duration like the last step before extension.
    */
    void Expand(TimeStepType size) override;

    /**
    * \brief Replaces the geometry instances with clones of the passed geometry.
    *
    * Replaces the geometries of all time steps with clones of the passed
    * geometry. Replacement strategy depends on the implementation of TimeGeometry
    * sub class.
    * @remark The time points itself stays untouched. Use this method if you want
    * to change the spatial properties of a TimeGeometry and preserve the time
    * "grid".
    */
    void ReplaceTimeStepGeometries(const BaseGeometry *geometry) override;

    /**
    * \brief Sets the geometry for the given time step
    *
    * If passed time step is not valid. Nothing will be changed.
    * @pre geometry must point to a valid instance.
    */
    void SetTimeStepGeometry(BaseGeometry *geometry, TimeStepType timeStep) override;

    /**
    * \brief Makes a deep copy of the current object
    */
    itk::LightObject::Pointer InternalClone() const override;

    void ClearAllGeometries();

    /** Append the passed geometry to the time geometry.
     * @pre The passed geometry pointer must be valid.
     * @pre The minimumTimePoint must not be smaller than the maximum time point of the currently last time step.
     * Therefore time steps must not be overlapping in time.
     * @pre minimumTimePoint must not be larger then maximumTimePoint.*/
    void AppendNewTimeStep(BaseGeometry *geometry, TimePointType minimumTimePoint, TimePointType maximumTimePoint);

    /** Same than AppendNewTimeStep. But clones geometry before adding it.*/
    void AppendNewTimeStepClone(const BaseGeometry* geometry,
                              TimePointType minimumTimePoint,
                              TimePointType maximumTimePoint );

    void ReserveSpaceForGeometries( TimeStepType numberOfGeometries );

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

  protected:
    ~ArbitraryTimeGeometry() override;

    std::vector<BaseGeometry::Pointer> m_GeometryVector;
    std::vector<TimePointType> m_MinimumTimePoints;
    std::vector<TimePointType> m_MaximumTimePoints;

  }; // end class ArbitraryTimeGeometry

} // end namespace MITK
#endif // ArbitraryTimeGeometry_h
