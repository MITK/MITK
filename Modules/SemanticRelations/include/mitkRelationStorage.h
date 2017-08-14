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

#ifndef MITKRELATIONSTORAGE_H
#define MITKRELATIONSTORAGE_H

// semantic relations module
#include "mitkDICOMHelper.h"
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

namespace mitk
{
  class RelationStorage
  {
  public:

    void SetDataStorage(DataStorage::Pointer dataStorage);

    std::vector<SemanticTypes::Lesion> GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID);
    SemanticTypes::Lesion GetRepresentedLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);
    std::vector<DataNode::Pointer> GetAllSegmentationsOfCase(const SemanticTypes::CaseID& caseID);

    SemanticTypes::ControlPoint GetControlPointOfData(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID);
    std::vector<SemanticTypes::ControlPoint> GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID);

    SemanticTypes::InformationType GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    std::vector<SemanticTypes::InformationType> GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID);

    std::vector<mitk::DataNode::Pointer> GetAllImagesOfCase(const SemanticTypes::CaseID& caseID);
    std::vector<std::string> GetAllImageIDsOfCase(const SemanticTypes::CaseID& caseID);
    std::vector<SemanticTypes::CaseID> GetAllCaseIDs();

    void AddCase(const SemanticTypes::CaseID& caseID);
    void AddImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID);
    void AddSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID, const SemanticTypes::ID& parentDataNodeID);
    void RemoveSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationNodeID);

    void AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void LinkSegmentationToLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::Lesion& lesion);
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);

    void AddControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    void OverwriteControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    void LinkDataToControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID, const SemanticTypes::ControlPoint& controlPoint);
    void UnlinkDataFromControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID);
    void RemoveControlPointFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);

    void AddInformationTypeToImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::InformationType informationType);
    void RemoveInformationTypeFromImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void RemoveInformationTypeFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType informationType);

  private:

    // access the storage and retrieve the case data, stored under the given case ID
    mitk::PropertyList::Pointer GetStorageData(const SemanticTypes::CaseID& caseID);

    DataStorage::Pointer m_DataStorage;

  };

} // namespace mitk

#endif // MITKRELATIONSTORAGE_H
