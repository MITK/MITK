/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStepper_h
#define mitkStepper_h

#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <string>

namespace mitk
{
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
  class MITKCORE_EXPORT Stepper : public itk::Object
  {
  public:
    mitkClassMacroItkParent(Stepper, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkGetConstMacro(Pos, unsigned int);

    /** \brief Set the current position, clamped to [0, GetSteps()-1].
     * \param[in] pos The desired position.
     */
    virtual void SetPos(unsigned int pos)
    {
      unsigned int newPos;
      if (m_Steps != 0)
      {
        newPos = (pos > m_Steps - 1 ? m_Steps - 1 : pos);
      }
      else
      {
        newPos = 0;
      }

      if (this->m_Pos != newPos)
      {
        this->m_Pos = newPos;
        this->Modified();
      }
    }

    itkGetConstMacro(Steps, unsigned int);
    itkSetMacro(Steps, unsigned int);

    itkGetConstMacro(AutoRepeat, bool);
    itkSetMacro(AutoRepeat, bool);
    itkBooleanMacro(AutoRepeat);

    /** Causes the stepper to shift direction when the boundary is reached */
    itkSetMacro(PingPong, bool);
    itkGetConstMacro(PingPong, bool);
    itkBooleanMacro(PingPong);

    /** If set to true, the Next() decreases the stepper and Previous()
     * decreases it */
    itkSetMacro(InverseDirection, bool);
    itkGetConstMacro(InverseDirection, bool);
    itkBooleanMacro(InverseDirection);

    /** \brief Set the scalar range (min/max) corresponding to the step positions.
     * \param[in] min The minimum scalar value.
     * \param[in] max The maximum scalar value.
     */
    void SetRange(ScalarType min, ScalarType max);

    /** \brief Mark the current range as invalid while keeping the range itself. */
    void InvalidateRange();

    /** \brief Get the minimum value of the range. */
    ScalarType GetRangeMin() const;

    /** \brief Get the maximum value of the range. */
    ScalarType GetRangeMax() const;

    /** \brief Check whether the range has been set and is currently valid. */
    bool HasValidRange() const;

    /** \brief Remove the range entirely. */
    void RemoveRange();

    /** \brief Check whether a range has been set (may or may not be valid). */
    bool HasRange() const;

    /** \brief Set the unit name string (e.g. "mm") for the stepping range.
     * \param[in] unitName The unit name string.
     */
    void SetUnitName(const char *unitName);

    /** \brief Get the unit name string. */
    const char *GetUnitName() const;

    /** \brief Remove the unit name. */
    void RemoveUnitName();

    /** \brief Check whether a unit name has been set. */
    bool HasUnitName() const;

    /** \brief Advance to the next position in the list.
     *
     * Respects AutoRepeat, PingPong, and InverseDirection settings.
     */
    virtual void Next();

    /** \brief Go to the previous position in the list.
     *
     * Respects AutoRepeat, PingPong, and InverseDirection settings.
     */
    virtual void Previous();

    /** \brief Move the position by a given delta (positive or negative).
     *
     * Respects AutoRepeat wrapping. If the resulting position would be
     * negative, it is clamped to 0 (unless AutoRepeat is on).
     * \param[in] sliceDelta Number of steps to move (can be negative).
     */
    virtual void MoveSlice(int sliceDelta);

    /** \brief Go to the first position (index 0). */
    virtual void First();

    /** \brief Go to the last position (index GetSteps() - 1). */
    virtual void Last();

  protected:
    Stepper();
    ~Stepper() override;

    /** \brief Increment position by one, respecting AutoRepeat and PingPong. */
    void Increase();

    /** \brief Decrement position by one, respecting AutoRepeat and PingPong. */
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

#endif
