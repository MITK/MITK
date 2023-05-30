/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLogBackend_h
#define mitkLogBackend_h

#include <mitkLogBackendText.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Log backend implementation for MITK.
   */
  class MITKCORE_EXPORT LogBackend : public LogBackendText
  {
  public:
    void ProcessMessage(const LogMessage&) override;

    /** \brief Registers MITK log backend.
     */
    static void Register();

    /** \brief Unregister MITK log backend.
     */
    static void Unregister();

    /** \brief Set extra log file path in addition to the console log.
     */
    static void SetLogFile(const std::string& file);

    /** \brief Activates and handles a rolling log file with the given prefix and path.
     *
     * This method handles 10 log files with a given prefix, e.g. "myLogFile".
     * The 10 log files will then look like this:
     * <ul>
     *   <li>myLogFile-0.log (current file)</li>
     *   <li>myLogFile-1.log (last file)</li>
     *   <li>myLogFile-2.log</li>
     *   <li><em>[...]</em></li>
     *   <li>myLogFile-9.log</li>
     * </ul>
     *
     * Everytime this method this called, normally when MITK is started, then
     * all log files are copied one file below (0 to 1, 1 to 2, and so on).
     * The oldest logfile (number 9) is always deleted. So users have access to
     * the log files of the last 10 runs.
     *
     * \throws mitk::Exception Throws an exception if there is a problem with renaming the logfiles, e.g., because of
     * file access problems.
     *
     * \param prefixPath Should hold the prefix of the logfile together with its path. E.g.,
     * "C:/programs/mitk/myLogFile".
     */
    static void RotateLogFiles(const std::string& prefixPath);

    /** \brief Increment the names of logfiles with the given prefixPath.
     *
     * This means, if the prefixPath is "myLogFile" myLogFile-0.log
     * is renamed to myLogFile-1.log, myLogFile-1.log to myLogFile-2.log,
     * and so on. The oldest logfile is deleted. The number of log files is
     * defined by the parameter "numLogFiles". The first logfile name is
     * "free" (e.g., [prefix]-0.log) again. This name is returned.
     *
     * \param prefixPath Should hold the prefix of the logfile together with its path. E.g.,
     * "C:/programs/mitk/myLogFile".
     * \param numLogFiles Sets the number of logfiles. Default value is 10. This means logfiles from [prefix]-0.log
     * to [prefix]-1.log are stored.
     *
     * \return Returns a new logfile name which is free again because the old first log file was renamed.
     *
     * \throws mitk::Exception Throws an exception if there is a problem with renaming the logfiles, e.g., because of
     * file access problems.
     */
    static std::string IncrementLogFileNames(const std::string& prefixPath, int numLogFiles = 10);

    /** \return Return the log file if there is one. Returns an empty string if no log file is active.
     */
    static std::string GetLogFile();

    /** \brief Enable an additional logging output window by means of itk::OutputWindow.
     *
     * This might be relevant for showing log output in applications with no default output console.
     */
    static void EnableAdditionalConsoleWindow(bool enable);

    /** \brief Automatically extract and remove the "--logfile <file>" parameters from the standard C main parameter
     * list and calls SetLogFile if needed.
     */
    static void CatchLogFileCommandLineParameter(int& argc, char** argv);

    LogBackendBase::OutputType GetOutputType() const override;

  protected:
    /** \brief Check if a file exists.
     *
     * \return Returns true if the file exists, false if not.
     */
    static bool CheckIfFileExists(const std::string &filename);
  };
}

#endif
