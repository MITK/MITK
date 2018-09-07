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

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/json.h"
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"
#include "cpprest/producerconsumerstream.h"

#include "MitkRESTServerExports.h"

typedef web::http::experimental::listener::http_listener MitkListener;
typedef web::http::http_request MitkRequest;
typedef web::http::methods MitkRESTMethods;
typedef web::http::status_codes MitkRestStatusCodes;

namespace mitk {

  class MITKRESTSERVER_EXPORT RESTServer
  {
  public:

    RESTServer();
    RESTServer(utility::string_t url);
    virtual ~RESTServer();

    pplx::task<void> open() { return m_Listener.open(); }
    pplx::task<void> close() { return m_Listener.close(); }

  protected:
    virtual void HandleGet(MitkRequest message) { };
    virtual void HandlePut(MitkRequest message) { };
    virtual void HandlePost(MitkRequest message) { };
    virtual void HandleDelete(MitkRequest message) { };
    void HandleError(pplx::task<void>& t);

    MitkListener m_Listener;
  };
};

#endif // MITKRESTSERVER_H