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

#ifndef ProportionalTimeGeometry_h
#define ProportionalTimeGeometry_h

//MITK
#include <mitkTimeGeometry.h>
#include <mitkGeometry3D.h>
#include <mitkCommon.h>
#include <MitkCoreExports.h>

namespace mitk {
  /**
  * \brief Organizes geometries over proportional time steps
  *
  * For this TimeGeometry implementation it is assumed that
  * the durations of the time steps are equidistant, e.g.
  * the durations of the time steps in one ProportionalTimeGeometry
  * are the same. The geometries of the time steps are independent,
  * and not linked to each other. Since the timeBounds of the
  * geometries are different for each time step it is not possible
  * to set the same geometry to different time steps. Instead
  * copies should be used.
  *
  * \addtogroup geometry
  */
  class MITKCORE_EXPORT ProportionalTimeGeometry : public TimeGeometry
  {
  public:
    mitkClassMacro(ProportionalTimeGeometry, TimeGeometry);

    ProportionalTimeGeometry();
    typedef ProportionalTimeGeometry self;
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
    virtual TimeStepType     CountTimeSteps() const override;
    /**
    * \brief Returns the first time point for which the object is valid.
    *
    * Returns the first valid time point for this geometry. If only one
    * time steps available it usually goes from -max to +max. The time point
    * is given in ms.
    */
    virtual TimePointType    GetMinimumTimePoint () const override;
    /**
    * \brief Returns the last time point for which the object is valid
    *
    * Gives the last time point for which a valid geometrie is saved in
    * this time geometry. The time point is given in ms.
    */
    virtual TimePointType    GetMaximumTimePoint () const override;

    /**
    * \brief Returns the first time point for which the object is valid.
    *
    * Returns the first valid time point for the given TimeStep. The time point
    * is given in ms.
    */
    virtual TimePointType    GetMinimumTimePoint(TimeStepType step) const override;
    /**
    * \brief Returns the last time point for which the object is valid
    *
    * Gives the last time point for the Geometry specified by the given TimeStep. The time point is given in ms.
    */
    virtual TimePointType    GetMaximumTimePoint(TimeStepType step) const override;

    /**
    * \brief Get the time bounds (in ms)
    */
    virtual TimeBounds GetTimeBounds( ) const override;

    /**
    * \brief Get the time bounds for the given TimeStep (in ms)
    */
    virtual TimeBounds GetTimeBounds(TimeStepType step) const override;

    /**
    * \brief Tests if a given time point is covered by this object
    *
    * Returns true if a geometry can be returned for the given time
    * point and falls if not. The time point must be given in ms.
    */
    virtual bool IsValidTimePoint (TimePointType timePoint) const override;
    /**
    * \brief Test for the given time step if a geometry is availible
    *
    * Returns true if a geometry is defined for the given time step.
    * Otherwise false is returned.
    * The time step is defined as positiv number.
    */
    virtual bool IsValidTimeStep  (TimeStepType timeStep) const override;
    /**
    * \brief Converts a time step to a time point
    *
    * Converts a time step to a time point in a way that
    * the new time point indicates the same geometry as the time step.
    * If the original time steps does not point to a valid geometry,
    * a time point is calculated that also does not point to a valid
    * geometry, but no exception is raised.
    */
    virtual TimePointType  TimeStepToTimePoint (TimeStepType timeStep) const override;
        /**
    * \brief Converts a time point to the corresponding time step
    *
    * Converts a time point to a time step in a way that
    * the new time step indicates the same geometry as the time point.
    * If a negativ invalid time point is given always time step 0 is
    * returned. If an positiv invalid time step is given an invalid
    * time step will be returned.
    */
    virtual TimeStepType   TimePointToTimeStep (TimePointType timePoint) const override;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns a clone of the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    */
    virtual BaseGeometry::Pointer GetGeometryCloneForTimeStep( TimeStepType timeStep) const override;

    /**
    * \brief Returns the geometry which corresponds to the given time point
    *
    * Returns the geometry which defines the given time point. If
    * the given time point is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    virtual BaseGeometry::Pointer GetGeometryForTimePoint ( TimePointType timePoint) const override;
    /**
    * \brief Returns the geometry which corresponds to the given time step
    *
    * Returns the geometry which defines the given time step. If
    * the given time step is invalid an null-pointer is returned.
    *
    * If the returned geometry is changed this will affect the saved
    * geometry.
    */
    virtual BaseGeometry::Pointer GetGeometryForTimeStep  ( TimeStepType timeStep) const override;

    /**
    * \brief Tests if all necessary informations are set and the object is valid
    */
    virtual bool IsValid () const override;

    /**
    * \brief Initilizes a new object with one time steps which contains an empty geometry.
    */
    virtual void Initialize() override;

    /**
    * \brief Expands the time geometry to the given number of time steps.
    *
    * Initializes the new time steps with empty geometries if no timesteps
    * in the geometry so far. Otherwise fills the new times steps with
    * clones of the first time step.
    * Shrinking is not supported.
    */
    virtual void Expand(TimeStepType size) override;
    /**
    * \brief Sets the geometry for the given time step
    *
    * This method does not afflict other time steps, since the geometry for
    * each time step is saved individually.
    */
    virtual void SetTimeStepGeometry(BaseGeometry* geometry, TimeStepType timeStep) override;

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
    virtual void ReplaceTimeStepGeometries(const BaseGeometry* geometry) override;

    /**
    * \brief Makes a deep copy of the current object
    */
    virtual itk::LightObject::Pointer InternalClone () const override;

    itkGetConstMacro(FirstTimePoint, TimePointType);
    itkSetMacro(FirstTimePoint, TimePointType);
    itkGetConstMacro(StepDuration, TimePointType);
    itkSetMacro(StepDuration, TimePointType);

//    void SetGeometryForTimeStep(TimeStepType timeStep, BaseGeometry& geometry);
    void ClearAllGeometries ();
//    void AddGeometry(BaseGeometry geometry);
    void ReserveSpaceForGeometries (TimeStepType numberOfGeometries);

    /**
    * \brief Initializes the TimeGeometry with equally time Step geometries
    *
    * Saves a copy for each time step.
    */
    void Initialize (const BaseGeometry* geometry, TimeStepType timeSteps);
    /**
    * \brief Initialize the TimeGeometry with empty BaseGeometry
    */
    void Initialize (TimeStepType timeSteps);

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  protected:
    virtual ~ProportionalTimeGeometry();

    std::vector<BaseGeometry::Pointer> m_GeometryVector;
    TimePointType m_FirstTimePoint;
    TimePointType m_StepDuration;
  }; // end class ProportialTimeGeometry


  /**
  * @brief Equal A function comparing two ProportionalTimeGeometries for being identical.
  *
  * @ingroup MITKTestingAPI
  *
  * The function compares two instances of ProportionalTimeGeometries in all their aspects.
  *
  * The parameter eps is a tolerance value for all methods which are internally used for comparison.
  * If you want to use different tolerance values for different parts of the geometry, feel free to use
  * the other comparison methods and write your own implementation of Equal.
  *
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param eps Tolerance for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  *
  * @return True, if all comparison are true. False in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::ProportionalTimeGeometry& leftHandSide, const mitk::ProportionalTimeGeometry& rightHandSide, ScalarType eps, bool verbose);



} // end namespace MITK
#endif // ProportionalTimeGeometry_h
