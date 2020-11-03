/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLESIONMANAGER_H
#define MITKLESIONMANAGER_H

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkLesionData.h"

/*
* @brief Provides helper functions that are needed to work with lesions.
*
*   These functions help to generate new lesions, check for existing lesions or provide functionality
*   to generate new and find existing lesion class types.
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
  * @param caseID         The current case identifier is defined by the given string.
  * @param lesionUID      The lesion UID as string.
  *
  * @return   The lesion with its UID and the lesion class.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::Lesion GetLesionByUID(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionUID);
  /**
  * @brief Find and return the whole lesion class including its UID given a specific lesion class type.
  *
  * @param caseID             The current case identifier is defined by the given string.
  * @param  lesionClassType   The lesion class type as string.
  *
  * @return   The lesion class with its UID and the class type.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionClass FindExistingLesionClass(const SemanticTypes::CaseID& caseID, const std::string& lesionClassType);
  /**
  * @brief Compute and store lesion presence for all available control points and information types.
  *
  * @param  lesionData          The lesion data that holds the lesion and will hold the additional lesion data.
  * @param  caseID              The current case ID.
  */
  MITKSEMANTICRELATIONS_EXPORT void ComputeLesionPresence(LesionData& lesionData, const SemanticTypes::CaseID& caseID);

} // namespace mitk

#endif // MITKLESIONMANAGER_H
