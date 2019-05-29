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

#include <MitkSemanticRelationsExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"

namespace mitk
{
  namespace RelationStorage
  {
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::LesionVector GetAllLesionsOfCase(const SemanticTypes::CaseID& caseID);
    SemanticTypes::Lesion GetLesionOfSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);

    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPointVector GetAllControlPointsOfCase(const SemanticTypes::CaseID& caseID);
    SemanticTypes::ControlPoint GetControlPointOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);

    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ExaminationPeriodVector GetAllExaminationPeriodsOfCase(const SemanticTypes::CaseID& caseID);

    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::InformationTypeVector GetAllInformationTypesOfCase(const SemanticTypes::CaseID& caseID);
    SemanticTypes::InformationType GetInformationTypeOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);

    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllImageIDsOfCase(const SemanticTypes::CaseID& caseID);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllImageIDsOfControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllImageIDsOfInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllSegmentationIDsOfCase(const SemanticTypes::CaseID& caseID);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllSegmentationIDsOfImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    MITKSEMANTICRELATIONS_EXPORT SemanticTypes::IDVector GetAllSegmentationIDsOfLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);

    SemanticTypes::ID GetImageIDOfSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);

    MITKSEMANTICRELATIONS_EXPORT std::vector<SemanticTypes::CaseID> GetAllCaseIDs();
    MITKSEMANTICRELATIONS_EXPORT bool InstanceExists(const SemanticTypes::CaseID& caseID);

    void AddCase(const SemanticTypes::CaseID& caseID);
    void AddImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void RemoveImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void AddSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::ID& parentID);
    void RemoveSegmentation(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);

    void AddLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void OverwriteLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void LinkSegmentationToLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID, const SemanticTypes::Lesion& lesion);
    void UnlinkSegmentationFromLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& segmentationID);
    void RemoveLesion(const SemanticTypes::CaseID& caseID, const SemanticTypes::Lesion& lesion);
    void RemoveLesionClass(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& lesionClassID);

    void AddControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);
    void LinkImageToControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::ControlPoint& controlPoint);
    void UnlinkImageFromControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void RemoveControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint);

    void AddExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    void AddControlPointToExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    void RemoveControlPointFromExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriod& examinationPeriod);
    void RemoveExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ExaminationPeriod& examinationPeriod);

    void AddInformationTypeToImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID, const SemanticTypes::InformationType& informationType);
    void RemoveInformationTypeFromImage(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& imageID);
    void RemoveInformationType(const SemanticTypes::CaseID& caseID, const SemanticTypes::InformationType& informationType);

  } // namespace RelationStorage
} // namespace mitk

#endif // MITKRELATIONSTORAGE_H
