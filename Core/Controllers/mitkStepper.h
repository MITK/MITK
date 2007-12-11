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


#ifndef STEPPER_H_HEADER_INCLUDED_C1E77191
#define STEPPER_H_HEADER_INCLUDED_C1E77191

#include "mitkCommon.h"
#include "mitkVector.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <string>

namespace mitk {

/**
 * \brief Helper class to step through a list
 *
 * A helper class to step through a list. Does not contain the list, just the
 * position in the list (between 0 and GetSteps()). Provides methods like
 * First (go to the first element), Next (go to the next one), etc.
 *
 * Besides the actual number of steps, the stepper can also hold a stepping
 * range, indicating the scalar values corresponding to the covered steps.
 * For example, steppers are generally used to slice a dataset with a plane;
 * Hereby, Steps indicates the total number of steps (positions) available for
 * the plane, Pos indicates the current step, and Range indicates the physical
 * minimum and maximum values for the plane, in this case a value in mm.
 *
 * The range can also be supplied with a unit name (a string) which can be
 * used by classes providing information about the stepping (e.g. graphical
 * sliders).
 *
 * \ingroup NavigationControl
 */
class Stepper : public itk::Object
{
public:
  mitkClassMacro(Stepper, itk::Object);
  itkNewMacro(Self);

  itkGetMacro(Pos, unsigned int);
  
  virtual void SetPos(unsigned int pos) 
  { 
    // copied from itkMacro.h, itkSetClampMacro(...)
    if (this->m_Pos != (pos > m_Steps-1 ? m_Steps-1 : pos)) 
    {
      this->m_Pos = pos > m_Steps-1 ? m_Steps-1 : pos ;
      this->Modified();
    }
  }

  itkGetMacro(Steps, unsigned int);
  itkSetMacro(Steps, unsigned int);

  itkGetMacro(AutoRepeat, bool);
  itkSetMacro(AutoRepeat, bool);
  itkBooleanMacro(AutoRepeat);

  /** Causes the stepper to shift direction when the boundary is reached */
  itkSetMacro(PingPong, bool);
  itkGetMacro(PingPong, bool);
  itkBooleanMacro(PingPong);

  /** If set to true, the Next() decreases the stepper and Previous()
   * decreases it */
  itkSetMacro(InverseDirection, bool);
  itkGetMacro(InverseDirection, bool);
  itkBooleanMacro(InverseDirection);

  void SetRange( ScalarType min, ScalarType max );
  void InvalidateRange();
  ScalarType GetRangeMin() const;
  ScalarType GetRangeMax() const;
  bool HasValidRange() const;
  void RemoveRange();
  bool HasRange() const;

  void SetUnitName( const char *unitName );
  const char *GetUnitName() const;
  void RemoveUnitName();
  bool HasUnitName() const;


  virtual void Next();

  virtual void Previous();

  virtual void First();

  virtual void Last();

protected:
  Stepper();
  virtual ~Stepper();

  void Increase();

  void Decrease();

  unsigned int m_Pos;

  unsigned int m_Steps;

  bool m_AutoRepeat;
  
  bool m_PingPong;
  bool m_InverseDirection;

  ScalarType m_RangeMin;
  ScalarType m_RangeMax;
  bool m_RangeValid;
  bool m_HasRange;

  std::string m_UnitName;
  bool m_HasUnitName;

};

} // namespace mitk



#endif /* STEPPER_H_HEADER_INCLUDED_C1E77191 */
