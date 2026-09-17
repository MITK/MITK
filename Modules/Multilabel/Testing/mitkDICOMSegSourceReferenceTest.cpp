/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLog.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkSegSourceReferenceTestHelpers.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itksys/SystemTools.hxx>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/ofstd/ofstring.h>

#include <cstdlib>
#include <filesystem>
#include <set>
#include <string>

#ifndef MITK_MULTILABEL_DCMQI_CT3SLICE_DIR
#define MITK_MULTILABEL_DCMQI_CT3SLICE_DIR ""
#endif

/**
 * \brief Covers DICOM SEG source references for a segmentation whose source
 *        came from the DICOM reader.
 *
 * The writer's other suites construct their sources by hand, supplying the
 * DICOM identity the source-image relation needs. This one drives the
 * reader, which is the only way to observe whether that identity is actually
 * produced in a real workflow.
 *
 * Both synthesis modes are written from one fixture on purpose. Synthetic is
 * a fallback rather than an override - the writer mints placeholder source
 * items only when the relation yielded none - so a source-reference defect
 * moves strict mode from success to a hard failure while leaving synthetic
 * mode quietly emitting invented provenance. Asserting strict alone would
 * miss the second half entirely.
 */
class mitkDICOMSegSourceReferenceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegSourceReferenceTestSuite);
  MITK_TEST(StrictWriteReferencesExactlyTheSourceInstances);
  MITK_TEST(SyntheticWriteReferencesExactlyTheSourceInstances);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_Source;
  std::set<std::string> m_ExpectedSOPInstanceUIDs;
  std::string m_TempPath;

  /** Resolve the dcmqi source fixtures, or skip the suite.
   *  The directory arrives as MITK_MULTILABEL_DCMQI_CT3SLICE_DIR, set by the
   *  module's CMakeLists, which registers the test only when the fixtures are
   *  present. The macro is empty otherwise, so this guard is what a driver
   *  launched by hand runs into. */
  static std::string FixtureDirOrSkip()
  {
    const std::string dir(MITK_MULTILABEL_DCMQI_CT3SLICE_DIR);

    if (dir.empty() || !std::filesystem::is_directory(dir))
    {
      MITK_INFO << "dcmqi ct-3slice source DICOMs are unavailable; skipping. "
                   "They ship with the dcmqi source tree, which a non-SuperBuild "
                   "build does not check out.";

      // CppUnit reports an exception from setUp as a test error, so exiting
      // is the only route from inside a fixture to the 77 exit code ctest
      // reads as a skip - cf. Modules/Core/test/mitkImageDataItemTest.cpp.
      std::exit(77);
    }

    return dir;
  }

  static std::string ReadTag(const std::string& path, const DcmTagKey& tag)
  {
    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad() || ff.getDataset() == nullptr)
      return {};

    OFString value;
    if (ff.getDataset()->findAndGetOFString(tag, value).bad())
      return {};

    return value.c_str();
  }

  /** Build the segmentation the Workbench would build: initialised from the
   *  source image and wired to it through the same helper the segmentation
   *  view calls. */
  mitk::MultiLabelSegmentation::Pointer BuildDerivedSeg() const
  {
    constexpr mitk::Label::PixelType labelValue = 1;

    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(m_Source);

    auto label = mitk::Label::New();
    label->SetName("L");
    label->SetValue(labelValue);
    label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label->SetAlgorithmName("SourceReferenceTest");
    seg->AddLabel(label, 0, true, true);

    {
      auto groupImage = seg->GetGroupImage(0);
      mitk::ImageWriteAccessor writeAccessor(groupImage);
      auto* pixels = static_cast<mitk::Label::PixelType*>(writeAccessor.GetData());
      const auto dims = groupImage->GetDimensions();
      const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
      for (unsigned int z = 0; z < dims[2]; ++z)
      {
        pixels[z * sliceSize + 0] = labelValue;
        pixels[z * sliceSize + 1] = labelValue;
      }
    }

    // ImageWriteAccessor holds an exclusive lock on the image; scoping it
    // releases that lock before SetupDerivedSegmentation runs, so code it
    // calls remains free to access the image.
    mitk::LabelSetImageHelper::SetupDerivedSegmentation(seg, m_Source);

    return seg;
  }

  std::string WriteSeg(const mitk::MultiLabelSegmentation* seg,
                       const mitk::IFileWriter::Options& options,
                       const std::string& nameHint) const
  {
    const std::string path = (std::filesystem::path(m_TempPath) / (nameHint + ".dcm")).string();
    mitk::IOUtil::Save(seg, path, options);
    return path;
  }

  /** Fail unless the SEG at segPath references exactly the source instances
   *  the fixture declares - no fewer, so the relation reached the file, and
   *  no more, so nothing was invented alongside it. */
  void AssertReferencesExactlyTheFixture(const std::string& segPath, const std::string& mode) const
  {
    const auto diff = mitk::test::DiffSourceReferences(
      m_ExpectedSOPInstanceUIDs, mitk::test::ReadReferencedSOPInstanceUIDs(segPath));

    if (!diff.empty())
      CPPUNIT_FAIL(mode + "-mode SEG must reference exactly the fixture's source instances:" + diff);
  }

public:
  void setUp() override
  {
    const auto fixtureDir = FixtureDirOrSkip();

    // Claim the temp directory before anything that can throw. tearDown runs
    // even when setUp fails, so a later failure must still find a path it can
    // safely remove.
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-DICOMSegSourceReferenceTest-XXXXXX");

    // Parse the fixtures independently of MITK so the written SEG is graded
    // against what the files declare, not against MITK's own reading of them.
    // ct-3slice holds a single series, so every file here belongs to the block
    // pinned below - the oracle and the loaded image cover the same instances.
    for (const auto& entry : std::filesystem::directory_iterator(fixtureDir))
    {
      if (entry.path().extension() != ".dcm")
        continue;

      const auto sopInstanceUID = ReadTag(entry.path().string(), DCM_SOPInstanceUID);
      if (!sopInstanceUID.empty())
        m_ExpectedSOPInstanceUIDs.insert(sopInstanceUID);
    }

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the fixture declares source SOP Instance UIDs",
                           !m_ExpectedSOPInstanceUIDs.empty());

    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      {"MITK DICOM Reader v2 (autoselect)"}, {""});

    // IOUtil::Load<T> yields only the first output, and BaseDICOMReaderService
    // restricts the read to one block only for a file path, not a directory.
    // 01/02/03.dcm share a series, so the pinned block still spans all three.
    const auto firstSlice = (std::filesystem::path(fixtureDir) / "01.dcm").string();
    m_Source = mitk::IOUtil::Load<mitk::Image>(firstSlice, &readerFunctor);

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the fixture loads as an image",
                           m_Source.IsNotNull());
  }

  void tearDown() override
  {
    m_Source = nullptr;
    m_ExpectedSOPInstanceUIDs.clear();

    // An empty path would send RemoveADirectory into the drive root.
    if (!m_TempPath.empty())
      itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

  void StrictWriteReferencesExactlyTheSourceInstances()
  {
    auto seg = this->BuildDerivedSeg();

    mitk::IFileWriter::Options options; // strict is the default
    const auto path = this->WriteSeg(seg, options, "strict");

    this->AssertReferencesExactlyTheFixture(path, "Strict");
  }

  void SyntheticWriteReferencesExactlyTheSourceInstances()
  {
    auto seg = this->BuildDerivedSeg();

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->WriteSeg(seg, options, "synthetic");

    // The upper bound carries this case: a real relation must suppress
    // synthesis outright, so a SEG that referenced the fixture's instances
    // and a minted series besides is the failure to catch here.
    this->AssertReferencesExactlyTheFixture(path, "Synthetic");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegSourceReference)
