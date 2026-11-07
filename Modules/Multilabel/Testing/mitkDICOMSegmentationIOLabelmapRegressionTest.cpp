/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestingMacros.h>

#include <itkImageFileReader.h>

#include <dcmqi/ColorUtilities.h>
#include <dcmqi/Helper.h>
#include <dcmqi/Itk2DicomConverter.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// Regression guard for the labelmap branch of
// mitk::DICOMSegmentationIO::DoRead. A labelmap-format DICOM SEG (SOP Class
// UID_LabelMapSegmentationStorage, Sup 243) is read through MITK. The label
// attributes are checked against the dcmqi metadata the SEG is built from.
// The voxels are checked against the reference segmentation in MITK-Data,
// an export of MITK's own reader (MITK-Data a1e677b, updated in 08f86e5),
// which pins them to what earlier reader versions yielded.
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

  mitk::DICOMCodeSequence CodeFromJSON(const nlohmann::json& code)
  {
    return mitk::DICOMCodeSequence(code.at("CodeValue").get<std::string>(),
                                   code.at("CodingSchemeDesignator").get<std::string>(),
                                   code.at("CodeMeaning").get<std::string>());
  }

  /**
   * The label the reader is expected to yield for one segment of the dcmqi metadata. A key
   * this function does not know fails the test, so that no attribute goes unchecked.
   */
  mitk::Label::Pointer LabelFromJSON(const nlohmann::json& segment)
  {
    static const std::set<std::string> knownKeys = { "labelID", "SegmentLabel", "SegmentDescription",
      "SegmentedPropertyCategoryCodeSequence", "SegmentedPropertyTypeCodeSequence", "SegmentAlgorithmType",
      "SegmentAlgorithmName", "recommendedDisplayRGBValue" };
    for (const auto& item : segment.items())
    {
      if (knownKeys.count(item.key()) == 0)
        mitkThrow() << "The metadata has the segment attribute " << item.key() << ", which the test does not check.";
    }

    auto label = mitk::Label::New();
    label->SetValue(segment.at("labelID").get<mitk::Label::PixelType>());
    // The format carries no opacity, visibility or lock state; the reader's values are pinned.
    label->SetOpacity(0.6f);
    label->SetVisible(true);
    label->SetLocked(true);
    label->SetName(segment.at("SegmentLabel").get<std::string>());
    if (segment.contains("SegmentDescription"))
      label->SetDescription(segment["SegmentDescription"].get<std::string>());
    if (segment.contains("SegmentedPropertyCategoryCodeSequence"))
      label->SetSegmentedPropertyCategory(CodeFromJSON(segment["SegmentedPropertyCategoryCodeSequence"]));
    if (segment.contains("SegmentedPropertyTypeCodeSequence"))
    {
      label->SetSegmentedPropertyType(
        mitk::DICOMCodeSequenceWithModifiers(CodeFromJSON(segment["SegmentedPropertyTypeCodeSequence"])));
    }
    label->SetAlgorithmTypeStr(segment.at("SegmentAlgorithmType").get<std::string>());
    if (segment.contains("SegmentAlgorithmName"))
      label->SetAlgorithmName(segment["SegmentAlgorithmName"].get<std::string>());

    // The SEG stores the colour as integer CIELab, which dcmqi converts from sRGB on writing
    // and back on reading; the reader then divides by 255.
    const auto rgb = segment.at("recommendedDisplayRGBValue").get<std::vector<int>>();
    int cielab[3];
    dcmqi::ColorUtilities::getIntegerScaledCIELabPCSFromSRGB(cielab[0], cielab[1], cielab[2], rgb.at(0), rgb.at(1), rgb.at(2));
    int roundTrip[3];
    dcmqi::ColorUtilities::getSRGBFromIntegerScaledCIELabPCS(roundTrip[0], roundTrip[1], roundTrip[2],
      static_cast<Uint16>(cielab[0]), static_cast<Uint16>(cielab[1]), static_cast<Uint16>(cielab[2]));
    mitk::Color color;
    color.Set(roundTrip[0] / 255.f, roundTrip[1] / 255.f, roundTrip[2] / 255.f);
    label->SetColor(color);

    return label;
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
  const mitk::test::ScopedTemporaryDirectory tempDir("mitkDICOMSegmentationIOLabelmapRegressionTest-XXXXXX");
  const std::string tempFile = tempDir.GetFilePath("labelmap.dcm");
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

  // dcmqi's labelmap writer inserts a Background segment (number 0) so the
  // SEG carries 3 SegmentSequence items: Background + liver + spine. MITK
  // must strip Background on read (pixel value 0 maps to UNLABELED), so
  // the reloaded seg has exactly 2 labels and none of them is "Background".
  MITK_TEST_CONDITION(loadedSeg->GetTotalNumberOfLabels() == 2u,
    "Labelmap SEG read strips the Sup 243 Background segment (2 labels remain)");
  bool hasBackgroundLabel = false;
  bool hasZeroValueLabel = false;
  for (const auto labelValue : loadedSeg->GetAllLabelValues())
  {
    if (labelValue == mitk::MultiLabelSegmentation::UNLABELED_VALUE)
      hasZeroValueLabel = true;
    const auto label = loadedSeg->GetLabel(labelValue);
    if (label.IsNotNull() && std::string(label->GetName()) == "Background")
      hasBackgroundLabel = true;
  }
  MITK_TEST_CONDITION(!hasZeroValueLabel,
    "No reloaded label may carry the UNLABELED_VALUE (0)");
  MITK_TEST_CONDITION(!hasBackgroundLabel,
    "No reloaded label may be named 'Background'");

  const auto reference = mitk::IOUtil::Load(referencePath);
  MITK_TEST_CONDITION_REQUIRED(reference.size() == 1,
    "Reference segmentation loaded as exactly one result");

  auto* referenceSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(reference[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(referenceSeg != nullptr,
    "Reference is a MultiLabelSegmentation");

  // The attributes, against the metadata the SEG was built from.
  std::vector<std::string> differences;
  std::set<mitk::Label::PixelType> segmentValues;
  const auto losses = mitk::test::DICOMSegReadLabelLosses();
  const auto metadataJSON = nlohmann::json::parse(metadata);
  for (const auto& group : metadataJSON.at("segmentAttributes"))
  {
    for (const auto& segment : group)
    {
      auto expected = LabelFromJSON(segment);
      for (const auto& loss : losses)
        loss.apply(*expected);
      segmentValues.insert(expected->GetValue());
      if (!loadedSeg->ExistLabel(expected->GetValue()))
      {
        differences.push_back(mitk::test::detail::DescribeLabel(expected) + ": missing");
        continue;
      }
      mitk::test::detail::CompareLabelProperties(*expected, *loadedSeg->GetLabel(expected->GetValue()), 0.f, differences);
    }
  }
  for (const auto value : loadedSeg->GetAllLabelValues())
  {
    if (segmentValues.count(value) == 0)
      differences.push_back(mitk::test::detail::DescribeLabel(loadedSeg->GetLabel(value)) + ": unexpected");
  }
  MITK_TEST_CONDITION(differences.empty(),
    "Loaded labelmap SEG matches the metadata it was built from" << mitk::test::JoinDifferences(differences));

  // The voxels, against the reference.
  MITK_TEST_CONDITION_REQUIRED(referenceSeg->GetNumberOfGroups() == 1,
    "The reference has a single group");
  MITK_TEST_CONDITION(loadedSeg->GetLabelValuesByGroup(0) == referenceSeg->GetLabelValuesByGroup(0),
    "Loaded labelmap SEG has the label values of the reference");
  MITK_TEST_CONDITION(mitk::Equal(*loadedSeg->GetGroupImage(0), *referenceSeg->GetGroupImage(0), mitk::eps, true),
    "Loaded labelmap SEG matches the reference in geometry and voxels");

  MITK_TEST_END();
}
