/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProgressBarImplementation_h
#define mitkProgressBarImplementation_h
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief GUI-independent interface for all GUI-dependent implementations of a progress bar.
   *
   * Subclasses implement the actual rendering of the progress bar using the platform's
   * GUI toolkit (e.g., Qt).
   *
   * \sa mitk::ProgressBar
   */
  class MITKCORE_EXPORT ProgressBarImplementation
  {
  public:
    /** \brief Constructor. */
    ProgressBarImplementation(){};

    /** \brief Destructor. */
    virtual ~ProgressBarImplementation(){};

    /** \brief Sets whether the current progress percentage is displayed.
     *
     * \param visible if true, the percentage text is shown.
     */
    virtual void SetPercentageVisible(bool visible) = 0;

    /** \brief Explicitly resets the progress bar to zero. */
    virtual void Reset() = 0;

    /** \brief Adds steps to the total number of steps to complete.
     *
     * \param steps the number of steps to add to the total.
     */
    virtual void AddStepsToDo(unsigned int steps) = 0;

    /** \brief Advances the current progress by the given number of steps.
     *
     * \param steps the number of steps done since the last Progress() call.
     */
    virtual void Progress(unsigned int steps) = 0;
  };

} // end namespace mitk

#endif
