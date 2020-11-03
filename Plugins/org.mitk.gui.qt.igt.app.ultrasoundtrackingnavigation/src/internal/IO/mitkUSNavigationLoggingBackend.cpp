/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSNavigationLoggingBackend.h"

#include <mitkCommon.h>

mitk::USNavigationLoggingBackend::USNavigationLoggingBackend()
{
}

mitk::USNavigationLoggingBackend::~USNavigationLoggingBackend()
{
  if ( m_OutputStream.is_open() ) { m_OutputStream.close(); }
}

void mitk::USNavigationLoggingBackend::SetOutputFileName(std::string filename)
{
  if ( m_OutputStream.is_open() ) { m_OutputStream.close(); }

  m_OutputStream.open(filename.c_str());
  if ( ! m_OutputStream.is_open() )
  {
    MITK_ERROR("USNavigationLoggingBackend")
      << "File '" << filename << "' cannot be opened for logging.";
    mitkThrow() << "File '" << filename << "' cannot be opened for logging.";
  }
}

void mitk::USNavigationLoggingBackend::ProcessMessage(const mbilog::LogMessage& logMessage)
{
  if ( m_OutputStream.is_open()) {this->FormatSmart(m_OutputStream, logMessage);}
  if (logMessage.category == "USNavigationLogging")
    {
      m_lastNavigationMessages.push_back(logMessage.message);
      m_allNavigationMessages.push_back(logMessage.message);
    }
}

std::vector<std::string> mitk::USNavigationLoggingBackend::GetNavigationMessages()
{
return m_lastNavigationMessages;
}

void mitk::USNavigationLoggingBackend::WriteCSVFileWithNavigationMessages(std::string filename)
{
  std::ofstream csvStream;
  csvStream.open(filename.c_str());
  if ( ! csvStream.is_open() ) {MITK_ERROR("USNavigationLoggingBackend") << "File '" << filename << "' cannot be opened for logging."; return;}
  for (std::size_t i = 0; i < m_allNavigationMessages.size(); i++)
    {
    csvStream << m_allNavigationMessages.at(i) << "\n";
    }
  csvStream.close();
}

void mitk::USNavigationLoggingBackend::ClearNavigationMessages()
{
m_lastNavigationMessages = std::vector<std::string>();
}

mbilog::OutputType mitk::USNavigationLoggingBackend::GetOutputType() const
{
  return mbilog::File;
}
