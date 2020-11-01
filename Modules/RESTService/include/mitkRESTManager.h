/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRESTManager_h
#define mitkRESTManager_h

#include <MitkRESTServiceExports.h>
#include <mitkIRESTManager.h>
#include <mitkRESTUtil.h>

namespace mitk
{
  /**
   * @class RESTManager
   * @brief this is a microservice for managing REST-requests, used for non-qt applications.
   *
   * RESTManagerQt in the CppRestSdkQt module inherits from this class and is the equivalent microservice
   * used for Qt applications.
   */
  class MITKRESTSERVICE_EXPORT RESTManager : public IRESTManager
  {
  public:
    RESTManager();
    ~RESTManager() override;

    /**
     * @brief Executes a HTTP request in the mitkRESTClient class
     *
     * @throw mitk::Exception if RequestType is not suported
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param headers the headers for the request (optional)
     * @return task to wait for
     */
    pplx::task<web::json::value> SendRequest(
      const web::uri &uri,
      const RequestType &type = RequestType::Get,
      const std::map<utility::string_t, utility::string_t> headers = {}) override;

    /**
     * @brief Executes a HTTP request in the mitkRESTClient class
     *
     * @throw mitk::Exception if RequestType is not suported
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param body the body for the request (optional)
     * @param headers the headers for the request (optional)
     * @param filePath the file path to store the request to (optional)
     * @return task to wait for
     */
    pplx::task<web::json::value> SendJSONRequest(const web::uri &uri,
                                                 const RequestType &type = RequestType::Get,
                                                 const web::json::value *body = nullptr,
                                                 const std::map<utility::string_t, utility::string_t> headers = {},
                                                 const utility::string_t &filePath = {}) override;

    /**
     * @brief Executes a HTTP request in the mitkRESTClient class
     *
     * @throw mitk::Exception if RequestType is not suported
     * @param uri defines the URI the request is send to
     * @param type the RequestType of the HTTP request (optional)
     * @param body the body for the request (optional)
     * @param headers the headers for the request (optional)
     * @return task to wait for
     */
    pplx::task<web::json::value> SendBinaryRequest(
      const web::uri &uri,
      const RequestType &type = RequestType::Get,
      const std::vector<unsigned char> *body = {},
      const std::map<utility::string_t, utility::string_t> headers = {}) override;

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
     * @param method the http method of the request
     * @param headers the http headers of the request
     * @return the response
     */

    web::http::http_response Handle(const web::uri &uri,
                                    const web::json::value &body,
                                    const web::http::method &method,
                                    const mitk::RESTUtil::ParamMap &headers) override;

    /**
     * @brief Handles the deletion of an observer for all or a specific uri
     *
     * @param observer the observer which shouldn't receive requests anymore
     * @param uri the uri for which the observer doesn't handle requests anymore (optional)
     */
    void HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri = {}) override;

    /**
     * @brief internal use only
     */
    const std::map<int, RESTServer *> &GetServerMap() override;
    std::map<std::pair<int, utility::string_t>, IRESTObserver *> &GetObservers() override;

  private:
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
    void RequestForATakenPort(const web::uri &uri, IRESTObserver *observer);

    /**
     * @brief deletes an observer, called by HandleDeleteObserver method
     *
     * @param it the iterator comparing the observers in HandleDeleteObserver method
     * @return bool if there is another observer under the port
     */
    bool DeleteObserver(std::map<std::pair<int, utility::string_t>, IRESTObserver *>::iterator &it);

    void SetServerMap(const int port, RESTServer *server);
    void DeleteFromServerMap(const int port);
    void SetObservers(const std::pair<int, utility::string_t> key, IRESTObserver *observer);

    std::map<int, RESTServer *> m_ServerMap;                                  // Map with port server pairs
    std::map<std::pair<int, utility::string_t>, IRESTObserver *> m_Observers; // Map with all observers
  };
} // namespace mitk

#endif
