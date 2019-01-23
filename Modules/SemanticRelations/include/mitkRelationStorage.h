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
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>

namespace mitk
{
  namespace RelationStorage
  {
    SemanticTypes::LesionVector GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID);
    SemanticTypes::Lesion GetRepresentedLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);

    std::vector<std::string> GetAllSegmentationIDsOfCase(const SemanticTypes::CaseID& caseID);

    SemanticTypes::ControlPoint GetControlPointOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    SemanticTypes::ControlPointVector GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID);

    SemanticTypes::ExaminationPeriodVector GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID);

    SemanticTypes::InformationType GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    SemanticTypes::InformationTypeVector GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID);

    std::vector<std::string> GetAllImageIDsOfCase(const SemanticTypes::CaseID& caseID);
    std::vector<SemanticTypes::CaseID> GetAllCaseIDs();

    void AddCase(const SemanticTypes::CaseID& caseID);
    void AddImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageNodeID);
    void RemoveImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageNodeID);
    void AddSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationNodeID, const SemanticTypes::ID& parentDataNodeID);
    void RemoveSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationNodeID);

    void AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void LinkSegmentationToLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::Lesion& lesion);
    void UnlinkSegmentationFromLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void RemoveLesionClass(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionClassID);

    void AddControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    void LinkDataToControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID, const SemanticTypes::ControlPoint& controlPoint);
    void UnlinkDataFromControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& dataNodeID);
    void RemoveControlPointFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);

    void AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    void AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod examinationPeriod);
    void RemoveControlPointFromExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod examinationPeriod);
    void RemoveExaminationPeriodFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod examinationPeriod);

    void AddInformationTypeToImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::InformationType informationType);
    void RemoveInformationTypeFromImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void RemoveInformationTypeFromCase(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType informationType);

  } // namespace RelationStorage
} // namespace mitk

#endif // MITKRELATIONSTORAGE_H
