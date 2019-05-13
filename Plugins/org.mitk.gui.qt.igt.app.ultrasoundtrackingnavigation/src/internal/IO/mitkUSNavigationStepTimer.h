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

#ifndef MITKUSNAVIGATIONSTEPTIMER_H
#define MITKUSNAVIGATIONSTEPTIMER_H

#include <mitkCommon.h>

#include <itkObjectFactory.h>
#include <itkTimeProbe.h>

namespace mitk {

/**
 * \brief Timer for measureing the durations of navigation steps.
 * The different steps are identified by indices. To measure the duraion of a
 * step it can be activated by calling SetActiveIndex(). The duration is
 * measured until another step is activated by calling SetActiveIndex() with an
 * other index or Stop() is called.
 *
 * If a file name for an output file was set (SetOutputFileName()), the results
 * are written to this file, every time a new duration was measured. If a file
 * name is set later on, the results are written to the file at the moment of
 * setting the file name.
 */
class USNavigationStepTimer : public itk::Object {
public:
  mitkClassMacroItkParent(USNavigationStepTimer, itk::Object)
  itkNewMacro(USNavigationStepTimer)

  /**
   * \brief Stops the timer for the current index.
   * The measured time to if written to the output file if a file path was
   * already set.
   */
  void Stop();

  /**
   * \brief Removes all measured durations.
   */
  void Reset();

  /**
   * \brief Sets the name of the output file.
   * If there are already measurements of durations, they are written to this
   * file immediately.
   */
  void SetOutputFileName(std::string filename);

  /**
   * \brief Starts measurement for the step with the given index.
   */
  void SetActiveIndex(unsigned int index, std::string description = "");

  /**
   * \brief Returns the sum of all durations.
   */
  double GetTotalDuration();


  /**
   * \brief Returns measured duration for the step with the given index.
   * All measurements of this index are summed up.
   */
  double GetDurationForIndex(unsigned int index);

  /**
   * \brief Read durations from the file of the given file name.
   * This file must have exactly the same format as it is written by this class.
   */
  void ReadFromFile(std::string filename);

protected:
  USNavigationStepTimer();
  virtual ~USNavigationStepTimer();

  struct DurationForIndex {
    unsigned int index;
    double duration;
    std::string description;
    DurationForIndex(unsigned int index, double duration) : index(index), duration(duration) {}
    DurationForIndex(unsigned int index, double duration, std::string description) : index(index), duration(duration), description(description) {}
  };

  void FinishCurrentIndex();
  void WriteLineToFile(const DurationForIndex& element);
  void WriteEverythingToFile();

  typedef std::vector<DurationForIndex> DurationsVector;
  DurationsVector m_Durations;

  unsigned int m_CurrentIndex;
  std::string m_CurrentDescription;
  itk::TimeProbe m_TimeProbe;

  std::string m_OutputFileName;
};
} // namespace mitk

#endif // MITKUSNAVIGATIONSTEPTIMER_H
