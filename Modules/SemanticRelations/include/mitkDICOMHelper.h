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

// semantic relations module
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDICOMTagPath.h>

// c++
#include <string>

/*
* @brief Provides helper functions to convert DICOM Tag information.
*
*   In order to identify the patient of an image or segmentation or to set the control point of DICOM data,
*   these functions are used to retrieve the DICOM tags from the given data nodes and convert them into semantic types
*   that can be used by the SemanticRelations class.
*/
namespace mitk
{
  /**
  * @brief Creates a property name for a DICOM tag.
  *        The tag is "0x0010, 0x0010" (PatientName)
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetCaseIDDICOMProperty();
  /**
  * @brief Creates a property name for a DICOM tag.
  *        The tag is "0x0020, 0x000e" (SeriesInstanceUID)
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetNodeIDDICOMProperty();
  /**
  * @brief Creates a property name for a DICOM tag.
  *        The tag is "0x0008, 0x0022" (AcquisitionDate)
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetDateDICOMProperty();
  /**
  * @brief Creates a property name for a DICOM tag.
  *        The tag is "0x0008, 0x0060" (Modality)
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetModalityDICOMProperty();
  /*
  * @brief  Extracts a specific DICOM tag from the node's base data
  *         and returns the tag as a string. This tag string is used as an identifier for the patient (case).
  *
  * @pre    The given data node or the node's base data has to be valid (!nullptr).
  * @pre    The node's base data has to have the specific DICOM Tag property set.
  * @throw  mitk::Exception if the given data node, the node's base data or the extracted DICOM tag are invalid (==nullptr).
  *
  * @par dataNode   The data node, of which the DICOM tag should be extracted.
  *
  * @return         The extracted DICOM tag as string.
  *                 An empty string, if the DICOM tag can not be extracted (i.e. the data node or
  *                 the underlying base data is invalid or the DICOM tag does not exist for the given data node).
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::CaseID GetCaseIDFromDataNode(const mitk::DataNode* dataNode);
  /*
  * @brief Extracts a specific DICOM tag (currently "0x0020, 0x000e": SeriesInstanceUID) from the node's base data
  *        and returns the tag as a string. This tag string is used as an identifier for the image instance.
  *
  * @pre    The given data node or the node's base data has to be valid (!nullptr).
  * @pre    The node's base data has to have the "0x0020, 0x000e" DICOM Tag property set.
  * @throw  mitk::Exception if the given data node, the node's base data or the extracted DICOM tag are invalid (==nullptr).
  *
  * @par dataNode   The data node, of which the DICOM tag should be extracted.
  *
  * @return         The extracted DICOM tag as string.
  *                 An empty string, if the DICOM tag can not be extracted (i.e. the data node or
  *                 the underlying base data is invalid or the DICOM tag does not exist for the given data node).
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ID GetIDFromDataNode(const mitk::DataNode* dataNode);
  /*
  * @brief Extracts a specific DICOM tag (currently "0x0008, 0x0022": AcquisitionDate) from the node's base data
  *        and returns the tag as a control point.
  *
  * @pre    The given data node or the node's base data has to be valid (!nullptr).
  * @pre    The node's base data has to have the "0x0008, 0x0022" DICOM Tag property set.
  * @throw  mitk::Exception if the given data node, the node's base data or the extracted DICOM tag are invalid (==nullptr).
  *
  * @par dataNode   The data node, of which the DICOM tag should be extracted.
  *
  * @return         The extracted DICOM tag as control point.
  *                 An empty control point, if the DICOM tag can not be extracted (i.e. the data node or
  *                 the underlying base data is invalid or the DICOM tag does not exist for the given data node).
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GetDICOMDateFromDataNode(const mitk::DataNode* dataNode);
  /**
  * @brief Extracts a specific DICOM tag from the node's base data and returns the tag as a information type (a string).
  *
  * @pre    The given data node or the node's base data has to be valid (!nullptr).
  * @pre    The node's base data has to have the "0x0008, 0x0060" DICOM Tag property set.
  * @throw  mitk::Exception if the given data node, the node's base data or the extracted DICOM tag are invalid (==nullptr).
  *
  * @par dataNode   The data node, of which the DICOM tag should be extracted.
  *
  * @return         The extracted DICOM tag as information type (a string).
  *                 An empty information type, if the DICOM tag can not be extracted (i.e. the data node or
  *                 the underlying base data is invalid or the DICOM tag does not exist for the given data node).
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::InformationType GetDICOMModalityFromDataNode(const mitk::DataNode* dataNode);
  /*
  * @brief Removes leading and trailing whitespace from the given string.
  *
  * @par identifier   The value of a DICOM tag.
  *
  * @return           The trimmed DICOM tag
  */
  MITKSEMANTICRELATIONS_EXPORT std::string TrimDICOM(const std::string& identifier);
} // namespace mitk

#endif // MITKDICOMHELPER_H
