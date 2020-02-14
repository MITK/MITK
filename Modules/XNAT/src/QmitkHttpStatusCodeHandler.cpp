/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkHttpStatusCodeHandler.h"
#include "QMessageBox"
#include <mitkCommon.h>
#include <sstream>

QmitkHttpStatusCodeHandler::QmitkHttpStatusCodeHandler()
{
}

QmitkHttpStatusCodeHandler::~QmitkHttpStatusCodeHandler()
{
}

static void CreateMessageBox(int statusCode, std::string errorMessage)
{
  std::stringstream ss;
  // Qt uses different type of status code numbers.

  switch (statusCode)
  {
    case 1:
      QMessageBox::warning(nullptr,
                           "ConnectionRefusedError - 1",
                           "The remote server refused the connection (the server is not accepting requests).");
      break;
    case 2:
      QMessageBox::warning(
        nullptr,
        "RemoteHostClosedError - 2",
        "The remote server closed the connection prematurely, before the entire reply was received and processed.");
      break;
    case 3:
      QMessageBox::warning(nullptr, "HostNotFoundError - 3", "The remote host name was not found (invalid hostname).");
      break;
    case 4:
      QMessageBox::warning(nullptr, "TimeoutError - 4", "The connection to the remote server timed out.");
      break;
    case 5:
      QMessageBox::warning(nullptr,
                           "OperationCanceledError - 5",
                           "The operation was canceled via calls to abort() or close() before it was finished.");
      break;
    case 6:
      QMessageBox::warning(nullptr, "SslHandshakeFailedError - 6", "The SSL/TLS handshake failed and the encrypted "
                                                                   "channel could not be established. The sslErrors() "
                                                                   "signal should have been emitted.");
      break;
    case 7:
      QMessageBox::warning(nullptr,
                           "TemporaryNetworkFailureError - 7",
                           "The connection was broken due to disconnection from the network, however the system has "
                           "initiated roaming to another access point. The request should be resubmitted and will be "
                           "processed as soon as the connection is re-established.");
      break;
    case 8:
      QMessageBox::warning(
        nullptr,
        "NetworkSessionFailedError - 8",
        "The connection was broken due to disconnection from the network or failure to start the network.");
      break;
    case 9:
      QMessageBox::warning(nullptr,
                           "BackgroundRequestNotAllowedError - 9",
                           "The background request is not currently allowed due to platform policy.");
      break;
    case 10:
      QMessageBox::warning(nullptr, "TooManyRedirectsError - 10 ", "While following redirects, the maximum limit was "
                                                                   "reached. The limit is by default set to 50 or as "
                                                                   "set by QNetworkRequest::setMaxRedirectsAllowed().");
      break;
    case 11:
      QMessageBox::warning(nullptr, "InsecureRedirectError - 11 ", "While following redirects, the network access API "
                                                                   "detected a redirect from a encrypted protocol "
                                                                   "(https) to an unencrypted one (http).");
      break;
    case 101:
      QMessageBox::warning(
        nullptr,
        "ProxyConnectionRefusedError - 101",
        "The connection to the proxy server was refused (the proxy server is not accepting requests).");
      break;
    case 102:
      QMessageBox::warning(
        nullptr,
        "ProxyConnectionClosedError - 102",
        "The proxy server closed the connection prematurely, before the entire reply was received and processed.");
      break;
    case 103:
      QMessageBox::warning(
        nullptr, "ProxyNotFoundError - 103", "The proxy host name was not found (invalid proxy hostname).");
      break;
    case 104:
      QMessageBox::warning(
        nullptr,
        "ProxyTimeoutError - 104",
        "The connection to the proxy timed out or the proxy did not reply in time to the request sent.");
      break;
    case 105:
      QMessageBox::warning(nullptr, "ProxyAuthenticationRequiredError - 105", "The proxy requires authentication in "
                                                                              "order to honour the request but did not "
                                                                              "accept any credentials offered (if "
                                                                              "any).");
      break;
    case 201:
      QMessageBox::warning(nullptr,
                           "ContentAccessDenied - 201",
                           "The access to the remote content was denied (similar to HTTP error 401).");
      break;
    case 202:
      QMessageBox::warning(nullptr,
                           "ContentOperationNotPermittedError - 202",
                           "The operation requested on the remote content is not permitted.");
      break;
    case 203:
      QMessageBox::warning(nullptr,
                           "ContentNotFoundError - 203",
                           "The remote content was not found at the server (similar to HTTP error 404).");
      break;
    case 204:
      QMessageBox::warning(nullptr, "AuthenticationRequiredError - 204", "The remote server requires authentication to "
                                                                         "serve the content but the credentials "
                                                                         "provided were not accepted (if any).");
      break;
    case 205:
      QMessageBox::warning(nullptr, "ContentReSendError - 205", "The request needed to be sent again, but this failed "
                                                                "for example because the upload data could not be read "
                                                                "a second time.");
      break;
    case 206:
      QMessageBox::warning(
        nullptr,
        "ContentConflictError - 206",
        "The request could not be completed due to a conflict with the current state of the resource.");
      break;
    case 207:
      QMessageBox::warning(
        nullptr, "ContentGoneError - 207", "The requested resource is no longer available at the server.");
      break;
    case 401:
      QMessageBox::warning(
        nullptr,
        "InternalServerError - 401",
        "The server encountered an unexpected condition which prevented it from fulfilling the request.");
      break;
    case 402:
      QMessageBox::warning(nullptr,
                           "OperationNotImplementedError - 402",
                           "The server does not support the functionality required to fulfill the request.");
      break;
    case 403:
      QMessageBox::warning(
        nullptr, "ServiceUnavailableError - 403", "The server is unable to handle the request at this time.");
      break;
    case 301:
      QMessageBox::warning(nullptr,
                           "ProtocolUnknownError - 301",
                           "The Network Access API cannot honor the request because the protocol is not known.");
      break;
    case 302:
      QMessageBox::warning(
        nullptr, "ProtocolInvalidOperationError - 302", "The requested operation is invalid for this protocol.");
      break;
    case 99:
      QMessageBox::warning(nullptr, "UnknownNetworkError - 99", "An unknown network-related error was detected.");
      break;
    case 199:
      QMessageBox::warning(nullptr, "UnknownProxyError - 199", "An unknown proxy-related error was detected.");
      break;
    case 299:
      QMessageBox::warning(
        nullptr, "UnknownContentError - 299", "An unknown error related to the remote content was detected.");
      break;
    case 399:
      QMessageBox::warning(
        nullptr,
        "ProtocolFailure - 399",
        "A breakdown in protocol was detected (parsing error, invalid or unexpected responses, etc.).");
      break;
    case 499:
      QMessageBox::warning(
        nullptr, "UnknownServerError - 499", "An unknown error related to the server response was detected.");
      break;
    default:
      ss << "An Http Error occured with error code " << statusCode << " and server message: " << errorMessage;
      QMessageBox::warning(nullptr, "HTTP ERROR", ss.str().c_str());
      break;
  }
}

bool QmitkHttpStatusCodeHandler::HandleErrorMessage(const char *_errorMsg)
{
  static int lastCode = 0;
  std::string errorMsg(_errorMsg, strnlen(_errorMsg, strlen(_errorMsg)));
  bool success = true;
  /*
   * sample error response:
   * ERROR: An error occurred: ctkRuntimeException: Syncing with http request failed.
   * {d55ec279-8a65-46d6-80d3-cec079066109}: 202: Error downloading
   * https:... - server replied: Forbidden
   */

  if (errorMsg.find("request failed.") == std::string::npos)
    success = false;

  std::string::size_type indexOfErrorCode = errorMsg.find(": Error") - 3;
  std::string::size_type indexOfServerResponse =
    errorMsg.rfind("server replied: ") + 16; // Length of "server replied : " is 16

  if (indexOfErrorCode == std::string::npos || indexOfServerResponse == std::string::npos)
    success = false;

  std::string statusCodeString = errorMsg.substr(indexOfErrorCode, 3);
  std::stringstream str;
  str << statusCodeString;
  int statusCode;
  str >> statusCode;

  std::string serverResponse = errorMsg.substr(indexOfServerResponse);

  if (lastCode != statusCode)
    ::CreateMessageBox(statusCode, serverResponse);

  if (!success && lastCode != statusCode)
  {
    QMessageBox::warning(nullptr, "General Error", errorMsg.c_str());
  }

  if (lastCode != statusCode)
    lastCode = statusCode;
  else
    lastCode = 0;

  return success;
}
