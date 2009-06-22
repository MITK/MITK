/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef TIMESLICEDGEOMETRY_H_HEADER_INCLUDED_C1EBD0AD
#define TIMESLICEDGEOMETRY_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkGeometry3D.h"

namespace mitk {

//##Documentation
//## @brief Describes a geometry consisting of several geometries which
//## exist at different times.
//##
//## The geometry contains m_TimeSteps geometries, which can be accessed
//## using GetGeometry3D(int t). To convert between world-time in
//## milliseconds and the integer timestep-number use MSToTimeStep.
//## The hull (in space and time) of the TimeSlicedGeometry contains all
//## contained geometries.
//## @warning The hull (i.e., transform, bounding-box and
//## time-bounds) is only guaranteed to be up-to-date after calling
//## UpdateInformation().
//##
//## TimeSlicedGeometry and the associated Geometry3Ds have to be
//## initialized in the method GenerateOutputInformation() of BaseProcess (or
//## CopyInformation/ UpdateOutputInformation of BaseData, if possible, e.g.,
//## by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//##
//## @ingroup Geometry
class MITK_CORE_EXPORT TimeSlicedGeometry : public Geometry3D
{
public:
  mitkClassMacro(TimeSlicedGeometry, Geometry3D);

  itkNewMacro(Self);

  //##Documentation
  //## @brief Re-calculate the hull of the contained geometries.
  //##
  //## The transforms, bounding-box and time-bounds of this
  //## geometry (stored in members of the super-class Geometry3D)
  //## are re-calculated from the contained geometries.
  void UpdateInformation();

  //##Documentation
  //## @brief Get the number of time-steps
  itkGetConstMacro(TimeSteps, unsigned int);

  //##Documentation
  //## @brief Set/Get whether the TimeSlicedGeometry is evenly-timed (m_EvenlyTimed)
  //##
  //## If (a) we don't have a Geometry3D stored for the requested time,
  //## (b) m_EvenlyTimed is activated and (c) the first geometry (t=0)
  //## is set, then we clone the geometry and set the m_TimeBounds accordingly.
  //## \sa GetGeometry3D
   itkGetConstMacro(EvenlyTimed, bool);
  virtual void SetEvenlyTimed(bool on = true);

  //##Documentation
  //## @brief Set the Geometry3D for time @a t
  virtual bool SetGeometry3D(mitk::Geometry3D* geometry3D, int t);

  //##Documentation
  //## @brief Get the Geometry3D at time @a t
  virtual mitk::Geometry3D* GetGeometry3D(int t) const;

  //##Documentation
  //## @brief Test whether @a t is a valid time step
  virtual bool IsValidTime(int t) const;

  //##Documentation
  //## @brief Convert time in ms to a time step
  virtual int MSToTimeStep(mitk::ScalarType time_in_ms) const;

  //##Documentation
  //## @brief Convert time step to time in ms
  virtual mitk::ScalarType TimeStepToMS(int timestep) const;

  //##Documentation
  //## @brief Completely initialize this instance as evenly-timed with
  //## \a timeSteps geometries of type Geometry3D, each initialized by
  //## Geometry3D::Initialize().
  virtual void Initialize(unsigned int timeSteps);

  //##Documentation
  //## @brief Completely initialize this instance as evenly-timed with
  //## \a timeSteps geometries identical to the provided Geometry3D
  //## except for the time bounds
  virtual void InitializeEvenlyTimed(mitk::Geometry3D* geometry3D, unsigned int timeSteps);

  //##Documentation
  //## @brief Initialize this instance to contain \a timeSteps
  //## geometries, but without setting them yet
  virtual void InitializeEmpty(unsigned int timeSteps);

  //##Documentation
  //## @brief Expand the number of time steps contained
  //## to \a timeSteps.
  //##
  //## New, additional time steps will be initialized empty.
  //## Only enlargement of the time steps vector is intended and possible.
  virtual void ExpandToNumberOfTimeSteps( unsigned int timeSteps );

  virtual void SetImageGeometry(const bool isAnImageGeometry);

  //##Documentation
  //## @brief Copy the m_TimeBounds of the geometries contained
  //## in timeslicedgeometry into the geometries contained in this
  //## TimeSlicedGeometry object.
  //##
  //## Useful for initialization of the TimeSlicedGeometry of the
  //## output in GenerateOutputInformation() methods of process objects,
  //## see for example BoundingObjectCutter::GenerateOutputInformation().
  //## @param t start time index
  //## @param endtimeindex (endtimeindex) is the time index of
  //## the last geometry whose time-bounds are copied. If
  //## timeslicedgeometry or this TimeSlicedGeometry object does
  //## not contain enough geometries, endtimeindex is reduced
  //## appropriately.
  void CopyTimes(const mitk::TimeSlicedGeometry* timeslicedgeometry, unsigned int t=0, unsigned int endtimeindex = itk::NumericTraits<unsigned int>::max());

  //##Documentation
  //## @brief duplicates the geometry
  virtual AffineGeometryFrame3D::Pointer Clone() const;

  virtual void ExecuteOperation(Operation* operation);
protected:
  TimeSlicedGeometry();

  virtual ~TimeSlicedGeometry();

  void InitializeGeometry(Self * newGeometry) const;

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  mutable std::vector<Geometry3D::Pointer> m_Geometry3Ds;

  //##Documentation
  //## @brief Number of time steps
  unsigned int m_TimeSteps;

  //##Documentation
  //## @brief \a true in case the time steps have equal length
  bool m_EvenlyTimed;

  static const std::string EVENLY_TIMED;
  static const std::string TIME_STEPS;
};

} // namespace mitk

#endif /* TIMESLICEDGEOMETRY_H_HEADER_INCLUDED_C1EBD0AD */
