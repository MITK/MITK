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

#ifndef mitkRESTManagerQt_h
#define mitkRESTManagerQt_h


#include <QObject>
#include <QThread>

#include <mitkIRESTManager.h>
#include <mitkRESTManager.h>
#include <mitkRESTClient.h>
#include <mitkRESTServerMicroServiceQt.h>

#include <MitkCppRestSdkQtExports.h>

namespace mitk
{
  class MITKCPPRESTSDKQT_EXPORT RESTManagerQt : public QObject, public RESTManager
  {
    Q_OBJECT

  public:
    RESTManagerQt();
    ~RESTManagerQt() override;

    /**
     * @brief starts listening for requests if there isn't another observer listening and the port is free
     *
     * @param uri defines the URI for which incoming requests should be send to the observer
     * @param observer the observer which handles the incoming requests
     */
    void ReceiveRequest(const web::uri &uri, IRESTObserver *observer) override;

    /**
     * @brief Handles the deletion of an observer for all or a specific uri
     *
     * @param observer the observer which shouldn't receive requests anymore
     * @param uri the uri for which the observer doesn't handle requests anymore (optional)
     */
    void HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri) override;

  private:
    std::map<int, QThread *> m_ServerThreadMap; // Map with threads for servers
  };
} // namespace mitk
#endif // !mitkRESTManager_h