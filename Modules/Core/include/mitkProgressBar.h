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

  //##Documentation
  //## @brief Sending a message to the applications ProgressBar
  //##
  //## Holds a GUI dependent ProgressBarImplementation and sends the progress further.
  //## All mitk-classes use this class to display progress on GUI-ProgressBar.
  //## The mainapplication has to set the internal held ProgressBarImplementation with SetImplementationInstance(..).
  //## @ingroup Interaction
  class MITKCORE_EXPORT ProgressBar : public itk::Object
  {
  public:
    itkTypeMacro(ProgressBar, itk::Object);

    //##Documentation
    //## @brief static method to get the GUI dependent ProgressBar-instance
    //## so the methods for steps to do and progress can be called
    //## No reference counting, cause of decentral static use!
    static ProgressBar *GetInstance();

    //##Documentation
    //## @brief Supply a GUI- dependent ProgressBar. Has to be set by the application
    //## to connect the application dependent subclass of mitkProgressBar
    void RegisterImplementationInstance(ProgressBarImplementation *implementation);

    void UnregisterImplementationInstance(ProgressBarImplementation *implementation);

    //##Documentation
    //## @brief Adds steps to totalSteps.
    void AddStepsToDo(unsigned int steps);

    //##Documentation
    //## @brief Explicitly reset progress bar.
    void Reset();

    //##Documentation
    //## @brief Sets the current amount of progress to current progress + steps.
    //## @param steps the number of steps done since last Progress(int steps) call.
    void Progress(unsigned int steps = 1);

    //##Documentation
    //## @brief Sets whether the current progress value is displayed.
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
