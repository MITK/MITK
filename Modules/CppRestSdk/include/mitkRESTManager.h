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

#ifndef mitkRESTManager_h
#define mitkRESTManager_h

#include <mitkIRESTManager.h>
#include <mitkRESTClientMicroService.h>
#include <mitkRESTServerMicroService.h>

#include <MitkCppRestSdkExports.h>

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTManager : public IRESTManager
  {

  public:
    RESTManager();
    ~RESTManager() override;

    /**
     * @brief Executes a HTTP request in the mitkRESTClientMicroService class
     *
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param body the body for the request (optional)
     * @param filePath the file path to store the request to
     * @return task to wait for
     */
    pplx::task<web::json::value> SendRequest(const web::uri &uri,
                                             const RequestType &type = get,
                                             const web::json::value &body= NULL,
                                             const utility::string_t &filePath = L"") override;
    
    /**
     * @brief starts listening for requests if there isn't another observer listening and the port is free
     *
     * @param uri defines the URI for which incoming requests should be send to the observer
     * @param observer the observer which handles the incoming requests
     */
    void ReceiveRequest(const web::uri &uri, IRESTObserver *observer) override;

    /**
     * @brief Handles incoming requests by notifying the observer which should receive it
     *
     * @param uri defines the URI of the request
     * @param body the body of the request
     * @return the data which is modified by the notified observer
     */
    web::json::value Handle(const web::uri &uri, web::json::value &body) override;

    /**
     * @brief Handles the deletion of an observer for all or a specific uri
     *
     * @param observer the observer which shouldn't receive requests anymore
     * @param uri the uri for which the observer doesn't handle requests anymore (optional)
     */
    virtual void HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri) override;

    virtual std::map<int, IRESTServerMicroService *> GetM_ServerMap() override;
    virtual std::map<std::pair<int, utility::string_t>, IRESTObserver *> GetM_Observers() override;

    /**
     * @brief adds an observer if a port is free, called by ReceiveRequest method
     *
     * @param uri the uri which builds the key for the observer map
     * @param observer the observer which is added
     */
    void AddObserver(const web::uri &uri, IRESTObserver *observer);

    /**
     * @brief handles server management if there is already a server under a port, called by ReceiveRequest method
     *
     * @param uri the uri which which is requested to be added
     * @param observer the observer which proceeds the request
     */
    void ServerUnderPort(const web::uri &uri, IRESTObserver *observer);

    /**
     * @brief deletes an observer, called by HandleDeleteObserver method
     *
     * @param it the iterator comparing the observers in HandleDeleteObserver method
     * @param the uri for which the observer doesn't want to receive requests anymore
     * @return bool if there is another observer under the port
     */
    bool DeleteObserver(std::map < std::pair<int, utility::string_t>, IRESTObserver *>::iterator &it, const web::uri &uri);

  protected:
    std::map<int, IRESTServerMicroService *> m_ServerMap;                     // Map with port server pairs
    std::map<std::pair<int, utility::string_t>, IRESTObserver *> m_Observers; // Map with all observers
  };
} // namespace mitk
#endif // !mitkRESTManager_h
