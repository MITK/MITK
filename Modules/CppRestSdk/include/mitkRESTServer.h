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

// hm.. maybe go after that warning at some time? seems like a nasty hack, but works so far :)
#pragma warning(disable : 4251)

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
    RESTServer(utility::string_t url);
    virtual ~RESTServer();

    pplx::task<void> Open() { return m_Listener.open(); }
    pplx::task<void> Close() { return m_Listener.close(); }

    static std::string convertToUtf8(utility::string_t stringT) { return utility::conversions::to_utf8string(stringT); }

  protected:
    virtual void HandleGet(MitkRequest message){};
    virtual void HandlePut(MitkRequest message){};
    virtual void HandlePost(MitkRequest message){};
    virtual void HandleDelete(MitkRequest message){};
    void HandleError(pplx::task<void> &t);

    MitkListener m_Listener;
  };
};

#endif // MITKRESTSERVER_H
