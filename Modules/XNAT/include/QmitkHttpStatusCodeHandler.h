/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkHttpStatusCodeHandler_h
#define QmitkHttpStatusCodeHandler_h

#include <MitkXNATExports.h>

#include <string>

/**
 * \brief Utility class for handling HTTP and network error status codes from XNAT requests.
 *
 * QmitkHttpStatusCodeHandler is a non-instantiable utility class that parses error messages
 * returned by CTK XNAT HTTP requests, extracts the status code, and displays an appropriate
 * warning message box to the user. It handles a wide range of Qt network error codes (connection
 * errors, proxy errors, content errors, protocol errors, and server errors) as well as standard
 * HTTP error codes.
 *
 * \note This class cannot be instantiated. All functionality is accessed through the static
 *       HandleErrorMessage method.
 *
 * \sa QmitkXnatTreeModel, ctkXnatSession
 */
class MITKXNAT_EXPORT QmitkHttpStatusCodeHandler final
{
public:
  /**
   * \brief Parse an error message from an XNAT HTTP request and display a user-facing warning dialog.
   *
   * Extracts the HTTP status code and server response from the error message string, then
   * displays an appropriate QMessageBox warning. Consecutive duplicate error codes are
   * suppressed to avoid flooding the user with repeated dialogs.
   *
   * The expected error message format is:
   * \code
   * ERROR: ... request failed. {uuid}: <code>: Error <url> - server replied: <message>
   * \endcode
   *
   * \param[in] _errorMsg The raw error message string from the CTK XNAT runtime exception.
   * \return \c true if the error message was successfully parsed as an HTTP request failure,
   *         \c false otherwise. A general error dialog is shown for unparseable messages.
   */
  static bool HandleErrorMessage(const char *_errorMsg);

private:
  QmitkHttpStatusCodeHandler();
  ~QmitkHttpStatusCodeHandler();
};

#endif
