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

#ifndef MITKUSNAVIGATIONLOGGINGBACKEND_H
#define MITKUSNAVIGATIONLOGGINGBACKEND_H

#include <fstream>
#include <vector>

#include "mbilogExports.h"
#include "mbilogTextBackendBase.h"
#include "mbilogLogMessage.h"
#include "mbilogLoggingTypes.h"

namespace mitk {
  /**
   *  \brief Backend for the mbi logging mechanism. This backend writes all messages to the given file.
   */
  class USNavigationLoggingBackend : public mbilog::TextBackendBase
  {
    public:

      USNavigationLoggingBackend();
      virtual ~USNavigationLoggingBackend();

      /** \brief Set file path und name for the output file.
       * The file will be opened and all log messages will be directed there from then on.
       *
       * \throws mitk::Exception if the file cannot be opened
       */
      void SetOutputFileName(std::string filename);

      /** \brief This method is called by the mbi logging mechanism if the object is registered in
       *         the mbi logging mechanism and a logging message is emitted. The method formats the
       *         logging messages to a normal string (depending on formatting mode) and writes it to std::cout.
       *
       *  \param logMessage Logging message.
       */
      virtual void ProcessMessage(const mbilog::LogMessage &logMessage );

      /** @return Returns all messages of the category "USNavigationLogging" since the last call of ClearNavigationMessages(). */
      std::vector<std::string> GetNavigationMessages();

      /** Writes a CSV file containing all stored navigation messages to the given output filename.
       */
      void WriteCSVFileWithNavigationMessages(std::string filename);

      /** Clears the internally stored navigation messages of the category "USNavigationLogging". */
      void ClearNavigationMessages();

      mbilog::OutputType GetOutputType(void) const;

    private:
      std::ofstream m_OutputStream;
      std::vector<std::string> m_lastNavigationMessages;
      std::vector<std::string> m_allNavigationMessages;
  };
} // namespace mitk


#endif // MITKUSNAVIGATIONLOGGINGBACKEND_H
