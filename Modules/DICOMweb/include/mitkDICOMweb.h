/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMweb_h
#define mitkDICOMweb_h

#include <MitkDICOMwebExports.h>

#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"
#include <iostream>
#include <mitkCommon.h>
#include <mitkIRESTManager.h>
#include <mitkRESTUtil.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

/**
 * @brief This class represents the implementation of the RESTful DICOMweb standard
 * (https://dicom.nema.org/medical/dicom/current/output/html/part18.html). It is used to communicate DICOM data over HTTP
 * to a DICOMweb enabled PACS server.
 *
 * @author Tobias Stein
 */

namespace mitk
{

 class MITKDICOMWEB_EXPORT DICOMweb
 {
 public:
   typedef web::http::uri_builder MitkUriBuilder;
   typedef web::http::http_request MitkRequest;
   typedef web::http::http_response MitkResponse;
   typedef web::http::methods MitkRESTMethods;

   DICOMweb();

   /**
    * @brief Creates a DICOMweb service instance which allow basic DICOM operations on the given base URI.
    *
    * @param baseURI the uri for the PACS server: for example a dcm4chee
    * <code>https://<ip:port>/dcm4chee-arc/aets/DCM4CHEE/</code>
    */
   DICOMweb(utility::string_t baseURI);

   /**
    * @brief Sends a STOW request with the file in the given path to the study given bei its UID.
    *
    * @param filePath the path to a valid DICOM file which should be send
    * @param studyUID the DICOM study uid
    * @return the task to wait for
    */
   pplx::task<void> SendSTOW(utility::string_t filePath, utility::string_t studyUID);

   /**
    * @brief Sends a WADO request for an DICOM object instance matching the given uid parameters and stores it at the
    * given file path.
    *
    * @param filePath the path at which the retrieved DICOM object instance will be stored
    * @param studyUID the DICOM study uid
    * @param seriesUID the DICOM series uid
    * @param instanceUID the DICOM instance uid
    * @return the task to wait for, which unfolds no value when finished
    */
   pplx::task<void> SendWADO(utility::string_t filePath,
                             utility::string_t studyUID,
                             utility::string_t seriesUID,
                             utility::string_t instanceUID);

   /**
    * @brief Sends a WADO request for an DICOM object series matching the given uid parameters and stores all the
    * containing instances at the given folder path.
    *
    * @param folderPath the path at which the retrieved DICOM object instances of the retrieved series will be stored
    * @param studyUID the DICOM study uid
    * @param seriesUID the DICOM series uid
    * @return the task to wait for, which unfolds the name of the first DICOM object file within the folder path
    */
   pplx::task<std::string> SendWADO(utility::string_t folderPath,
                                    utility::string_t studyUID,
                                    utility::string_t seriesUID);

   /**
    * @brief Sends a QIDO request containing the given parameters to filter the query.
    *
    * Example Map:
    * <code>
    *  mitk::RESTUtil::ParamMap seriesInstancesParams;
    *  seriesInstancesParams.insert(mitk::RESTUtil::ParamMap::value_type(U("limit"), U("1")));
    * </code>
    *
    * @param map the map of parameters to filter the query
    * @return the task to wait for, which unfolds the result JSON response for the request when finished
    */
   pplx::task<web::json::value> SendQIDO(mitk::RESTUtil::ParamMap map);

 private:
   /**
    * @brief Creates a QIDO request URI with the given parameter map
    */
   utility::string_t CreateQIDOUri(mitk::RESTUtil::ParamMap map);

   /**
    * @brief Creates a WADO request URI with the given parameter
    */
   utility::string_t CreateWADOUri(utility::string_t studyUID,
                                   utility::string_t seriesUID,
                                   utility::string_t instanceUID);

   /**
    * @brief Creates a STOW request URI with the study uid
    */
   utility::string_t CreateSTOWUri(utility::string_t studyUID);

   /**
    * @brief Initializes the rest manager for this service instance. Should be called in constructor to make sure the
    * public API can work properly.
    */
   void InitializeRESTManager();

   utility::string_t m_BaseURI;
   mitk::IRESTManager *m_RESTManager;
 };
}

#endif
