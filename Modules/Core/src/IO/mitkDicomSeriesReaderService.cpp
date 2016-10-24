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

#include "mitkDicomSeriesReaderService.h"

#include <mitkCustomMimeType.h>
#include <mitkDicomSeriesReader.h>
#include <mitkIOMimeTypes.h>
#include <mitkImage.h>
#include <mitkLocaleSwitch.h>
#include <mitkProgressBar.h>
#include <mitkPropertyNameHelper.h>

#include <iostream>

namespace mitk
{
  DicomSeriesReaderService::DicomSeriesReaderService()
    : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), "MITK DICOM Reader")
  {
    this->RegisterService();
  }

  std::vector<itk::SmartPointer<BaseData>> DicomSeriesReaderService::Read()
  {
    std::vector<BaseData::Pointer> result;

    mitk::LocaleSwitch localeSwitch("C");
    std::locale previousCppLocale(std::cin.getloc());
    std::locale l("C");
    std::cin.imbue(l);

    std::string fileName = this->GetLocalFileName();
    if (DicomSeriesReader::IsPhilips3DDicom(fileName))
    {
      MITK_INFO << "it is a Philips3D US Dicom file" << std::endl;
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

    DicomSeriesReader::FileNamesGrouping imageBlocks =
      DicomSeriesReader::GetSeries(itksys::SystemTools::GetFilenamePath(fileName),
                                   true); // true = group gantry tilt images
    const unsigned int size = imageBlocks.size();

    ProgressBar::GetInstance()->AddStepsToDo(size);
    ProgressBar::GetInstance()->Progress();

    unsigned int outputIndex = 0u;
    const DicomSeriesReader::FileNamesGrouping::const_iterator n_end = imageBlocks.end();

    for (DicomSeriesReader::FileNamesGrouping::const_iterator n_it = imageBlocks.begin(); n_it != n_end; ++n_it)
    {
      const std::string &uid = n_it->first;
      DataNode::Pointer node = DataNode::New();

      const DicomSeriesReader::ImageBlockDescriptor &imageBlockDescriptor(n_it->second);

      MITK_INFO << "--------------------------------------------------------------------------------";
      MITK_INFO << "DicomSeriesReader: Loading DICOM series " << outputIndex << ": Series UID "
                << imageBlockDescriptor.GetSeriesInstanceUID() << std::endl;
      MITK_INFO << "  " << imageBlockDescriptor.GetFilenames().size() << " '" << imageBlockDescriptor.GetModality()
                << "' files (" << imageBlockDescriptor.GetSOPClassUIDAsString() << ") loaded into 1 mitk::Image";
      MITK_INFO << "  multi-frame: " << (imageBlockDescriptor.IsMultiFrameImage() ? "Yes" : "No");
      MITK_INFO << "  reader support: " << DicomSeriesReader::ReaderImplementationLevelToString(
                                             imageBlockDescriptor.GetReaderImplementationLevel());
      MITK_INFO << "  pixel spacing type: "
                << DicomSeriesReader::PixelSpacingInterpretationToString(imageBlockDescriptor.GetPixelSpacingType());
      MITK_INFO << "  gantry tilt corrected: " << (imageBlockDescriptor.HasGantryTiltCorrected() ? "Yes" : "No");
      MITK_INFO << "  3D+t: " << (imageBlockDescriptor.HasMultipleTimePoints() ? "Yes" : "No");
      MITK_INFO << "--------------------------------------------------------------------------------";

      if (DicomSeriesReader::LoadDicomSeries(n_it->second.GetFilenames(), *node, true, true, true))
      {
        BaseData::Pointer data = node->GetData();
        PropertyList::ConstPointer dataProps = data->GetPropertyList().GetPointer();

        std::string nodeName(uid);
        std::string studyDescription;

        if (GetBackwardsCompatibleDICOMProperty(
              0x0008, 0x1030, "dicom.study.StudyDescription", dataProps, studyDescription))
        {
          nodeName = studyDescription;
          std::string seriesDescription;

          if (GetBackwardsCompatibleDICOMProperty(
                0x0008, 0x103e, "dicom.study.SeriesDescription", dataProps, seriesDescription))
          { // may not be a string property so use the generic access.
            nodeName += "/" + seriesDescription;
          }
        }

        StringProperty::Pointer nameProp = StringProperty::New(nodeName);
        data->SetProperty("name", nameProp);

        result.push_back(data);
        ++outputIndex;
      }
      else
      {
        MITK_ERROR << "DicomSeriesReader: Skipping series " << outputIndex << " due to some unspecified error..."
                   << std::endl;
      }

      ProgressBar::GetInstance()->Progress();
    }

    std::cin.imbue(previousCppLocale);

    return result;
  }

  DicomSeriesReaderService *DicomSeriesReaderService::Clone() const { return new DicomSeriesReaderService(*this); }
}
