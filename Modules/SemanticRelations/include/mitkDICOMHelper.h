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

#ifndef MITKDICOMHELPER_H
#define MITKDICOMHELPER_H

#include <MitkSemanticRelationsExports.h>

// semantic relation module
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDICOMTagPath.h>

// c++
#include <string>

namespace mitk
{
  namespace DICOMHelper
  {
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::CaseID GetCaseIDFromDataNode(const mitk::DataNode* dataNode);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ID GetIDFromDataNode(const mitk::DataNode* dataNode);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::Date GetDateFromDataNode(const mitk::DataNode* dataNode);

    MITKSEMANTICRELATIONS_EXPORT void ReformatDICOMTag(const std::string& tag, std::string& identifier);

    SemanticTypes::Date GetDateFromString(const std::string& dateAsString);

    std::string Trim(const std::string& identifier);
    std::string DICOMTagToName(const std::string& propertyName);
    std::string DICOMTagPathToName(const mitk::DICOMTagPath& tagPath);
    void ReformatDICOMDate(std::string& identifier);
    void ReformatDICOMTime(std::string& identifier);

  } // namespace DICOMHelper
} // namespace mitk

#endif // MITKDICOMHELPER_H
