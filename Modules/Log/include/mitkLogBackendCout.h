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
  /** \brief Default console backend of the MITK log mechanism.
   *
   * Formats log messages as human-readable text and writes them to \c std::cout.
   * This backend is automatically instantiated and registered when no other backend
   * has been registered at the time a log message is emitted. It is automatically
   * unregistered when a user backend is added.
   *
   * Two formatting modes are supported:
   * - **Smart/short** (default): Compact format with relative timestamps and level indicators.
   * - **Full/long**: Verbose pipe-delimited format suitable for post-processing.
   *
   * \sa LogBackendText, RegisterBackend()
   */
  class MITKLOG_EXPORT LogBackendCout : public LogBackendText
  {
  public:
    /** \brief Construct a console backend with smart/short formatting mode (default). */
    LogBackendCout();

    ~LogBackendCout() override;

    /** \brief Format and write the log message to \c std::cout.
     *
     * Delegates to FormatFull() or FormatSmart() depending on the current formatting mode.
     *
     * \param[in] message The log message to process.
     */
    void ProcessMessage(const LogMessage &message) override;

    /** \brief Set the formatting mode.
     *
     * \param[in] full If \c true, use the full/long format which includes all metadata
     *                 (file path, line number, function name, thread ID, module name)
     *                 in a pipe-delimited layout suitable for post-processing.
     *                 If \c false (default), use the smart/short format with compact
     *                 timestamps and level indicators.
     */
    void SetFull(bool full);

    /** \brief Return the output type of this backend.
     *
     * \return Always returns OutputType::Console.
     */
    OutputType GetOutputType() const override;

  private:
    bool m_UseFullOutput;
  };
}

#endif
