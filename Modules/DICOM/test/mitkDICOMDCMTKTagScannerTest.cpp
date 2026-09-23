/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMMultiFrameTestObject.h"
#include "mitkDICOMTestWarningCounter.h"

#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderTestHelper.h>
#include <mitkIOUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkStringProperty.h>

#include <itksys/SystemTools.hxx>

#include <utility>
#include <vector>

/**
 * Besides plain scanning, the suite covers the functional-group expansion: a
 * tag of interest is registered as the path inside the functional-group macro,
 * and the scanner of the frame-model reader also searches it under the shared
 * and the per-frame root, but only when asked to, only for a file with a frame
 * model and only for a path with more than one node.
 */
class mitkDICOMDCMTKTagScannerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMDCMTKTagScannerTestSuite);

  MITK_TEST(DeepScanning);
  MITK_TEST(MultiFileScanning);
  MITK_TEST(ClassicFileStoresTheSameFindingsWithTheSwitch);
  MITK_TEST(FrameModelFileIsSearchedInBothGroups);
  MITK_TEST(NoRootedFindingWithoutTheSwitch);
  MITK_TEST(RaggedFileIsNotExpanded);
  MITK_TEST(SingleElementPathIsNotExpanded);
  MITK_TEST(RootedRegistrationIsNotExpandedAndWarnsOncePerScan);
  MITK_TEST(RootedRegistrationDoesNotWarnWithoutFrameModel);
  MITK_TEST(NoFrameModelWithoutTheSwitch);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::DICOMDCMTKTagScanner::Pointer scanner;

  mitk::StringList doseFiles;
  mitk::StringList ctFiles;

  std::string m_TempDir;
  unsigned int m_FileCounter = 0;

  static constexpr unsigned int FRAME_COUNT = 4;

  static mitk::DICOMTagPath MacroRelative(unsigned int macroGroup,
                                          unsigned int macroElement,
                                          unsigned int leafGroup,
                                          unsigned int leafElement)
  {
    mitk::DICOMTagPath path;
    path.AddAnySelection(macroGroup, macroElement);
    path.AddElement(leafGroup, leafElement);
    return path;
  }

  static mitk::DICOMTagPath RescaleSlope() { return MacroRelative(0x0028, 0x9145, 0x0028, 0x1053); }
  static mitk::DICOMTagPath FrameReferenceDateTime() { return MacroRelative(0x0020, 0x9111, 0x0018, 0x9151); }

  static mitk::DICOMTagPath Rooted(unsigned int rootElement, const mitk::DICOMTagPath& path)
  {
    return mitk::DICOMTagPath().AddAnySelection(0x5200, rootElement) + path;
  }

  static mitk::DICOMTagPath Explicit(unsigned int rootElement, unsigned int item,
                                     unsigned int macroGroup, unsigned int macroElement,
                                     unsigned int leafGroup, unsigned int leafElement)
  {
    mitk::DICOMTagPath path;
    path.AddSelection(0x5200, rootElement, item);
    path.AddSelection(macroGroup, macroElement, 0);
    path.AddElement(leafGroup, leafElement);
    return path;
  }

  static std::string FrameTime(unsigned int k)
  {
    return "2025010111" + std::string(k < 10 ? "0" : "") + std::to_string(k) + "00.000000+0100";
  }

  static std::string Trimmed(const std::string& value)
  {
    const auto end = value.find_last_not_of(" \0");
    return std::string::npos == end ? std::string() : value.substr(0, end + 1);
  }

  mitk::DICOMMultiFrameTestObject MakeEnhanced() const
  {
    auto object = mitk::DICOMMultiFrameTestObject::EnhancedPET(FRAME_COUNT);
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = 1.0 + k;
      object.frames[k].frameReferenceDateTime = FrameTime(k);
    }
    return object;
  }

  std::string Write(const mitk::DICOMMultiFrameTestObject& object)
  {
    return object.Write(m_TempDir, "object" + std::to_string(m_FileCounter++) + ".dcm");
  }

  static mitk::DICOMDatasetAccessingImageFrameList Scan(const mitk::StringList& files,
                                                        const std::vector<mitk::DICOMTagPath>& paths,
                                                        bool expand)
  {
    auto aScanner = mitk::DICOMDCMTKTagScanner::New();
    aScanner->SetInputFiles(files);
    for (const auto& path : paths)
    {
      aScanner->AddTagPath(path);
    }
    aScanner->SetReadFrameModel(expand);
    aScanner->Scan();

    return aScanner->GetFrameInfoList();
  }

  /** Path and value of every finding, so two scans can be compared as a whole. */
  static std::vector<std::pair<std::string, std::string>> Rendered(
    const mitk::DICOMDatasetAccess::FindingsListType& findings)
  {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& finding : findings)
    {
      result.emplace_back(finding.path.ToStr(), finding.value);
    }
    return result;
  }

public:

  void setUp() override
  {
    doseFiles.push_back(GetTestDataFilePath("RT/Dose/RD.dcm"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/100"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/101"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/102"));
    ctFiles.push_back(GetTestDataFilePath("TinyCTAbdomen/104"));

    scanner = mitk::DICOMDCMTKTagScanner::New();

    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkDICOMDCMTKTagScannerTestXXXXXX");
  }

  void tearDown() override
  {
    if (!m_TempDir.empty())
    {
      itksys::SystemTools::RemoveADirectory(m_TempDir);
      m_TempDir.clear();
    }
  }

  void DeepScanning()
  {
    mitk::DICOMTagPath planUIDPath;
    planUIDPath.AddAnySelection(0x300C, 0x0002).AddElement(0x0008, 0x1155);
    mitk::DICOMTagPath planUIDPathRef;
    planUIDPathRef.AddSelection(0x300C, 0x0002, 0).AddElement(0x0008,0x1155);

    mitk::DICOMTagPath patientName(0x0010, 0x0010);

    scanner->SetInputFiles(doseFiles);
    scanner->AddTagPath(planUIDPath);
    scanner->AddTagPath(patientName);

    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", frames.size() == 1);

    mitk::DICOMDatasetAccess::FindingsListType findings = frames.front()->GetTagValueAsString(planUIDPath);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of first plan finding", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of first plan finding", findings.front().path == planUIDPathRef);
    CPPUNIT_ASSERT_MESSAGE("Testing value of first plan finding", findings.front().value == "1.2.826.0.1.3680043.8.176.2013826104526987.672.1228523524");

    findings = frames.front()->GetTagValueAsString(patientName);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of first plan finding", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of first plan finding", findings.front().path == patientName);
    CPPUNIT_ASSERT_MESSAGE("Testing value of first plan finding", findings.front().value == "L_H");
  }

  void MultiFileScanning()
  {
    mitk::DICOMTagPath instanceUID(0x0008, 0x0018);

    scanner->SetInputFiles(ctFiles);
    scanner->AddTagPath(instanceUID);

    scanner->Scan();

    mitk::DICOMDatasetAccessingImageFrameList frames = scanner->GetFrameInfoList();
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", frames.size() == 4);

    mitk::DICOMDatasetAccess::FindingsListType findings = frames[0]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 0", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 0", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 0", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940051");

    findings = frames[1]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 1", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 1", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 1", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940052");

    findings = frames[2]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 2", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 2", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 2", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940053");

    findings = frames[3]->GetTagValueAsString(instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing DICOMDCMTKTagScanner::GetFrameInfoList()", findings.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Testing validity of instance uid finding of frame 3", findings.front().isValid);
    CPPUNIT_ASSERT_MESSAGE("Testing path of instance uid finding of frame 3", findings.front().path == instanceUID);
    CPPUNIT_ASSERT_MESSAGE("Testing value of instance uid finding of frame 3", findings.front().value == "1.2.276.0.99.1.4.8323329.3795.1303917947.940055");
  }

  /** A file without functional groups gets no rooted search, so switching the
      expansion on changes nothing it stores. */
  void ClassicFileStoresTheSameFindingsWithTheSwitch()
  {
    const std::vector<mitk::DICOMTagPath> paths = {
      RescaleSlope(), FrameReferenceDateTime(), mitk::DICOMTagPath(0x0008, 0x0018), mitk::DICOMTagPath(0x0020, 0x0032)
    };

    const auto withoutSwitch = Scan(ctFiles, paths, false);
    const auto withSwitch = Scan(ctFiles, paths, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One file-level info per file either way", withoutSwitch.size(), withSwitch.size());

    for (std::size_t file = 0; file < withSwitch.size(); ++file)
    {
      for (const auto& path : paths)
      {
        const auto expected = Rendered(withoutSwitch[file]->GetTagValueAsString(path));
        const auto actual = Rendered(withSwitch[file]->GetTagValueAsString(path));
        CPPUNIT_ASSERT_MESSAGE("Same findings for " + path.ToStr() + " in file " + std::to_string(file),
                               expected == actual);
      }
    }
  }

  /** A per-frame attribute is stored once per frame under (5200,9230)[k], a
      shared one once under (5200,9229)[0]. */
  void FrameModelFileIsSearchedInBothGroups()
  {
    auto object = this->MakeEnhanced();
    object.rescalePlacement = mitk::DICOMMultiFrameTestObject::RescalePlacement::Shared;
    object.frames.front().slope = 7.0;

    const auto frames = Scan({ this->Write(object) }, { RescaleSlope(), FrameReferenceDateTime() }, true);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One file-level info for the file", std::size_t(1), frames.size());

    const auto times = frames.front()->GetTagValueAsString(Rooted(0x9230, FrameReferenceDateTime()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One per-frame finding per frame", std::size_t(FRAME_COUNT), times.size());
    unsigned int k = 0;
    for (const auto& finding : times)
    {
      CPPUNIT_ASSERT_MESSAGE("Finding " + std::to_string(k) + " under its explicit per-frame root",
                             finding.path == Explicit(0x9230, k, 0x0020, 0x9111, 0x0018, 0x9151));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Frame " + std::to_string(k) + "'s value", FrameTime(k), Trimmed(finding.value));
      ++k;
    }

    const auto shared = frames.front()->GetTagValueAsString(Rooted(0x9229, RescaleSlope()));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One shared finding", std::size_t(1), shared.size());
    CPPUNIT_ASSERT_MESSAGE("The shared finding under its explicit shared root",
                           shared.front().path == Explicit(0x9229, 0, 0x0028, 0x9145, 0x0028, 0x1053));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The shared slope", 7.0, std::stod(shared.front().value), 1e-9);

    CPPUNIT_ASSERT_MESSAGE("No per-frame finding for an attribute that is only shared",
                           frames.front()->GetTagValueAsString(Rooted(0x9230, RescaleSlope())).empty());
  }

  void NoRootedFindingWithoutTheSwitch()
  {
    auto object = this->MakeEnhanced();
    object.rescalePlacement = mitk::DICOMMultiFrameTestObject::RescalePlacement::Shared;

    const auto frames = Scan({ this->Write(object) }, { RescaleSlope(), FrameReferenceDateTime() }, false);

    CPPUNIT_ASSERT_MESSAGE("No per-frame finding without the switch",
                           frames.front()->GetTagValueAsString(Rooted(0x9230, FrameReferenceDateTime())).empty());
    CPPUNIT_ASSERT_MESSAGE("No shared finding without the switch",
                           frames.front()->GetTagValueAsString(Rooted(0x9229, RescaleSlope())).empty());
  }

  /** A ragged file keeps file-level infos, so expanding it would store
      findings nothing reads. */
  void RaggedFileIsNotExpanded()
  {
    auto object = this->MakeEnhanced();
    object.perFrameItemCountOverride = FRAME_COUNT - 1;

    const auto frames = Scan({ this->Write(object) }, { RescaleSlope(), FrameReferenceDateTime() }, true);

    CPPUNIT_ASSERT_MESSAGE("No per-frame finding for a ragged file",
                           frames.front()->GetTagValueAsString(Rooted(0x9230, FrameReferenceDateTime())).empty());
    CPPUNIT_ASSERT_MESSAGE("No per-frame rescale finding for a ragged file",
                           frames.front()->GetTagValueAsString(Rooted(0x9230, RescaleSlope())).empty());
  }

  /** The nested-only boundary: a single element that sits directly in each
      per-frame item is not found through a single-element registration. */
  void SingleElementPathIsNotExpanded()
  {
    auto object = this->MakeEnhanced();
    object.imageCommentsInPerFrameItems = true;
    const std::string file = this->Write(object);

    const mitk::DICOMTagPath imageComments(0x0020, 0x4000);
    mitk::DICOMTagPath rootedImageComments;
    rootedImageComments.AddAnySelection(0x5200, 0x9230).AddElement(0x0020, 0x4000);

    const auto registeredRooted = Scan({ file }, { rootedImageComments }, false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Test precondition: the element is in every per-frame item",
                                 std::size_t(FRAME_COUNT),
                                 registeredRooted.front()->GetTagValueAsString(rootedImageComments).size());

    const auto frames = Scan({ file }, { imageComments }, true);
    CPPUNIT_ASSERT_MESSAGE("A single-element registration is not searched under the roots",
                           frames.front()->GetTagValueAsString(rootedImageComments).empty());
  }

  /** A path already rooted in a functional group is searched as registered,
      not rooted a second time, and its registrant is told once per scan that
      it reaches no property for a frame-model object. */
  void RootedRegistrationIsNotExpandedAndWarnsOncePerScan()
  {
    const auto rooted = Rooted(0x9230, RescaleSlope());
    const mitk::StringList files = { this->Write(this->MakeEnhanced()), this->Write(this->MakeEnhanced()) };

    mitk::DICOMTestWarningCounter warnings(rooted.ToStr());
    const auto frames = Scan(files, { rooted }, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("One warning per scan however many files", 1u, warnings.GetCount());
    for (const auto& frame : frames)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The registered path is found as registered, once per frame",
                                   std::size_t(FRAME_COUNT), frame->GetTagValueAsString(rooted).size());
    }
  }

  /** The warning concerns frame-model files only; a classic series with such
      a registrant stays quiet. */
  void RootedRegistrationDoesNotWarnWithoutFrameModel()
  {
    const auto rooted = Rooted(0x9230, RescaleSlope());

    mitk::DICOMTestWarningCounter warnings(rooted.ToStr());
    Scan(ctFiles, { rooted }, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("No warning without a frame-model file", 0u, warnings.GetCount());
  }

  /** A scanner that does not feed a frame-model reader, such as the RT, SEG or
      CEST ones, neither detects a frame model nor warns about one, however
      the file is laid out. */
  void NoFrameModelWithoutTheSwitch()
  {
    auto ragged = this->MakeEnhanced();
    ragged.perFrameItemCountOverride = FRAME_COUNT - 1;
    const std::string raggedFile = this->Write(ragged);

    auto aScanner = mitk::DICOMDCMTKTagScanner::New();
    aScanner->SetInputFiles({ this->Write(this->MakeEnhanced()), raggedFile });
    aScanner->AddTagPath(RescaleSlope());

    mitk::DICOMTestWarningCounter warnings(raggedFile);
    aScanner->Scan();

    CPPUNIT_ASSERT_MESSAGE("No frame model without the switch", !aScanner->GetScanCache()->HasAnyFrameModel());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No frame-model warning without the switch", 0u, warnings.GetCount());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMDCMTKTagScanner)
