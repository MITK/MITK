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

#ifndef MITKDICOMWEB_H
#define MITKDICOMWEB_H

#include "MitkCppRestSdkExports.h"

#include "mitkRESTClient.h"

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT DICOMWeb : public RESTClient
  {
  public:
    /**
     * @brief Creates a client with the given base URL used to execute DICOMweb requests
     *
     * @param url the base URL for a PACS implementation like dcm4chee
     */
    DICOMWeb(utility::string_t url);
    virtual ~DICOMWeb();

    /**
     * @brief Executes a WADO-RS request for a single instance which should be stored to the given filePath.
     * The given studyUID, seriesUID, instanceUID defines the requested DICOM object.
     *
     * @param filePath the path at which the requested file should be stored
     * @param studyUID the studyUID of the requested DICOM object
     * @param seriesUID the seriesUID of the requested DICOM object
     * @param instanceUID the instanceUID of the requested DICOM object
     * @return a task to wait for
     */
    pplx::task<void> WadoRS(const utility::string_t filePath,
                            std::string studyUID,
                            std::string seriesUID,
                            std::string instanceUID);

    /**
     * @brief Executes a WADO-RS request for all instances of a DICOM series which should be stored to the given
     * folderPath. The given studyUID, seriesUID, defines the requested DICOM series.
     *
     * @param folderPath the path at which the requested series of files should be stored
     * @param studyUID the studyUID of the requested DICOM object
     * @param seriesUID the seriesUID of the requested DICOM object
     * @return task to wait for
     */
    pplx::task<std::string> WadoRS(const utility::string_t folderPath, std::string studyUID, std::string seriesUID);

    /**
     * @brief Executes a STOW-RS request with a DICOM instance given by the filePath which should be included in the
     * study given by the studyUID.	 *	 * @param filePath the path at which the file to be stored exists
     * @param studyUID the UID of the study at which the DICOM object should be stored
     * @return task to wait for
     */
    pplx::task<void> StowRS(utility::string_t filePath, std::string studyUID);

    /**
     * @brief Execute a QIDO-RS request with the given params on instance level
	 *
	 * @param params a map of specific QIDO parameters which filters the instance results
	 * @return task to wait for with a json response containing filtered instances by the given parameters
	 */
    pplx::task<web::json::value> QidoRSInstances(std::map<std::string, std::string> params);
  };
};

#endif // MITKDICOMWEB_H
