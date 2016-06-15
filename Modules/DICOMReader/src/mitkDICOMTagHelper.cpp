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

#include "mitkDICOMTagHelper.h"

#include <mitkIDICOMTagsOfInterest.h>

#include "usModuleContext.h"
#include "usGetModuleContext.h"

mitk::DefaultDICOMTagMapType::value_type MakeEntry(const mitk::DICOMTag& tag)
{
    return std::make_pair(GeneratPropertyNameForDICOMTag(tag).c_str(), tag);
}

mitk::DICOMTagMapType
mitk::GetCurrentDICOMTagsOfInterest()
{
  mitk::DICOMTagMapType result;

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

  IDICOMTagsOfInterest* toiRegister = us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(toiRegisters.front());
  if (!toiRegister)
  {
    MITK_ERROR << "Service lookup error, cannot get DICOM tag of interest service ";
  }

  return toiRegister->GetTagsOfInterest();
}

mitk::DefaultDICOMTagMapType
mitk::GetDefaultDICOMTagsOfInterest()
{
    DefaultDICOMTagMapType result;
    //These tags are copied from DICOMSeriesReader. The old naming style (deprecated)
    //is keept for backwards compatibility until it is removed.
    //Below we have also already added the properties with the new naming style
    // Patient module
    result.insert(std::make_pair("dicom.patient.PatientsName", DICOMTag(0x0010, 0x0010)));
    result.insert(std::make_pair("dicom.patient.PatientID", DICOMTag(0x0010, 0x0020)));
    result.insert(std::make_pair("dicom.patient.PatientsBirthDate", DICOMTag(0x0010, 0x0030)));
    result.insert(std::make_pair("dicom.patient.PatientsSex", DICOMTag(0x0010, 0x0040)));
    result.insert(std::make_pair("dicom.patient.PatientsBirthTime", DICOMTag(0x0010, 0x0032)));
    result.insert(std::make_pair("dicom.patient.OtherPatientIDs", DICOMTag(0x0010, 0x1000)));
    result.insert(std::make_pair("dicom.patient.OtherPatientNames", DICOMTag(0x0010, 0x1001)));
    result.insert(std::make_pair("dicom.patient.EthnicGroup", DICOMTag(0x0010, 0x2160)));
    result.insert(std::make_pair("dicom.patient.PatientComments", DICOMTag(0x0010, 0x4000)));
    result.insert(std::make_pair("dicom.patient.PatientIdentityRemoved", DICOMTag(0x0012, 0x0062)));
    result.insert(std::make_pair("dicom.patient.DeIdentificationMethod", DICOMTag(0x0012, 0x0063)));

    // General Study module
    result.insert(std::make_pair("dicom.study.StudyInstanceUID", DICOMTag(0x0020, 0x000d)));
    result.insert(std::make_pair("dicom.study.StudyDate", DICOMTag(0x0008, 0x0020)));
    result.insert(std::make_pair("dicom.study.StudyTime", DICOMTag(0x0008, 0x0030)));
    result.insert(std::make_pair("dicom.study.ReferringPhysiciansName", DICOMTag(0x0008, 0x0090)));
    result.insert(std::make_pair("dicom.study.StudyID", DICOMTag(0x0020, 0x0010)));
    result.insert(std::make_pair("dicom.study.AccessionNumber", DICOMTag(0x0008, 0x0050)));
    result.insert(std::make_pair("dicom.study.StudyDescription", DICOMTag(0x0008, 0x1030)));
    result.insert(std::make_pair("dicom.study.PhysiciansOfRecord", DICOMTag(0x0008, 0x1048)));
    result.insert(std::make_pair("dicom.study.NameOfPhysicianReadingStudy", DICOMTag(0x0008, 0x1060)));

    // General Series module
    result.insert(std::make_pair("dicom.series.Modality", DICOMTag(0x0008, 0x0060)));
    result.insert(std::make_pair("dicom.series.SeriesInstanceUID", DICOMTag(0x0020, 0x000e)));
    result.insert(std::make_pair("dicom.series.SeriesNumber", DICOMTag(0x0020, 0x0011)));
    result.insert(std::make_pair("dicom.series.Laterality", DICOMTag(0x0020, 0x0060)));
    result.insert(std::make_pair("dicom.series.SeriesDate", DICOMTag(0x0008, 0x0021)));
    result.insert(std::make_pair("dicom.series.SeriesTime", DICOMTag(0x0008, 0x0031)));
    result.insert(std::make_pair("dicom.series.PerformingPhysiciansName", DICOMTag(0x0008, 0x1050)));
    result.insert(std::make_pair("dicom.series.ProtocolName", DICOMTag(0x0018, 0x1030)));
    result.insert(std::make_pair("dicom.series.SeriesDescription", DICOMTag(0x0008, 0x103e)));
    result.insert(std::make_pair("dicom.series.OperatorsName", DICOMTag(0x0008, 0x1070)));
    result.insert(std::make_pair("dicom.series.BodyPartExamined", DICOMTag(0x0018, 0x0015)));
    result.insert(std::make_pair("dicom.series.PatientPosition", DICOMTag(0x0018, 0x5100)));
    result.insert(std::make_pair("dicom.series.SmallestPixelValueInSeries", DICOMTag(0x0028, 0x0108)));
    result.insert(std::make_pair("dicom.series.LargestPixelValueInSeries", DICOMTag(0x0028, 0x0109)));

    // VOI LUT module
    result.insert(std::make_pair("dicom.voilut.WindowCenter", DICOMTag(0x0028, 0x1050)));
    result.insert(std::make_pair("dicom.voilut.WindowWidth", DICOMTag(0x0028, 0x1051)));
    result.insert(std::make_pair("dicom.voilut.WindowCenterAndWidthExplanation", DICOMTag(0x0028, 0x1055)));

    // Image Pixel module
    result.insert(std::make_pair("dicom.pixel.PhotometricInterpretation", DICOMTag(0x0028, 0x0004)));
    result.insert(std::make_pair("dicom.pixel.Rows", DICOMTag(0x0028, 0x0010)));
    result.insert(std::make_pair("dicom.pixel.Columns", DICOMTag(0x0028, 0x0011)));

    // Image Plane module
    result.insert(std::make_pair("dicom.PixelSpacing", DICOMTag(0x0028, 0x0030)));
    result.insert(std::make_pair("dicom.ImagerPixelSpacing", DICOMTag(0x0018, 0x1164)));

    //additional for RT
    result.insert(std::make_pair("dicom.RescaleIntercept", DICOMTag(0x0028, 0x1052)));
    result.insert(std::make_pair("dicom.RescaleSlope", DICOMTag(0x0028, 0x1053)));
    result.insert(std::make_pair("dicom.ManufacturerModelName", DICOMTag(0x0008, 0x1090)));
    result.insert(std::make_pair("dicom.ManufacturerName", DICOMTag(0x0008, 0x0070)));
    result.insert(std::make_pair("dicom.InstitutionName", DICOMTag(0x0008, 0x0080)));
    result.insert(std::make_pair("dicom.StationName", DICOMTag(0x0008, 0x1010)));
    result.insert(std::make_pair("dicom.DoseGridScaling", DICOMTag(0x3004, 0x000e)));


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

    //Additions for PET
    result.insert(MakeEntry(DICOMTag(0x0018, 0x0031))); //dicom.pet.Tracer
    result.insert(MakeEntry(DICOMTag(0x0018, 0x1072))); //dicom.pet.RadiopharmaceuticalStartTime
    result.insert(MakeEntry(DICOMTag(0x0018, 0x1074))); //dicom.pet.RadionuclideTotalDose
    result.insert(MakeEntry(DICOMTag(0x0018, 0x1075))); //dicom.pet.RadionuclideHalfLife
    result.insert(MakeEntry(DICOMTag(0x0010, 0x1030))); //dicom.patient.PatientWeight
    result.insert(MakeEntry(DICOMTag(0x0010, 0x1020))); //dicom.patient.PatientSize
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1001))); //dicom.pet.RadioactivityUnits
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1102))); //dicom.pet.DecayCorrection
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1321))); //dicom.pet.DecayFactor
    result.insert(MakeEntry(DICOMTag(0x0054, 0x1300))); //dicom.pet.FrameReferenceTime

    //Other interesting acquisition correlated information
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0022))); //dicom.acuisition date
    result.insert(MakeEntry(DICOMTag(0x0008, 0x0032))); //dicom.acuisition time
    result.insert(MakeEntry(DICOMTag(0x0008, 0x002a))); //dicom.acuisition datetime
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

std::string
mitk::GeneratPropertyNameForDICOMTag(const mitk::DICOMTag& tag)
{
    std::ostringstream nameStream;
    nameStream << "DICOM." << std::setw(4) << std::setfill('0') << std::hex << tag.GetGroup() << "." << std::setw(4) << std::setfill('0') << std::hex << tag.GetElement();

    return nameStream.str();
};
