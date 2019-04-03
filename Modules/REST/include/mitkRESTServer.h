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

#ifndef mitkRESTServer_h
#define mitkRESTServer_h

#include <MitkRESTExports.h>
#include <cpprest/uri.h>

namespace mitk
{
  class MITKREST_EXPORT RESTServer
  {
  public:
    /**
     * @brief Creates an server listening to the given URI
     *
     * @param uri the URI at which the server is listening for requests
     */
    RESTServer(const web::uri &uri);
    ~RESTServer();

    web::uri GetUri();

     /**
     * @brief Opens the listener and starts the listening process
     */
    void OpenListener();

    /**
     * @brief Closes the listener and stops the listening process
     */
    void CloseListener();
  private:
    /**
     * @brief Handle for incoming GET requests
     *
     * @param MitkRequest incoming request object
     */
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
