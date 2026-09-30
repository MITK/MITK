/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTestWarningCounter_h
#define mitkDICOMTestWarningCounter_h

#include <mitkLog.h>
#include <mitkLogBackendBase.h>

#include <string>

namespace mitk
{
  /**
   * \brief Counts, while it lives, the messages of one level (warnings unless
   *        another is passed) whose text contains a fragment.
   *
   * Filtering by a fragment such as a property key or a tag path keeps a case
   * from counting the unrelated messages a read also emits, without pinning the
   * rest of the wording.
   */
  class DICOMTestWarningCounter : public LogBackendBase
  {
  public:
    explicit DICOMTestWarningCounter(const std::string& fragment, LogLevel level = LogLevel::Warn)
      : m_Fragment(fragment), m_Level(level)
    {
      RegisterBackend(this);
    }

    ~DICOMTestWarningCounter() override
    {
      UnregisterBackend(this);
    }

    DICOMTestWarningCounter(const DICOMTestWarningCounter&) = delete;
    DICOMTestWarningCounter& operator=(const DICOMTestWarningCounter&) = delete;

    void ProcessMessage(const LogMessage& message) override
    {
      if (m_Level == message.Level && std::string::npos != message.Message.find(m_Fragment))
      {
        ++m_Count;
      }
    }

    OutputType GetOutputType() const override
    {
      return OutputType::Other;
    }

    unsigned int GetCount() const
    {
      return m_Count;
    }

  private:
    std::string m_Fragment;
    LogLevel m_Level;
    unsigned int m_Count = 0;
  };
}

#endif
