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

#ifndef MITKLESIONMANAGER_H
#define MITKLESIONMANAGER_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"
#include "mitkLesionData.h"

// mitk core
#include <mitkDataNode.h>

/*
* @brief Provides helper functions that are needed to work with lesions.
*
*   These functions help to generate new lesions, check for existing lesions or provide functionality
*   to find existing lesion class types.
*/
namespace mitk
{
  typedef std::vector<SemanticTypes::LesionClass> LesionClassVector;

  /**
  * @brief Generate a new lesion and lesion class with UIDs and the given string as lesion class type.
  *
  * @param  lesionClassType   The lesion class type as string. Default parameter is "".
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::Lesion GenerateNewLesion(const std::string& lesionClassType = "");
  /**
  * @brief Generate a new lesion class with UID and the given string as lesion class type.
  *
  * @param  lesionClassType   The lesion class type as string. Default parameter is "".
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionClass GenerateNewLesionClass(const std::string& lesionClassType = "");
  /**
  * @brief Find and return a whole lesion including its lesion class given a specific lesion UID.
  *
  * @param  lesionUID     The lesion UID as string.
  * @param  allLesions    All currently known lesions of a specific case.
  *
  * @return   The lesion with its UID and the lesion class.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::Lesion GetLesionByUID(const SemanticTypes::ID& lesionUID, const std::vector<SemanticTypes::Lesion>& allLesions);
  /**
  * @brief Find and return the whole lesion class including its UID given a specific lesion class type.
  *
  * @param  lesionClassType   The lesion class type as string.
  * @param  allLesionClasses  All currently known lesion classes of a specific case.
  *
  * @return   The lesion class with its UID and the class type.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionClass FindExistingLesionClass(const std::string& lesionClassType, const std::vector<SemanticTypes::LesionClass>& allLesionClasses);
  /**
  * @brief Generate and store additional lesion data such as lesion presence and lesion volume for each control point.
  *
  * @param  lesionData          The lesion data that holds the lesion and will hold the additional lesion data.
  * @param  caseID              The current case ID.
  */
  MITKSEMANTICRELATIONS_EXPORT void GenerateAdditionalLesionData(LesionData& lesionData, const SemanticTypes::CaseID& caseID);
} // namespace mitk

#endif // MITKLESIONMANAGER_H
