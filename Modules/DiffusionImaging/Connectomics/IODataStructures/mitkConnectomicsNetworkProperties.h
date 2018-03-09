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


#ifndef ConnectomicsNetworkProperties_H_HEADER_INCLUDED
#define ConnectomicsNetworkProperties_H_HEADER_INCLUDED

#include <cstring>
#include "mitkProperties.h"
#include "mitkStringProperty.h"

namespace mitk {

  /** \file mitkConnectomicsNetworkProperties.h
   * \brief This file defines the data properties for connectomics networks in MITK.
   *
   * This file collects and explains the properties which can be used store additional
   * information in connectomics networks.
   */

  /**
   * \brief Additional header information
   *
   * This property contains a string with additional information which can be stored
   * as a header in file formats which support it.
   */
  const std::string connectomicsDataAdditionalHeaderInformation = "Connectomics.Data.AdditionalHeaderInformation";
} // namespace mitk

#endif //ConnectomicsNetworkProperties_H_HEADER_INCLUDED
