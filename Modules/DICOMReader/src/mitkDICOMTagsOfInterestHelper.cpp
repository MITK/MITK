/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY
{
}
 without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDICOMTagsOfInterestHelper.h"

#include <mitkIDICOMTagsOfInterest.h>

#include "usModuleContext.h"
#include "usGetModuleContext.h"

mitk::DICOMTagPathMapType::value_type MakeLegacyEntry(const std::string& propName, const mitk::DICOMTag& tag)
{
  return std::make_pair(tag, propName);
}

mitk::DICOMTagPathMapType::value_type MakeEntry(const mitk::DICOMTagPath& tagPath)
{
  return std::make_pair(tagPath, "");
}

mitk::DICOMTagPathMapType
mitk::GetCurrentDICOMTagsOfInterest()
{
  mitk::DICOMTagPathMapType result;

  std::vector<us::ServiceReference<mitk::IDICOMTagsOfInterest> > toiRegisters = us::GetModuleContext()->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
  if (toiRegisters.empty())
  {
    // bad, no service found, cannot get tags of interest
    MITK_ERROR << "DICOM tag error: no service for DICOM tags of interest";
    return result;
  }
  else if (toiRegisters.size() > 1)
  {
    MITK_WARN << "DICOM tag error: multiple service for DICOM tags of interest found. Using just one.";
  }

  auto* toiRegister = us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(toiRegisters.front());
  if (!toiRegister)
  {
    MITK_ERROR << "Service lookup error, cannot get DICOM tag of interest service ";
  }

  return toiRegister->GetTagsOfInterest();
}

mitk::DICOMTagPathMapType
mitk::GetDefaultDICOMTagsOfInterest()
{
  DICOMTagPathMapType result;
    //These tags are copied from DICOMSeriesReader. The old naming style (deprecated)
    //is kept for backwards compatibility until it is removed.
    //Below we have also already added the properties with the new naming style

    // Patient module
    /*dicom.patient.PatientsName*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x0010)));
    /*dicom.patient.PatientID*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x0020)));
    /*dicom.patient.PatientsBirthDate*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x0030)));
    /*dicom.patient.PatientsSex*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x0040)));
    /*dicom.patient.PatientsBirthTime*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x0032)));
    /*dicom.patient.OtherPatientIDs*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x1000)));
    /*dicom.patient.OtherPatientNames*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x1001)));
    /*dicom.patient.EthnicGroup*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x2160)));
    /*dicom.patient.PatientComments*/ result.insert(MakeEntry(DICOMTag(0x0010, 0x4000)));
    /*dicom.patient.PatientIdentityRemoved*/ result.insert(MakeEntry(DICOMTag(0x0012, 0x0062)));
    /*dicom.patient.DeIdentificationMethod*/ result.insert(MakeEntry(DICOMTag(0x0012, 0x0063)));

    // General Study module
    /*dicom.study.StudyInstanceUID*/ result.insert(MakeEntry(DICOMTag(0x0020, 0x000d)));
    /*dicom.study.StudyDate*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0020)));
    /*dicom.study.StudyTime*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0030)));
    /*dicom.study.ReferringPhysiciansName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0090)));
    /*dicom.study.StudyID*/ result.insert(MakeEntry(DICOMTag(0x0020, 0x0010)));
    /*dicom.study.AccessionNumber*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0050)));
    /*dicom.study.StudyDescription*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1030)));
    /*dicom.study.PhysiciansOfRecord*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1048)));
    /*dicom.study.NameOfPhysicianReadingStudy*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1060)));

    // General Series module
    /*dicom.series.Modality*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0060)));
    /*dicom.series.SeriesInstanceUID*/ result.insert(MakeEntry(DICOMTag(0x0020, 0x000e)));
    /*dicom.series.SeriesNumber*/ result.insert(MakeEntry(DICOMTag(0x0020, 0x0011)));
    /*dicom.series.Laterality*/ result.insert(MakeEntry(DICOMTag(0x0020, 0x0060)));
    /*dicom.series.SeriesDate*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0021)));
    /*dicom.series.SeriesTime*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0031)));
    /*dicom.series.PerformingPhysiciansName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1050)));
    /*dicom.series.ProtocolName*/ result.insert(MakeEntry(DICOMTag(0x0018, 0x1030)));
    /*dicom.series.SeriesDescription*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x103e)));
    /*dicom.series.OperatorsName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1070)));
    /*dicom.series.BodyPartExamined*/ result.insert(MakeEntry(DICOMTag(0x0018, 0x0015)));
    /*dicom.series.PatientPosition*/ result.insert(MakeEntry(DICOMTag(0x0018, 0x5100)));
    /*dicom.series.SmallestPixelValueInSeries*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0108)));
    /*dicom.series.LargestPixelValueInSeries*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0109)));

    // VOI LUT module
    /*dicom.voilut.WindowCenter*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x1050)));
    /*dicom.voilut.WindowWidth*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x1051)));
    /*dicom.voilut.WindowCenterAndWidthExplanation*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x1055)));

    // Image Pixel module
    /*dicom.pixel.PhotometricInterpretation*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0004)));
    /*dicom.pixel.Rows*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0010)));
    /*dicom.pixel.Columns*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0011)));

    // Image Plane module
    /*dicom.PixelSpacing*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x0030)));
    /*dicom.ImagerPixelSpacing*/ result.insert(MakeEntry(DICOMTag(0x0018, 0x1164)));

    //additional for RT
    /*dicom.RescaleIntercept*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x1052)));
    /*dicom.RescaleSlope*/ result.insert(MakeEntry(DICOMTag(0x0028, 0x1053)));
    /*dicom.ManufacturerModelName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1090)));
    /*dicom.ManufacturerName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0070)));
    /*dicom.InstitutionName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x0080)));
    /*dicom.StationName*/ result.insert(MakeEntry(DICOMTag(0x0008, 0x1010)));
    /*dicom.DoseGridScaling*/ result.insert(MakeEntry(DICOMTag(0x3004, 0x000e)));

    //Additions for RTPLAN
    DICOMTagPath doseReferenceSequence;
    doseReferenceSequence.AddAnySelection(0x300A, 0x0010);
    DICOMTagPath fractionGroupSequence;
    fractionGroupSequence.AddAnySelection(0x300A, 0x0070);
    DICOMTagPath beamSequence;
    beamSequence.AddAnySelection(0x300A, 0x00B0);
    DICOMTagPath referencedStructureSetSequence;
    referencedStructureSetSequence.AddAnySelection(0x300C, 0x0060);
    result.insert(MakeEntry(DICOMTagPath(doseReferenceSequence).AddElement(0x300A, 0x0013))); //dicom.DoseReferenceSequence.DoseReferenceUID
    result.insert(MakeEntry(DICOMTagPath(doseReferenceSequence).AddElement(0x300A, 0x0016))); //dicom.DoseReferenceSequence.DoseReferenceDescription
    result.insert(MakeEntry(DICOMTagPath(doseReferenceSequence).AddElement(0x300A, 0x0026))); //dicom.DoseReferenceSequence.TargetPrescriptionDose
    result.insert(MakeEntry(DICOMTagPath(fractionGroupSequence).AddElement(0x300A, 0x0078))); //dicom.FractionGroupSequence.NumberOfFractionsPlanned
    result.insert(MakeEntry(DICOMTagPath(fractionGroupSequence).AddElement(0x300A, 0x0080))); //dicom.FractionGroupSequence.NumberOfBeams
    result.insert(MakeEntry(DICOMTagPath(beamSequence).AddElement(0x300A, 0x00C6)));          //dicom.BeamSequence.RadiationType
    result.insert(MakeEntry(DICOMTagPath(referencedStructureSetSequence).AddElement(0x0008, 0x1155))); //dicom.ReferencedStructureSetSequence.ReferencedSOPInstanceUID

    //Additions for RTSTRUCT
    DICOMTagPath structureSetROISequence;
    structureSetROISequence.AddAnySelection(0x3006, 0x0020);
    result.insert(MakeEntry(DICOMTagPath(structureSetROISequence).AddElement(0x3006, 0x0022))); //dicom.StructureSetROISequence.ROINumber
    result.insert(MakeEntry(DICOMTagPath(structureSetROISequence).AddElement(0x3006, 0x0026))); //dicom.StructureSetROISequence.ROIName
    result.insert(MakeEntry(DICOMTagPath(structureSetROISequence).AddElement(0x3006, 0x0024))); //dicom.StructureSetROISequence.ReferencedFrameOfReferenceUID

    //Additions for RTDOSE
    DICOMTagPath planReferenceSequence;
    planReferenceSequence.AddAnySelection(0x300C, 0x0002);
    result.insert(MakeEntry(DICOMTagPath(planReferenceSequence).AddElement(0x0008, 0x1155))); //dicom.PlanReferenceSequence.ReferencedSOPInstanceUID

    //Additions for PET
    DICOMTagPath radioPharmaRootTag;
    radioPharmaRootTag.AddAnySelection(0x0054, 0x0016);
    DICOMTagPath radioNuclideRootTag(radioPharmaRootTag);
    radioNuclideRootTag.AddAnySelection(0x0054, 0x0300);

    result.insert(MakeEntry(DICOMTagPath(radioPharmaRootTag).AddElement(0x0018, 0x0031))); //dicom.pet.Radiopharmaceutical
    result.insert(MakeEntry(DICOMTagPath(radioPharmaRootTag).AddElement(0x0018, 0x1072))); //dicom.pet.RadiopharmaceuticalStartTime
    result.insert(MakeEntry(DICOMTagPath(radioPharmaRootTag).AddElement(0x0018, 0x1074))); //dicom.pet.RadionuclideTotalDose
    result.insert(MakeEntry(DICOMTagPath(radioPharmaRootTag).AddElement(0x0018, 0x1075))); //dicom.pet.RadionuclideHalfLife
    result.insert(MakeEntry(DICOMTagPath(radioPharmaRootTag).AddElement(0x0018, 0x1076))); //dicom.pet.RadionuclidePositronFraction

    result.insert(MakeEntry(DICOMTagPath(radioNuclideRootTag).AddElement(0x0008, 0x0100))); //dicom.pet.Radionuclide.CodeValue
    result.insert(MakeEntry(DICOMTagPath(radioNuclideRootTag).AddElement(0x0008, 0x0102))); //dicom.pet.Radionuclide.CodingSchemeDesignator
    result.insert(MakeEntry(DICOMTagPath(radioNuclideRootTag).AddElement(0x0008, 0x0104))); //dicom.pet.Radionuclide.CodemManing

    result.insert(MakeEntry(DICOMTag(0x0054, 0x1001))); //dicom.pet.RadioactivityUnits
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1102))); //dicom.pet.DecayCorrection
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1321))); //dicom.pet.DecayFactor
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1300))); //dicom.pet.FrameReferenceTime

    result.insert(MakeEntry(DICOMTag(0x0010, 0x1030))); //dicom.patient.PatientWeight
    result.insert(MakeEntry(DICOMTag(0x0010, 0x1020))); //dicom.patient.PatientSize

    //Other interesting acquisition correlated information
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0022))); //dicom.acquisition date
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0032))); //dicom.acquisition time
    result.insert(MakeEntry(DICOMTag(0x0008, 0x002a))); //dicom.acquisition datetime
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0080))); //dicom.Modality
    result.insert(MakeEntry(DICOMTag(0x0018, 0x002a))); //dicom.Sequence Name
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0020))); //dicom.Scanning Sequence
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0021))); //dicom.Sequence Variant
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0080))); //dicom.TR
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0081))); //dicom.TE
    result.insert(MakeEntry(DICOMTag(0x0018, 0x1310))); //dicom.Acquisition Matrix
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0087))); //dicom.Magnetic Field Strength

    //SOP
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0018))); //SOP Instance UID
    result.insert(MakeEntry(DICOMTag(0x0020, 0x0013))); //Instance number
    result.insert(MakeEntry(DICOMTag(0x0020, 0x1041))); //Slice location

    return result;
};
