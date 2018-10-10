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
  public:;
    DICOMWeb(utility::string_t url);
    virtual ~DICOMWeb();

    pplx::task<void> WadoRS(const utility::string_t filePath, std::string studyUID, std::string seriesUID, std::string instanceUID);
    pplx::task<std::string> WadoRS(const utility::string_t folderPath, std::string studyUID, std::string seriesUID);
    pplx::task<void> StowRS(utility::string_t filePath, std::string studyUID);
    pplx::task<web::json::value> QuidoRSInstances(std::map<std::string, std::string> params);

  };
};

#endif // MITKDICOMWEB_H
