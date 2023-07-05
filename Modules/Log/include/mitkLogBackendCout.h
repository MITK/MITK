/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackendCout_h
#define mitkLogBackendCout_h

#include <mitkLogBackendText.h>

namespace mitk
{
  /** \brief Default backend of the MITK log mechanism.
   *
   * This backend is used if no other backend is registered. The backend formats the log messages to a normal string
   * and writes them to std::cout.
   */
  class MITKLOG_EXPORT LogBackendCout : public LogBackendText
  {
  public:
    LogBackendCout();
    ~LogBackendCout() override;

    void ProcessMessage(const LogMessage &message) override;

    /** \brief Sets the formatting mode.
     *
     * If true, long messages will be displayed. Default is false (short/smart messages). Long messages provide all
     * information and are capable to be postprocessed (e.g. in a web viewer).
     */
    void SetFull(bool full);

    OutputType GetOutputType() const override;

  private:
    bool m_UseFullOutput;
  };
}

#endif
