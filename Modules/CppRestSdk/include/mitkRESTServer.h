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

#ifndef MITKRESTSERVER_H
#define MITKRESTSERVER_H

#include <QObject>

#include "cpprest/asyncrt_utils.h"
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_listener.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/uri.h"

#include "MitkCppRestSdkExports.h"

typedef web::http::experimental::listener::http_listener MitkListener;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTServer : public QObject
  {
  public:
    RESTServer();

    /**
     * @brief Creates an server listening to the given URL
     *
     * @param url the URL at which the server is listening for requests
     */
    RESTServer(utility::string_t url);
    virtual ~RESTServer();

    /**
     * @brief Opens the server connection and starts the listening process
	 * 
	 * @return a task to wait for
     */
    pplx::task<void> Open() { return m_Listener.open(); }

	/**
	 * @brief Closes the server connection and closes the listening process
	 *
	 * @return a task to wait for
	 */
    pplx::task<void> Close() { return m_Listener.close(); }

  protected:

	/**
	 * @brief Handle for incoming GET requests
	 *
	 * @param MitkRequest incoming request object
	 */
    virtual void HandleGet(MitkRequest){};

	/**
     * @brief Handle for incoming PUT requests
     *
     * @param MitkRequest incoming request object
     */
    virtual void HandlePut(MitkRequest){};

	/**
     * @brief Handle for incoming POST requests
     *
     * @param MitkRequest incoming request object
     */
    virtual void HandlePost(MitkRequest){};

	/**
     * @brief Handle for incoming DELETE requests
     *
     * @param MitkRequest incoming request object
     */
    virtual void HandleDelete(MitkRequest){};

    MitkListener m_Listener;
  };
};

#endif // MITKRESTSERVER_H
