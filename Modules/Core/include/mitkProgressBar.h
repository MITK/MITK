/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProgressBar_h
#define mitkProgressBar_h
#include <MitkCoreExports.h>
#include <itkObject.h>

namespace mitk
{
  class ProgressBarImplementation;

  /** \brief Sending a message to the application's ProgressBar.
   *
   * Holds a GUI-dependent ProgressBarImplementation and forwards progress updates
   * to all registered implementations. All MITK classes use this class to display
   * progress on the GUI ProgressBar. The main application has to register a
   * ProgressBarImplementation with RegisterImplementationInstance().
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT ProgressBar : public itk::Object
  {
  public:
    itkTypeMacro(ProgressBar, itk::Object);

    /** \brief Static method to get the singleton ProgressBar instance.
     *
     * No reference counting, due to decentralized static use.
     *
     * \return Pointer to the singleton ProgressBar instance.
     */
    static ProgressBar *GetInstance();

    /** \brief Register a GUI-dependent ProgressBar implementation.
     *
     * Must be called by the application to connect the application-dependent
     * subclass of ProgressBarImplementation.
     *
     * \param implementation The ProgressBarImplementation to register.
     */
    void RegisterImplementationInstance(ProgressBarImplementation *implementation);

    /** \brief Unregister a previously registered ProgressBar implementation.
     *
     * \param implementation The ProgressBarImplementation to unregister.
     */
    void UnregisterImplementationInstance(ProgressBarImplementation *implementation);

    /** \brief Adds steps to the total number of steps to do.
     *
     * \param steps Number of steps to add.
     */
    void AddStepsToDo(unsigned int steps);

    /** \brief Explicitly reset the progress bar. */
    void Reset();

    /** \brief Sets the current amount of progress to current progress + steps.
     *
     * \param steps The number of steps done since the last Progress() call.
     */
    void Progress(unsigned int steps = 1);

    /** \brief Sets whether the current progress value is displayed as a percentage.
     *
     * \param visible If true, the percentage is displayed.
     */
    void SetPercentageVisible(bool visible);

  protected:
    typedef std::vector<ProgressBarImplementation *> ProgressBarImplementationsList;
    typedef ProgressBarImplementationsList::iterator ProgressBarImplementationsListIterator;

    ProgressBar();

    ~ProgressBar() override;

    ProgressBarImplementationsList m_Implementations;

    static ProgressBar *m_Instance;
  };

} // end namespace mitk

#endif
