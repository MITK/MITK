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

#include "QmitkHttpStatusCodeHandler.h"
#include "QMessageBox"
#include <sstream>
#include <mitkLog.h>

QmitkHttpStatusCodeHandler::QmitkHttpStatusCodeHandler()
{
}

QmitkHttpStatusCodeHandler::~QmitkHttpStatusCodeHandler()
{
}

static void CreateMessageBox(int statusCode, std::string errorMessage)
{
  std::stringstream ss;
  //FIXME This seems to be a bug in the ctk framework. Rewise this as soon as possible!
  //The feedback status code seems to be off by 201.
  //This is a Qt bug - will be fixed after moving on 11.05.2016
  statusCode += 201;
  switch(statusCode)
  {
  case 202:
    QMessageBox::warning(nullptr, "202 - Accepted", "The request has been accepted, but not yet processed.");
    break;
  case 204:
    QMessageBox::warning(nullptr, "204 - No Content", "Successful request but no content could be returned.");
    break;
  case 301:
    QMessageBox::warning(nullptr, "301 - Moved Permanently", "All requests to this url cannot be processed anymore.");
    break;
  case 400:
    QMessageBox::warning(nullptr, "400 - Bad Request", "Server will not respond due to a client error..");
    break;
  case 401:
    QMessageBox::warning(nullptr, "401 - Unauthorized", "Please provide your authentication credentials.");
    break;
  case 403:
    QMessageBox::warning(nullptr, "403 - Forbidden", "You do not have the rights to perform this action.");
    break;
  case 404:
    QMessageBox::warning(nullptr, "404 - Not Found", "The requested resource could not be found.");
    break;
  case 500:
    QMessageBox::warning(nullptr, "500 - Internal Server Error", "An internal server error occured.");
    break;
  default:
    ss << "An Http Error occured with error code " << statusCode << " and server message: " << errorMessage;
    QMessageBox::warning(nullptr, "HTTP ERROR", ss.str().c_str());
    break;
  }
}

bool QmitkHttpStatusCodeHandler::HandleErrorMessage(const char *_errorMsg)
{
  std::string errorMsg(_errorMsg, strnlen(_errorMsg, strlen(_errorMsg)));
  bool success = true;
  /*
   * sample error response:
   * ERROR: An error occurred: ctkRuntimeException: Syncing with http request failed. {d55ec279-8a65-46d6-80d3-cec079066109}: 202: Error downloading
   * https:... - server replied: Forbidden
   */

  if(errorMsg.find("request failed.") == std::string::npos)
    success = false;

  std::string::size_type indexOfErrorCode = errorMsg.find(": Error") - 3;
  std::string::size_type indexOfServerResponse = errorMsg.rfind("server replied: ") + 16; //Length of "server replied : " is 16

  if(indexOfErrorCode == std::string::npos || indexOfServerResponse == std::string::npos)
    success =  false;

  std::string statusCodeString = errorMsg.substr(indexOfErrorCode,3);
  std::stringstream str;
  str << statusCodeString;
  int statusCode;
  str >> statusCode;

  std::string serverResponse = errorMsg.substr(indexOfServerResponse);

  ::CreateMessageBox(statusCode, serverResponse);

  if(!success)
  {
    QMessageBox::warning(nullptr, "General Error", errorMsg.c_str());
  }

  return success;
}
