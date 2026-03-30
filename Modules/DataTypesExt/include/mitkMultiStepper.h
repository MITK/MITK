/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiStepper_h
#define mitkMultiStepper_h

#include <MitkDataTypesExtExports.h>
#include <mitkCommon.h>
#include <mitkStepper.h>
#include <mitkVector.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <set>

namespace mitk
{
  /**
   * \brief Coordinates multiple Stepper instances to step through in parallel.
   *
   * A MultiStepper aggregates several sub-steppers and synchronizes their
   * positions. When the MultiStepper's position is set, each sub-stepper is
   * moved proportionally according to its range and repeat factor. The total
   * step count is determined by the sub-stepper with the largest effective
   * range.
   *
   * \sa Stepper
   * \ingroup NavigationControl
   */
  class MITKDATATYPESEXT_EXPORT MultiStepper : public Stepper
  {
  public:
    mitkClassMacro(MultiStepper, Stepper);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)

    /**
     * \brief Add a sub-stepper with an optional repeat factor.
     *
     * The effective range of the stepper is its step count multiplied by
     * the repeat factor.
     *
     * \param[in] stepper The stepper to add.
     * \param[in] repeat The repeat (scale) factor (default 1).
     */
    void AddStepper(Stepper::Pointer stepper, unsigned int repeat = 1);

    /**
     * \brief Remove a sub-stepper.
     *
     * \param[in] stepper The stepper to remove.
     * \param[in] repeat Not used; kept for API compatibility.
     */
    void RemoveStepper(Stepper::Pointer stepper, unsigned int repeat = 1);

    /**
     * \brief Set the position and synchronize all sub-steppers.
     *
     * Each sub-stepper is moved proportionally to its range.
     *
     * \param[in] pos The new position.
     */
    void SetPos(unsigned int pos) override;

    /**
     * \brief Setting steps directly is not supported; triggers an assertion.
     *
     * The step count is determined automatically from the sub-steppers.
     *
     * \param[in] steps Ignored.
     */
    void SetSteps(const unsigned int steps) override;

  protected:
    MultiStepper();
    ~MultiStepper() override;
    typedef std::set<Stepper::Pointer> StepperSet;
    typedef std::map<Stepper::Pointer, unsigned int> ScaleFactorMap;
    StepperSet m_SubSteppers;
    ScaleFactorMap m_ScaleFactors;
    Stepper::Pointer m_LargestRangeStepper;
    void UpdateStepCount();
  };

} // namespace mitk

#endif
