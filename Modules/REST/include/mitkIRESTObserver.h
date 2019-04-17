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

#ifndef mitkIRESTObserver_h
#define mitkIRESTObserver_h

#include <MitkRESTExports.h>

#include <cpprest/json.h>
#include <cpprest/uri.h>

namespace mitk
{
  class MITKREST_EXPORT IRESTObserver
  {
  public:
    /**
     * @brief Deletes an observer and calls HandleDeleteObserver() in RESTManager class
     *
     * @see HandleDeleteObserver()
     */
    virtual ~IRESTObserver();

    /**
     * @brief Called if there's an incoming request for the observer, observer implements how to handle request
     *
     * @param data the data of the incoming request
     * @return the modified data
     */
    virtual web::json::value Notify(const web::uri &uri, const web::json::value &data) = 0;


  private:
  };
}

#endif
