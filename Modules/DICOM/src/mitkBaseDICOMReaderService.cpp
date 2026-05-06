/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseDICOMReaderService.h>

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkDICOMFileReaderSelector.h>
#include <mitkImage.h>
#include <mitkDICOMFilesHelper.h>
#include <mitkDICOMTagsOfInterestHelper.h>
#include <mitkDICOMProperty.h>
#include "legacy/mitkDicomSeriesReader.h"
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkLocaleSwitch.h>
#include <mitkIPropertyProvider.h>
#include <mitkPropertyNameHelper.h>
#include <mitkPropertyKeyPath.h>
#include <mitkDICOMIOMetaInformationPropertyConstants.h>
#include <mitkStringProperty.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dctagkey.h>
#include <dcmtk/ofstd/ofstring.h>

#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

namespace
{
  // -------------------------------------------------------------------------
  // PET-specific private-tag workaround (issue #783).
  //
  // The IBSI-SUV-conformant pipeline needs vendor private elements that
  // MITK's standard DICOM-tag plumbing cannot address:
  //   - Siemens (0071,0x22) in private block "SIEMENS MED PT"
  //     (decay-correction reference datetime)
  //   - GE      (0009,0x0D) in private block "GEMS_PETD_01"
  //     (scan reference datetime)
  //   - Philips (7053,0x00) in private block "Philips PET Private Group"
  //     (CNTS SUV-scale factor)
  //   - Philips (7053,0x09) in private block "Philips PET Private Group"
  //     (CNTS activity-scale factor)
  //
  // MITK currently has no general infrastructure for addressing DICOM
  // private data elements: DICOMTag, DICOMTagPath, the DCMTK search-path
  // emitter, and the property-name encoder are (group, element) only and
  // carry no creator string. Extending them is a wide DICOM-module
  // refactor that the SUV milestone could not absorb.
  //
  // As an interim measure we lift the values out of the original
  // DcmDataset post-load, gated on Modality == "PT" (and per-vendor
  // additional gates), and attach them as named properties on the image.
  // The PET SUV pipeline consumes them by name through the regular
  // IPropertyProvider interface; nothing else in the system reads them.
  //
  // This hook lives in a generic DICOM reader because the alternative
  // (PET-module-side post-load enrichment) is unworkable: by the time the
  // BaseData reaches the PET module, the DcmDataset has been released.
  //
  // Once issue #783 is resolved, remove this hook and re-express the
  // private tags through whichever mechanism the issue settles on
  // (extended tags-of-interest, per-vendor private-tag registry, ...).
  // -------------------------------------------------------------------------

  constexpr const char* PROP_SIEMENS_DECAY_DATETIME = "mitk.pet.SiemensDecayDateTime";
  constexpr const char* PROP_GE_SCAN_DATETIME       = "mitk.pet.GEScanDateTime";

  // Philips PET CNTS scale factors. Two mutually exclusive flavors:
  //   - (7053,xx00) "Philips PET Private Group": SUV-scale; pixel * factor yields SUV
  //   - (7053,xx09) "Philips PET Private Group": activity-scale; pixel * factor yields [Bq/mL]
  // Naming mirrors the Siemens/GE properties above.
  constexpr const char* PROP_PHILIPS_SUV_SCALE      = "mitk.pet.PhilipsSUVScale";
  constexpr const char* PROP_PHILIPS_ACTIVITY_SCALE = "mitk.pet.PhilipsActivityScale";

  std::string TrimAndUpper(const std::string& s)
  {
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    auto last = s.find_last_not_of(" \t\r\n");
    std::string trimmed = s.substr(first, last - first + 1);
    std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return trimmed;
  }

  // Walk the DICOM private-creator block of a private group looking for
  // a creator-string match. Returns the offset shift
  // (creator_element << 8) on hit, or 0xFFFF on miss.
  Uint16 FindPrivateCreatorOffset(DcmDataset* dataset, Uint16 group, const char* creator)
  {
    for (Uint16 elem = 0x10; elem <= 0xFF; ++elem)
    {
      OFString value;
      if (dataset->findAndGetOFString(DcmTagKey(group, elem), value).good())
      {
        if (value == creator)
        {
          return static_cast<Uint16>(elem << 8);
        }
      }
    }
    return 0xFFFF;
  }

  std::string ReadPrivateString(DcmDataset* dataset, Uint16 group,
                                const char* creator, Uint16 elementOffset)
  {
    const Uint16 base = FindPrivateCreatorOffset(dataset, group, creator);
    if (0xFFFF == base) return {};
    OFString value;
    if (dataset->findAndGetOFString(
          DcmTagKey(group, static_cast<Uint16>(base | elementOffset)), value).good())
    {
      return std::string(value.c_str());
    }
    return {};
  }

  // Per-frame lift: open each file in the frame list once, extract the
  // requested private element, and build a DICOMCachedValueLookupTable
  // keyed by frame index (with the descriptor's frame -> (t, s) mapping
  // computed inline). Returns true if every frame yielded a non-empty
  // value; partial-availability data falls through to the next step.
  bool LiftPrivateTagPerFrame(const mitk::DICOMImageBlockDescriptor& desc,
                              Uint16 group, const char* creator, Uint16 elementOffset,
                              mitk::DICOMCachedValueLookupTable& outTable)
  {
    const auto& frameList = desc.GetImageFrameList();
    if (frameList.empty()) return false;
    const int framesPerTimeStep = desc.GetNumberOfFramesPerTimeStep();
    if (framesPerTimeStep <= 0) return false;

    bool anyValue = false;
    for (std::size_t i = 0; i < frameList.size(); ++i)
    {
      const auto& frame = frameList[i];
      if (frame.IsNull() || frame->Filename.empty()) return false;

      DcmFileFormat dff;
      // ERM_autoDetect (default): we need the dataset, not just the file
      // meta header. The vendor private tags live in groups 0x0071 / 0x0009
      // in the dataset, which ERM_metaOnly skips.
      if (!dff.loadFile(frame->Filename.c_str(), EXS_Unknown,
                        EGL_noChange, DCM_MaxReadLength,
                        ERM_autoDetect).good())
      {
        return false;
      }
      DcmDataset* ds = dff.getDataset();
      if (nullptr == ds) return false;

      const std::string v = ReadPrivateString(ds, group, creator, elementOffset);
      if (v.empty()) return false;

      const unsigned int t =
        static_cast<unsigned int>(i) / static_cast<unsigned int>(framesPerTimeStep);
      const unsigned int s =
        static_cast<unsigned int>(i) % static_cast<unsigned int>(framesPerTimeStep);
      outTable.SetTableValue(static_cast<int>(i), { t, s, v });
      anyValue = true;
    }
    return anyValue;
  }

  // Convenience: per-frame lift + property attach in one step. Returns
  // true when the lift succeeded and the property was attached.
  bool LiftAndAttach(const mitk::DICOMImageBlockDescriptor& desc,
                     mitk::BaseData*                        data,
                     Uint16                                 group,
                     const char*                            creator,
                     Uint16                                 elementOffset,
                     const char*                            propName)
  {
    mitk::DICOMCachedValueLookupTable table;
    if (!LiftPrivateTagPerFrame(desc, group, creator, elementOffset, table))
    {
      return false;
    }
    auto prop = mitk::GetDICOMPropertyForDICOMValuesFunctor(table);
    if (prop.IsNull()) return false;

    data->SetProperty(propName, prop);
    return true;
  }

  // Hook called per loaded image. No-op unless modality is "PT". Each
  // vendor branch is independent; they all share the same lift
  // mechanism (LiftPrivateTagPerFrame) and per-(t,s) property attach.
  //
  // The lift visits every file in the frame list (DCMTK in metaInfo-only
  // read mode) so the resulting DICOMProperty is per-(t, s) -- exactly
  // what mitk::GetDICOMPropertyForDICOMValuesFunctor produces for a
  // standard tag-of-interest. When the private value is identical
  // across all files (the common case) the SUV pipeline's per-slice
  // GetValue(t, s, ...) loop returns the same value for every slot, so
  // the resulting decay map / scale factor is uniform without any
  // special-casing here. Multi-bed acquisitions where the value varies
  // per file are handled automatically by the same per-slice path.
  void LiftPETVendorPrivateTags(const mitk::DICOMImageBlockDescriptor& desc,
                                mitk::BaseData*                        data)
  {
    if (nullptr == data) return;

    const std::string modality = TrimAndUpper(
      mitk::GetFirstDICOMValueAsString(data, mitk::DICOMTagPath(0x0008, 0x0060)));
    if ("PT" != modality && "PET" != modality) return;

    const std::string manuf = TrimAndUpper(
      mitk::GetFirstDICOMValueAsString(data, mitk::DICOMTagPath(0x0008, 0x0070)));

    // ---- Siemens decay-correction reference datetime ----
    if (std::string::npos != manuf.find("SIEMENS"))
    {
      LiftAndAttach(desc, data, 0x0071, "SIEMENS MED PT", 0x22,
                    PROP_SIEMENS_DECAY_DATETIME);
    }

    // ---- GE scan reference datetime ----
    const bool isGE = (manuf == "GE")
                   || (manuf.size() >= 3 && manuf.substr(0, 3) == "GE ");
    if (isGE)
    {
      LiftAndAttach(desc, data, 0x0009, "GEMS_PETD_01", 0x0D,
                    PROP_GE_SCAN_DATETIME);
    }

    // ---- Philips CNTS scale factors ----
    //
    // Gated on Units==CNTS as well as Manufacturer. The two factors are
    // mutually exclusive in real Philips data (one or the other, never
    // both), but both are looked up defensively in case a synthetic
    // input carries both.
    if (std::string::npos != manuf.find("PHILIPS"))
    {
      const std::string units = TrimAndUpper(
        mitk::GetFirstDICOMValueAsString(data, mitk::DICOMTagPath(0x0054, 0x1001)));
      if ("CNTS" == units)
      {
        constexpr const char* kPhilipsCreator = "Philips PET Private Group";
        LiftAndAttach(desc, data, 0x7053, kPhilipsCreator, 0x00,
                      PROP_PHILIPS_SUV_SCALE);
        LiftAndAttach(desc, data, 0x7053, kPhilipsCreator, 0x09,
                      PROP_PHILIPS_ACTIVITY_SCALE);
      }
    }
  }
}

namespace mitk
{

  BaseDICOMReaderService::BaseDICOMReaderService(const std::string& description)
    : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), description)
{
}

BaseDICOMReaderService::BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description)
  : AbstractFileReader(customType, description)
{
}

void BaseDICOMReaderService::SetOnlyRegardOwnSeries(bool regard)
{
  m_OnlyRegardOwnSeries = regard;
}

bool BaseDICOMReaderService::GetOnlyRegardOwnSeries() const
{
  return m_OnlyRegardOwnSeries;
}


std::vector<itk::SmartPointer<BaseData> > BaseDICOMReaderService::DoRead()
{
  std::vector<BaseData::Pointer> result;


  const std::string fileName = this->GetLocalFileName();
  //special handling of Philips 3D US DICOM.
  //Copied from DICOMSeriesReaderService
  if (DicomSeriesReader::IsPhilips3DDicom(fileName))
  {
      MITK_INFO << "it is a Philips3D US Dicom file" << std::endl;
      mitk::LocaleSwitch localeSwitch("C");
      std::locale previousCppLocale(std::cin.getloc());
      std::locale l("C");
      std::cin.imbue(l);

      DataNode::Pointer node = DataNode::New();
      mitk::DicomSeriesReader::StringContainer stringvec;
      stringvec.push_back(fileName);
      if (DicomSeriesReader::LoadDicomSeries(stringvec, *node))
      {
          BaseData::Pointer data = node->GetData();
          StringProperty::Pointer nameProp = StringProperty::New(itksys::SystemTools::GetFilenameName(fileName));
          data->GetPropertyList()->SetProperty("name", nameProp);
          result.push_back(data);
      }
      std::cin.imbue(previousCppLocale);
      return result;
  }

  //Normal DICOM handling (It wasn't a Philips 3D US)
  mitk::StringList relevantFiles = this->GetDICOMFilesInSameDirectory();

  if (relevantFiles.empty())
  {
      MITK_INFO << "DICOMReader service found no relevant files in specified location. No data is loaded. Location: "<<fileName;
  }
  else
  {
    bool pathIsDirectory = itksys::SystemTools::FileIsDirectory(fileName);

    if (!pathIsDirectory && m_OnlyRegardOwnSeries)
    {
      relevantFiles = mitk::FilterDICOMFilesForSameSeries(fileName, relevantFiles);
    }

    mitk::DICOMFileReader::Pointer reader = this->GetReader(relevantFiles);

      if(reader.IsNull())
      {
          MITK_INFO << "DICOMReader service found no suitable reader configuration for relevant files.";
      }
      else
      {
        if (!pathIsDirectory)
        { //we ensure that we only load the relevant image block files
          const auto nrOfOutputs = reader->GetNumberOfOutputs();
          for (unsigned int outputIndex = 0; outputIndex < nrOfOutputs; ++outputIndex)
          {
            const auto frameList = reader->GetOutput(outputIndex).GetImageFrameList();

            auto finding = std::find_if(frameList.begin(), frameList.end(), [&](const DICOMImageFrameInfo::Pointer& frame) { return frame->Filename == fileName; });

            if (finding != frameList.end())
            { //we have the block containing the fileName -> these are the really relevant files.
              relevantFiles.resize(frameList.size());
              std::transform(frameList.begin(), frameList.end(), relevantFiles.begin(), [](const DICOMImageFrameInfo::Pointer& frame) { return frame->Filename; });
              break;
            }
          }
        }
          const unsigned int ntotalfiles = relevantFiles.size();

          for( unsigned int i=0; i< ntotalfiles; i++)
          {
            m_ReadFiles.push_back( relevantFiles.at(i) );
          }

          reader->SetAdditionalTagsOfInterest(mitk::GetCurrentDICOMTagsOfInterest());
          reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);
          reader->SetInputFiles(relevantFiles);

          mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
          scanner->AddTagPaths(reader->GetTagsOfInterest());
          scanner->SetInputFiles(relevantFiles);
          scanner->Scan();

          reader->SetTagCache(scanner->GetScanCache());
          reader->AnalyzeInputFiles();
          reader->LoadImages();

          for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
          {
            const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
            mitk::BaseData::Pointer data = desc.GetMitkImage().GetPointer();

            if (data.IsNotNull())
            {
              std::string nodeName = GenerateNameFromDICOMProperties(&desc);

              StringProperty::Pointer nameProp = StringProperty::New(nodeName);
              data->SetProperty("name", nameProp);

              data->SetProperty(PropertyKeyPathToPropertyName(DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION()), StringProperty::New(reader->GetConfigurationLabel()));

              // PET workaround for vendor private tags
              // (Siemens/GE timing, Philips CNTS scale factors). See the
              // detailed comment on LiftPETVendorPrivateTags above and
              // issue #783 for the design discussion.
              LiftPETVendorPrivateTags(desc, data.GetPointer());

              result.push_back(data);
            }
            else
            {
              MITK_ERROR << "Errpr occured when loading a DICOM image block. The invalid block contained at least the following file: " << desc.GetImageFrameList().front()->Filename;
            }
          }
      }
  }

  return result;
}

StringList BaseDICOMReaderService::GetDICOMFilesInSameDirectory() const
{
  std::string fileName = this->GetLocalFileName();

  mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(fileName);

  return relevantFiles;
}

IFileReader::ConfidenceLevel BaseDICOMReaderService::GetConfidenceLevel() const
{
  IFileReader::ConfidenceLevel abstractConfidence = AbstractFileReader::GetConfidenceLevel();

  if (Unsupported == abstractConfidence)
  {
    if (itksys::SystemTools::FileIsDirectory(this->GetInputLocation().c_str()))
    {
      // In principle we support dicom directories
      return Supported;
    }
  }

  return abstractConfidence;
}

std::string GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider)
{
  std::string nodeName = mitk::DataNode::NO_NAME_VALUE();

  auto studyProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x1030).c_str());
  if (studyProp.IsNotNull())
  {
    nodeName = studyProp->GetValueAsString();
  }

  auto seriesProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str());

  if (seriesProp.IsNotNull())
  {
    if (studyProp.IsNotNull())
    {
      nodeName += " / ";
    }
    else
    {
      nodeName = "";

    }
    nodeName += seriesProp->GetValueAsString();
  }

  return nodeName;
};

}
