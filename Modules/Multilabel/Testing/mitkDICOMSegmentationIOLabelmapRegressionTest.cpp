/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestingMacros.h>

#include <itkImageFileReader.h>

#include <dcmqi/Helper.h>
#include <dcmqi/Itk2DicomConverter.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Regression guard for the labelmap branch of
// mitk::DICOMSegmentationIO::DoRead. A labelmap-format DICOM SEG (SOP Class
// UID_LabelMapSegmentationStorage, Sup 243) is read through MITK and
// compared against a checked-in MITK reference segmentation.
//
// dcmqi's labelmap converter is driven inline to produce the SEG fixture
// from two single-segment input NRRDs plus three source DICOM images
// (equivalent to itkimage2segimage --segmentationType labelmap). Producing
// the fixture from the test, rather than pinning a pre-generated .dcm,
// keeps the assertion encoding-specific: any future change in the labelmap
// branch lands on this commit during bisect, decoupled from changes to the
// pre-generated fixture's transfer syntax or UIDs.

namespace
{
  std::string ReadEntireFile(const std::string& path)
  {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in)
      return std::string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  std::vector<std::string> SplitCommaList(const std::string& csv)
  {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(csv);
    while (std::getline(stream, token, ','))
    {
      if (!token.empty())
        tokens.push_back(token);
    }
    return tokens;
  }
}

int mitkDICOMSegmentationIOLabelmapRegressionTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DICOMSegmentationIOLabelmapRegression");

  MITK_TEST_CONDITION_REQUIRED(argc == 6,
    "Expected arguments: <metadata-json> <liver-seg-nrrd> <spine-seg-nrrd> "
    "<source-dicoms-comma-separated> <reference-mitk-segmentation>");

  const std::string metadataPath = argv[1];
  const std::string liverSegPath = argv[2];
  const std::string spineSegPath = argv[3];
  const std::string sourceDicomCsv = argv[4];
  const std::string referencePath = argv[5];

  // dcmqi consumes JSON metadata as text; the handler then exposes it as
  // a populated object that the converter reads back.
  const std::string metadata = ReadEntireFile(metadataPath);
  MITK_TEST_CONDITION_REQUIRED(!metadata.empty(),
    "Loaded dcmqi segmentation metadata from JSON");

  using ImageType = itk::Image<short, 3U>;
  using ReaderType = itk::ImageFileReader<ImageType>;

  auto loadSegmentation = [](const std::string& path) -> ImageType::ConstPointer {
    auto reader = ReaderType::New();
    reader->SetFileName(path);
    reader->Update();
    return reader->GetOutput();
  };

  std::vector<ImageType::ConstPointer> segmentations;
  segmentations.emplace_back(loadSegmentation(liverSegPath));
  segmentations.emplace_back(loadSegmentation(spineSegPath));

  // Source DICOMs supply the SEG's geometry and ReferencedSeriesSequence.
  const auto sourceDicomPaths = SplitCommaList(sourceDicomCsv);
  MITK_TEST_CONDITION_REQUIRED(!sourceDicomPaths.empty(),
    "Source DICOM list parsed at least one path");
  std::vector<DcmItem*> sourceDatasets = dcmqi::Helper::loadDatasets(sourceDicomPaths);
  MITK_TEST_CONDITION_REQUIRED(!sourceDatasets.empty(),
    "Source DICOM datasets loaded by dcmqi::Helper::loadDatasets");

  // outputLabelMap = true selects the Sup 243 SOP Class. The remaining
  // flags match dcmqi's CLI defaults for the labelmap test recipes.
  dcmqi::JSONSegmentationMetaInformationHandler handler(metadata.c_str());
  handler.read();
  std::unique_ptr<DcmDataset> labelmapSeg(
    dcmqi::Itk2DicomConverter::itkimage2dcmSegmentation(
      sourceDatasets,
      segmentations,
      handler,
      /*skipEmptySlices=*/true,
      /*useLabelIDAsSegmentNumber=*/true,
      /*referencesGeometryCheck=*/true,
      /*doDicomValueChecks=*/true,
      /*outputLabelMap=*/true));
  for (DcmItem* item : sourceDatasets)
    delete item;
  MITK_TEST_CONDITION_REQUIRED(labelmapSeg != nullptr,
    "dcmqi produced a labelmap DICOM SEG dataset");

  // Verify we really got a labelmap SOP Class and not the binary one,
  // so the test cannot pass against the binary read branch.
  OFString sopClassUid;
  labelmapSeg->findAndGetOFString(DCM_SOPClassUID, sopClassUid);
  MITK_TEST_CONDITION_REQUIRED(
    std::string(sopClassUid.c_str()) == UID_LabelMapSegmentationStorage,
    "Generated SEG carries the Sup 243 labelmap SOP Class");

  // Persist the labelmap SEG to a temp file so the standard MITK reader
  // exercises the entire DoRead path (including the dcmqi factory call
  // that we want to regression-guard).
  const std::string tempFile = mitk::IOUtil::CreateTemporaryFile(
    "mitkDICOMSegmentationIOLabelmapRegressionTest_XXXXXX.dcm");
  DcmFileFormat fileFormat(labelmapSeg.get());
  const auto saveCondition = fileFormat.saveFile(tempFile.c_str(), EXS_LittleEndianExplicit);
  MITK_TEST_CONDITION_REQUIRED(saveCondition.good(),
    "Labelmap SEG dataset saved to temporary file");

  const auto loaded = mitk::IOUtil::Load(tempFile);
  MITK_TEST_CONDITION_REQUIRED(loaded.size() == 1,
    "MITK loader produced exactly one result for the labelmap SEG");

  auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(loadedSeg != nullptr,
    "Loaded data is a MultiLabelSegmentation");

  // One MITK group per labelmap SEG by definition (Sup 243 forbids
  // overlap within a single SEG file); both segments share that group.
  MITK_TEST_CONDITION(loadedSeg->GetNumberOfGroups() == 1,
    "Labelmap SEG loaded as a single MITK group");

  const auto reference = mitk::IOUtil::Load(referencePath);
  MITK_TEST_CONDITION_REQUIRED(reference.size() == 1,
    "Reference segmentation loaded as exactly one result");

  auto* referenceSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(reference[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(referenceSeg != nullptr,
    "Reference is a MultiLabelSegmentation");

  const bool equal = mitk::Equal(*loadedSeg, *referenceSeg, mitk::eps, true);
  if (!equal)
  {
    // First-run aid: on mismatch, persist what the reader produced next to
    // the reference so a human can diff the two and decide whether the
    // reference needs updating or the reader has regressed.
    const std::string actualOut = referencePath + ".actual.nrrd";
    try
    {
      mitk::IOUtil::Save(loadedSeg, actualOut);
      MITK_ERROR << "Labelmap regression mismatch; loaded seg written to "
                 << actualOut;
    }
    catch (const std::exception &e)
    {
      MITK_ERROR << "Labelmap regression mismatch; failed to persist loaded "
                 << "seg for diffing: " << e.what();
    }

    auto dumpLabel = [](const mitk::Label *label, const std::string &tag) {
      if (label == nullptr)
      {
        MITK_ERROR << tag << ": <null>";
        return;
      }
      const auto color = label->GetColor();
      MITK_ERROR << tag
                 << " value=" << label->GetValue()
                 << " name='" << label->GetName()
                 << "' color=(" << color[0] << "," << color[1] << "," << color[2] << ")"
                 << " trackingID='" << label->GetTrackingID()
                 << "' trackingUID='" << label->GetTrackingUID()
                 << "' algorithmType=" << label->GetAlgorithmTypeStr()
                 << " algorithmName='" << label->GetAlgorithmName() << "'";
    };
    MITK_ERROR << "Groups: loaded=" << loadedSeg->GetNumberOfGroups()
               << " reference=" << referenceSeg->GetNumberOfGroups();
    for (const auto value : loadedSeg->GetAllLabelValues())
      dumpLabel(loadedSeg->GetLabel(value), "  loaded");
    for (const auto value : referenceSeg->GetAllLabelValues())
      dumpLabel(referenceSeg->GetLabel(value), "  reference");
  }
  MITK_TEST_CONDITION(equal,
    "Loaded labelmap DICOM SEG matches the reference MultiLabelSegmentation");

  MITK_TEST_END();
}
