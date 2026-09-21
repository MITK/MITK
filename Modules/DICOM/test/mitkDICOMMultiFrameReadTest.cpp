/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMMultiFrameTestObject.h"

#include <mitkDICOMIOMetaInformationPropertyConstants.h>
#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkIOMetaInformationPropertyConstants.h>
#include <mitkIOUtil.h>
#include <mitkIOVolumeSplitReason.h>
#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkPropertyKeyPath.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <nlohmann/json.hpp>

#include <set>

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itksys/SystemTools.hxx>

/**
 * \brief Covers the per-frame read model for multi-frame objects with
 *        functional groups.
 *
 * MITK-Data holds no object with a Per-Frame Functional Groups Sequence, so
 * every input is generated (\c DICOMMultiFrameTestObject). The suite asserts
 * that a per-frame attribute reaches the (t, z) slot its own pixels occupy,
 * under a key relative to the functional-group item, whether the encoder put
 * the macro in the shared or in the per-frame group.
 *
 * The functional-group tags of interest are registered by the suite itself.
 * They are not part of the default registry, so without this a scan would not
 * look for them and every assertion here would grade an empty property set.
 */
class mitkDICOMMultiFrameReadTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMMultiFrameReadTestSuite);

  MITK_TEST(PerSliceFilesAndSOPInstanceUID);
  MITK_TEST(PerFrameRescaleIsOneFrameRelativeProperty);
  MITK_TEST(SharedRescaleIsOneFrameRelativeProperty);
  MITK_TEST(UniformPerFrameRescaleIsOneFrameRelativeProperty);
  MITK_TEST(FrameReferenceDateTimeReachesEverySlot);
  MITK_TEST(SharedAndPerFrameAttributesBothReachEverySlot);
  MITK_TEST(ValuesFollowPixelsWhenInStackPositionDescends);
  MITK_TEST(SourceFramePropertyNamesTheFrameOfEverySlot);
  MITK_TEST(PixelsUsePerFrameRescale);
  MITK_TEST(NonIntegralInterceptLoadsAsDouble);
  MITK_TEST(TopLevelValuesAreUniformAcrossSlices);
  MITK_TEST(PerFrameValuesSurviveSaveAndReload);
  MITK_TEST(PlainMultiFrameIsNotExpanded);
  MITK_TEST(SingleFrameWithoutGroupsIsUnchanged);
  MITK_TEST(SingleFrameEnhancedGetsFrameRelativeKeys);
  MITK_TEST(TopLevelDuplicateLosesAgainstTheFrame);
  MITK_TEST(RaggedObjectKeepsTheOneFrameModel);
  MITK_TEST(TwoEnhancedFilesInOneSeriesBecomeTwoCompleteVolumes);
  MITK_TEST(EnhancedFilesWithTopLevelGeometryAreSeparated);
  MITK_TEST(EnhancedFileIsSeparatedFromSingleFrameFiles);
  MITK_TEST(SingleFrameEnhancedFilesStillStackIntoOneVolume);
  MITK_TEST(FrameModelFilesAreNotCondensedInto3DnT);
  MITK_TEST(MixedDirectoryLoadsEveryKindCompletely);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempDir;
  mitk::IDICOMTagsOfInterest* m_TagsOfInterest = nullptr;
  std::vector<mitk::DICOMTagPath> m_RegisteredTags;
  unsigned int m_CaseCounter = 0;

  static constexpr unsigned int FRAME_COUNT = 6;

  /** DICOM pads a string value to even length, and DCMTK reports the padding,
      so a value is compared without it. */
  static std::string Trimmed(const std::string& value)
  {
    const auto end = value.find_last_not_of(" \0");
    return std::string::npos == end ? std::string() : value.substr(0, end + 1);
  }

  static mitk::DICOMTagPath Rooted(unsigned int rootGroup,
                                   unsigned int rootElement,
                                   unsigned int macroGroup,
                                   unsigned int macroElement,
                                   unsigned int leafGroup,
                                   unsigned int leafElement)
  {
    mitk::DICOMTagPath path;
    path.AddAnySelection(rootGroup, rootElement);
    path.AddAnySelection(macroGroup, macroElement);
    path.AddElement(leafGroup, leafElement);
    return path;
  }

  /** The published key shape: the attribute's path relative to the
      functional-group item, with the macro's own item index kept. */
  static mitk::DICOMTagPath FrameRelative(unsigned int macroGroup,
                                          unsigned int macroElement,
                                          unsigned int leafGroup,
                                          unsigned int leafElement)
  {
    mitk::DICOMTagPath path;
    path.AddAnySelection(macroGroup, macroElement);
    path.AddElement(leafGroup, leafElement);
    return path;
  }

  static mitk::DICOMTagPath RescaleSlopeRelative() { return FrameRelative(0x0028, 0x9145, 0x0028, 0x1053); }
  static mitk::DICOMTagPath FrameReferenceDateTimeRelative() { return FrameRelative(0x0020, 0x9111, 0x0018, 0x9151); }

  void Register(const mitk::DICOMTagPath& path)
  {
    m_TagsOfInterest->AddTagOfInterest(path);
    m_RegisteredTags.push_back(path);
  }

  /** A directory of its own per case: loading a file pulls in every DICOM
      file of its directory before filtering by series. */
  std::string CaseDir()
  {
    const std::string path = m_TempDir + "/case" + std::to_string(m_CaseCounter++);
    itksys::SystemTools::MakeDirectory(path);
    return path;
  }

  std::vector<mitk::Image::Pointer> LoadAll(const std::string& path)
  {
    mitk::PreferenceListReaderOptionsFunctor readerFunctor({"MITK DICOM Reader v2 (autoselect)"}, {""});
    std::vector<mitk::Image::Pointer> result;

    for (const auto& data : mitk::IOUtil::Load(path, &readerFunctor))
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(data.GetPointer());
      CPPUNIT_ASSERT_MESSAGE("Loaded data is an image", image.IsNotNull());
      result.push_back(image);
    }

    return result;
  }

  mitk::Image::Pointer LoadOne(const std::string& path)
  {
    const auto loaded = this->LoadAll(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Generated object loads as exactly one image", std::size_t(1), loaded.size());
    return loaded.front();
  }

  /** The split reason the reader recorded on the block an image came from. */
  static mitk::IOVolumeSplitReason::Pointer SplitReasonOf(const mitk::Image* image)
  {
    const auto reason = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::VOLUME_SPLIT_REASON()));
    CPPUNIT_ASSERT_MESSAGE("The image carries a split reason", reason.IsNotNull());

    return mitk::IOVolumeSplitReason::FromJSON(nlohmann::json::parse(reason->GetValueAsString()));
  }

  static const mitk::TemporoSpatialStringProperty* AsDICOMProperty(const mitk::BaseProperty* property)
  {
    return dynamic_cast<const mitk::TemporoSpatialStringProperty*>(property);
  }

  /** The single property the given path must resolve to, with its published
      name checked, so a test cannot pass on a differently shaped key. */
  mitk::BaseProperty::ConstPointer TheOnlyProperty(const mitk::Image* image,
                                                   const mitk::DICOMTagPath& path,
                                                   const std::string& expectedName)
  {
    const auto matches = mitk::GetPropertyByDICOMTagPath(image, path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exactly one property for " + path.ToStr(), std::size_t(1), matches.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Published property name", expectedName, matches.begin()->first);
    return matches.begin()->second;
  }

  void AssertNoRootedKeys(const mitk::Image* image)
  {
    for (const auto& key : image->GetPropertyKeys())
    {
      CPPUNIT_ASSERT_MESSAGE("No property key is rooted in a functional-group sequence: " + key,
                             key.rfind("DICOM.5200.", 0) != 0);
    }
  }

  template <typename TPixel>
  TPixel PixelAt(const mitk::Image* image, unsigned int z) const
  {
    mitk::ImagePixelReadAccessor<TPixel, 3> accessor(image);
    const itk::Index<3> index = { { 0, 0, static_cast<itk::IndexValueType>(z) } };
    return accessor.GetPixelByIndex(index);
  }

  static std::string FrameTime(unsigned int k)
  {
    // Distinct per frame and ordered, so a value read at a slot names its frame.
    return "2025010111" + std::string(k < 10 ? "0" : "") + std::to_string(k) + "00.000000+0100";
  }

  mitk::DICOMMultiFrameTestObject MakeEnhanced(unsigned int frameCount = FRAME_COUNT) const
  {
    auto object = mitk::DICOMMultiFrameTestObject::EnhancedPET(frameCount);
    for (unsigned int k = 0; k < frameCount; ++k)
    {
      object.frames[k].frameReferenceDateTime = FrameTime(k);
    }
    return object;
  }

public:
  void setUp() override
  {
    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkDICOMMultiFrameReadTestXXXXXX");

    auto references = us::GetModuleContext()->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the DICOM tags-of-interest service is registered",
                           !references.empty());
    m_TagsOfInterest = us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(references.front());
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the tags-of-interest service resolves",
                           nullptr != m_TagsOfInterest);

    // Both roots of each attribute: the reader must publish one key whichever
    // group the encoder used, which is only testable if both are scanned.
    this->Register(Rooted(0x5200, 0x9230, 0x0028, 0x9145, 0x0028, 0x1053));
    this->Register(Rooted(0x5200, 0x9229, 0x0028, 0x9145, 0x0028, 0x1053));
    this->Register(Rooted(0x5200, 0x9230, 0x0028, 0x9145, 0x0028, 0x1052));
    this->Register(Rooted(0x5200, 0x9229, 0x0028, 0x9145, 0x0028, 0x1052));
    this->Register(Rooted(0x5200, 0x9230, 0x0020, 0x9111, 0x0018, 0x9151));
    this->Register(Rooted(0x5200, 0x9230, 0x0020, 0x9111, 0x0020, 0x9057));
  }

  void tearDown() override
  {
    for (const auto& path : m_RegisteredTags)
    {
      m_TagsOfInterest->RemoveTag(path);
    }
    m_RegisteredTags.clear();
    m_TagsOfInterest = nullptr;

    if (!m_TempDir.empty())
    {
      itksys::SystemTools::RemoveADirectory(m_TempDir);
      m_TempDir.clear();
    }
  }

  /** (a) One frame info per frame, so every per-slice property has a slot per
      slice, all naming the one file the frames came from. */
  void PerSliceFilesAndSOPInstanceUID()
  {
    const auto image = this->LoadOne(this->MakeEnhanced().Write(this->CaseDir(), "enhanced.dcm"));

    const auto files = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES()));
    CPPUNIT_ASSERT_MESSAGE("READER_FILES is present", files.IsNotNull());
    const auto* filesProperty = AsDICOMProperty(files);
    CPPUNIT_ASSERT_MESSAGE("READER_FILES is a temporo-spatial property", nullptr != filesProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One READER_FILES entry per slice",
                                 std::size_t(FRAME_COUNT), filesProperty->GetAvailableSlices(0).size());

    const std::string first = filesProperty->GetValue(0, 0, false, false);
    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every slot names the same file",
                                   first, filesProperty->GetValue(0, z, false, false));
    }

    const auto sopInstanceUIDs = image->GetConstProperty("dicom.image.0008.0018");
    CPPUNIT_ASSERT_MESSAGE("Per-slice SOP Instance UID property is present", sopInstanceUIDs.IsNotNull());
  }

  /** (b) Per-frame placement: one property, frame-relative key, frame z's
      value at slot (0, z). */
  void PerFrameRescaleIsOneFrameRelativeProperty()
  {
    auto object = this->MakeEnhanced();
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = 1.0 + k;
    }

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced.dcm"));

    const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Frame " + std::to_string(z) + "'s slope at its own slot",
                                   mitk::ConvertValueToDICOMStr(1.0 + z),
                                   Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }

    this->AssertNoRootedKeys(image);
  }

  /** (b) Shared placement: the same key, the shared value repeated per slot. */
  void SharedRescaleIsOneFrameRelativeProperty()
  {
    auto object = this->MakeEnhanced();
    object.rescaleInSharedGroup = true;
    object.frames.front().slope = 7.0;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced.dcm"));

    const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Shared rescale slope is a temporo-spatial property", nullptr != dicomProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Shared value repeated at slot " + std::to_string(z),
                                   mitk::ConvertValueToDICOMStr(7.0),
                                   Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }

    this->AssertNoRootedKeys(image);
  }

  /** (b) One value repeated at every per-frame item: still one property, and
      still one value per slot rather than a collapsed single value. */
  void UniformPerFrameRescaleIsOneFrameRelativeProperty()
  {
    const auto image = this->LoadOne(this->MakeEnhanced().Write(this->CaseDir(), "enhanced.dcm"));

    const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Uniform rescale slope is a temporo-spatial property", nullptr != dicomProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One slot per frame even when the value is uniform",
                                 std::size_t(FRAME_COUNT), dicomProperty->GetAvailableSlices(0).size());
  }

  /** (c) The same for an attribute of another macro. */
  /**
   * One file carrying a shared attribute and a per-frame one at the same time.
   *
   * The value store answers a frame-scoped query from that frame's entries and
   * from the frame-independent ones together. Narrowing to the frame must not
   * hide the shared value, and widening to the shared value must not let a
   * neighbouring frame's entry through.
   */
  void SharedAndPerFrameAttributesBothReachEverySlot()
  {
    auto object = this->MakeEnhanced();
    object.rescaleInSharedGroup = true;
    object.frames.front().slope = 7.0;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "shared_and_per_frame.dcm"));

    const auto sharedProperty =
      this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* sharedDicomProperty = AsDICOMProperty(sharedProperty);
    CPPUNIT_ASSERT_MESSAGE("The shared rescale is a temporo-spatial property", nullptr != sharedDicomProperty);

    const auto timeProperty =
      this->TheOnlyProperty(image, FrameReferenceDateTimeRelative(), "DICOM.0020.9111.[0].0018.9151");
    const auto* timeDicomProperty = AsDICOMProperty(timeProperty);
    CPPUNIT_ASSERT_MESSAGE("Frame Reference DateTime is a temporo-spatial property", nullptr != timeDicomProperty);

    const std::string sharedSlope = Trimmed(sharedDicomProperty->GetValue(0, 0, false, false));
    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The shared value answers at every slot",
                                   sharedSlope, Trimmed(sharedDicomProperty->GetValue(0, z, false, false)));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The per-frame value stays at its own slot",
                                   FrameTime(z), Trimmed(timeDicomProperty->GetValue(0, z, false, false)));
    }
  }

  void FrameReferenceDateTimeReachesEverySlot()
  {
    const auto image = this->LoadOne(this->MakeEnhanced().Write(this->CaseDir(), "enhanced.dcm"));

    const auto property =
      this->TheOnlyProperty(image, FrameReferenceDateTimeRelative(), "DICOM.0020.9111.[0].0018.9151");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Frame Reference DateTime is a temporo-spatial property", nullptr != dicomProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Frame " + std::to_string(z) + "'s time at its own slot",
                                   FrameTime(z), Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }
  }

  /** (c) The mapping invariant: the value at a slot and the pixels at that
      slot come from one frame. Asserted against the pixel constant rather
      than against a z number, so a future reordering that moves pixels
      without values, or the reverse, fails here. In-Stack Position descends
      while the plane positions ascend, so an implementation that keys on the
      dimension index instead of on the item index cannot pass. */
  void ValuesFollowPixelsWhenInStackPositionDescends()
  {
    auto object = this->MakeEnhanced();
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].inStackPosition = FRAME_COUNT - k;
    }

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced.dcm"));

    const auto property =
      this->TheOnlyProperty(image, FrameReferenceDateTimeRelative(), "DICOM.0020.9111.[0].0018.9151");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Frame Reference DateTime is a temporo-spatial property", nullptr != dicomProperty);

    for (unsigned int z = 0; z < image->GetDimension(2); ++z)
    {
      const auto constant = this->PixelAt<short>(image, z);
      const auto frame = std::find_if(object.frames.cbegin(), object.frames.cend(),
                                      [constant](const mitk::DICOMMultiFrameTestFrame& candidate)
                                      { return candidate.constant == constant; });
      CPPUNIT_ASSERT_MESSAGE("Slice " + std::to_string(z) + " carries a frame's constant",
                             frame != object.frames.cend());
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Value at slot " + std::to_string(z) + " belongs to the frame whose pixels fill it",
                                   frame->frameReferenceDateTime,
                                   Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }
  }

  /** (i) The stored frame index per slot, so the file-to-image mapping stays
      recoverable from the loaded image. */
  void SourceFramePropertyNamesTheFrameOfEverySlot()
  {
    const auto image = this->LoadOne(this->MakeEnhanced().Write(this->CaseDir(), "enhanced.dcm"));

    const auto frames = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("The per-slot source frame property is present", frames.IsNotNull());
    const auto* framesProperty = AsDICOMProperty(frames);
    CPPUNIT_ASSERT_MESSAGE("The source frame property is temporo-spatial", nullptr != framesProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Slot " + std::to_string(z) + " names its stored frame",
                                   std::to_string(z), Trimmed(framesProperty->GetValue(0, z, false, false)));
    }
  }

  /** (d) Each frame's own Pixel Value Transformation reaches its pixels, and
      the component type is the one GDCM's rule gives for the widest pair. */
  void PixelsUsePerFrameRescale()
  {
    auto object = this->MakeEnhanced();
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = (k % 2 == 0) ? 4.0 : 3.0;
    }

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced.dcm"));

    // Stored range for 16 bits signed times slope 4 exceeds int16, and both
    // pairs are integral, so GDCM's smallest-integer-that-fits rule gives int.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Component type follows GDCM's rule over all frames' pairs",
                                 std::string("int"), image->GetPixelType().GetComponentTypeAsString());

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      const double expected = object.frames[z].constant * object.frames[z].slope + object.frames[z].intercept;
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Slice " + std::to_string(z) + " uses its own frame's rescale",
                                   static_cast<int>(expected), this->PixelAt<int>(image, z));
    }
  }

  /** (d) A non-integral pair makes GDCM's rule pick double, and the
      correction must not narrow it. */
  void NonIntegralInterceptLoadsAsDouble()
  {
    auto object = this->MakeEnhanced();
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = (k % 2 == 0) ? 4.0 : 3.0;
      object.frames[k].intercept = -10.5;
    }

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced.dcm"));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A non-integral pair loads as double",
                                 std::string("double"), image->GetPixelType().GetComponentTypeAsString());

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      const double expected = object.frames[z].constant * object.frames[z].slope + object.frames[z].intercept;
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Slice " + std::to_string(z) + " uses its own frame's rescale",
                                           expected, this->PixelAt<double>(image, z), 1e-9);
    }
  }

  /** (e) A top-level attribute has one value for the whole image, whatever
      the frame model does. */
  void TopLevelValuesAreUniformAcrossSlices()
  {
    const auto image = this->LoadOne(this->MakeEnhanced().Write(this->CaseDir(), "enhanced.dcm"));

    const auto rows = mitk::GetFirstDICOMValueAsString(image, mitk::DICOMTagPath(0x0028, 0x0010));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Rows is the object's row count", std::string("8"), rows);

    const auto modality = mitk::GetFirstDICOMValueAsString(image, mitk::DICOMTagPath(0x0008, 0x0060));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Modality is uniform", std::string("PT"), modality);
  }

  /** (f) The published keys are persisted, so a round trip keeps the per-slot
      values. A property without persistence info is dropped on save without a
      log line, which is exactly what this guards. */
  void PerFrameValuesSurviveSaveAndReload()
  {
    auto object = this->MakeEnhanced();
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = 1.0 + k;
    }

    const std::string directory = this->CaseDir();
    const auto image = this->LoadOne(object.Write(directory, "enhanced.dcm"));

    const std::string nrrd = directory + "/roundtrip.nrrd";
    mitk::IOUtil::Save(image, nrrd);
    const auto reloaded = mitk::IOUtil::Load<mitk::Image>(nrrd);
    CPPUNIT_ASSERT_MESSAGE("The saved image reloads", reloaded.IsNotNull());

    const auto slopes =
      this->TheOnlyProperty(reloaded, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* slopeProperty = AsDICOMProperty(slopes);
    CPPUNIT_ASSERT_MESSAGE("Rescale slope survives as a temporo-spatial property", nullptr != slopeProperty);

    const auto times =
      this->TheOnlyProperty(reloaded, FrameReferenceDateTimeRelative(), "DICOM.0020.9111.[0].0018.9151");
    const auto* timeProperty = AsDICOMProperty(times);
    CPPUNIT_ASSERT_MESSAGE("Frame Reference DateTime survives", nullptr != timeProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Slope at slot " + std::to_string(z) + " survives",
                                   mitk::ConvertValueToDICOMStr(1.0 + z),
                                   Trimmed(slopeProperty->GetValue(0, z, false, false)));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Frame time at slot " + std::to_string(z) + " survives",
                                   FrameTime(z), Trimmed(timeProperty->GetValue(0, z, false, false)));
    }

    const auto frames = reloaded->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("The per-slot source frame property survives", frames.IsNotNull());
  }

  /** (g) A multi-frame object without functional groups gets no frame model:
      one entry in READER_FILES and the pixels it always had. */
  void PlainMultiFrameIsNotExpanded()
  {
    auto object = this->MakeEnhanced();
    object.functionalGroups = false;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "plain.dcm"));

    const auto files = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES()));
    const auto* filesProperty = AsDICOMProperty(files);
    CPPUNIT_ASSERT_MESSAGE("READER_FILES is a temporo-spatial property", nullptr != filesProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A plain multi-frame object keeps one frame info",
                                 std::size_t(1), filesProperty->GetAvailableSlices(0).size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("All frames still load",
                                 FRAME_COUNT, image->GetDimension(2));
    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Slice " + std::to_string(z) + " keeps its pixels",
                                   object.frames[z].constant, this->PixelAt<short>(image, z));
    }

    const auto frames = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("No source frame property without a frame model", frames.IsNull());
  }

  /** (h) A single-frame file without functional groups is untouched. */
  void SingleFrameWithoutGroupsIsUnchanged()
  {
    auto object = this->MakeEnhanced(1);
    object.functionalGroups = false;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "single.dcm"));

    const auto files = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES()));
    const auto* filesProperty = AsDICOMProperty(files);
    CPPUNIT_ASSERT_MESSAGE("READER_FILES is a temporo-spatial property", nullptr != filesProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One entry for one file",
                                 std::size_t(1), filesProperty->GetAvailableSlices(0).size());

    const auto frames = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("No source frame property without a frame model", frames.IsNull());
  }

  /** (h) A single-frame object that does have functional groups gets the
      frame model, so every such object presents its values the same way. */
  void SingleFrameEnhancedGetsFrameRelativeKeys()
  {
    auto object = this->MakeEnhanced(1);
    object.frames.front().slope = 5.0;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "enhanced_single.dcm"));

    const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The one frame's slope at its one slot",
                                 mitk::ConvertValueToDICOMStr(5.0), Trimmed(dicomProperty->GetValue(0, 0, false, false)));

    this->AssertNoRootedKeys(image);

    const auto frames = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("A single-frame functional-group object still records its frame", frames.IsNotNull());
  }

  /** (j) A non-conformant file carrying one attribute both at the top level
      and in a functional group: the frame's value wins. */
  void TopLevelDuplicateLosesAgainstTheFrame()
  {
    // The collision only exists when the frame-relative form is scanned as a
    // top-level path of its own, which is what makes the two findings share
    // one published key.
    this->Register(RescaleSlopeRelative());

    auto object = this->MakeEnhanced();
    object.duplicateAtTopLevel = true;
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      object.frames[k].slope = 1.0 + k;
    }

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "duplicate.dcm"));

    const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);

    for (unsigned int z = 0; z < FRAME_COUNT; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The frame's value wins at slot " + std::to_string(z),
                                   mitk::ConvertValueToDICOMStr(1.0 + z),
                                   Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }
  }

  /** (k) A Per-Frame Functional Groups Sequence that does not describe every
      frame cannot be mapped to slots, so the file keeps the one-frame model
      and publishes rooted keys as it does today. */
  void RaggedObjectKeepsTheOneFrameModel()
  {
    auto object = this->MakeEnhanced();
    object.perFrameItemCountOverride = FRAME_COUNT - 1;

    const auto image = this->LoadOne(object.Write(this->CaseDir(), "ragged.dcm"));

    const auto files = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES()));
    const auto* filesProperty = AsDICOMProperty(files);
    CPPUNIT_ASSERT_MESSAGE("READER_FILES is a temporo-spatial property", nullptr != filesProperty);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A ragged object keeps one frame info",
                                 std::size_t(1), filesProperty->GetAvailableSlices(0).size());

    CPPUNIT_ASSERT_MESSAGE("A ragged object publishes no frame-relative key",
                           mitk::GetPropertyByDICOMTagPath(image, RescaleSlopeRelative()).empty());

    bool foundRootedKey = false;
    for (const auto& key : image->GetPropertyKeys())
    {
      foundRootedKey |= key.rfind("DICOM.5200.", 0) == 0;
    }
    CPPUNIT_ASSERT_MESSAGE("A ragged object keeps the rooted keys", foundRootedKey);

    const auto frames = image->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
    CPPUNIT_ASSERT_MESSAGE("No source frame property without a frame model", frames.IsNull());
  }

  /** Writes \p count objects of one series into one directory, applying
      \p configure to each before it is written. */
  template <typename TConfigure>
  std::string WriteSeries(unsigned int count, TConfigure configure)
  {
    const std::string directory = this->CaseDir();
    const std::string studyUID = "1.2.826.0.1.3680043.8.498.90000" + std::to_string(m_CaseCounter) + ".1";
    const std::string seriesUID = "1.2.826.0.1.3680043.8.498.90000" + std::to_string(m_CaseCounter) + ".2";

    for (unsigned int file = 0; file < count; ++file)
    {
      auto object = this->MakeEnhanced();
      object.studyInstanceUID = studyUID;
      object.seriesInstanceUID = seriesUID;
      object.instanceNumber = file + 1;
      configure(object, file);
      object.Write(directory, "file_" + std::to_string(file) + ".dcm");
    }

    return directory;
  }

  /** (l) Two conformant functional-group files of one series.
   *
   * They reach the reader as two blocks whatever the frame model does, because
   * a conformant object carries Image Position (Patient) in the Plane Position
   * macro and the sorters read the top level only. What this pins is the
   * outcome that matters: two complete volumes, each with its own per-frame
   * values, rather than one volume of depth two.
   */
  void TwoEnhancedFilesInOneSeriesBecomeTwoCompleteVolumes()
  {
    const std::string directory = this->WriteSeries(2, [](mitk::DICOMMultiFrameTestObject& object, unsigned int file)
    {
      object.zOffset = file * FRAME_COUNT * object.sliceSpacing;
      for (auto& frame : object.frames)
      {
        frame.slope = 1.0 + file;
      }
    });

    const auto images = this->LoadAll(directory);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two multi-frame files yield two volumes, not one of depth two",
                                 std::size_t(2), images.size());

    std::set<std::string> slopes;
    for (const auto& image : images)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Each file loads as the complete volume it is",
                                   FRAME_COUNT, image->GetDimension(2));

      const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
      const auto* dicomProperty = AsDICOMProperty(property);
      CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);

      // One file's slope at every one of its slots, so neither volume borrowed
      // the other's per-frame values.
      const std::string slope = Trimmed(dicomProperty->GetValue(0, 0, false, false));
      for (unsigned int z = 0; z < FRAME_COUNT; ++z)
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("One file's slope at all of its slots",
                                     slope, Trimmed(dicomProperty->GetValue(0, z, false, false)));
      }
      slopes.insert(slope);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The two volumes carry the two files' distinct slopes",
                                 std::size_t(2), slopes.size());
  }

  /** (l) The separation itself, on the only input that reaches it.
   *
   * A file that carries both top-level geometry and per-frame functional groups
   * is non-conformant but does occur, and it is the one case the sorters put
   * into a block with another file. Without the separation ITK's multi-file
   * branch would set the moving dimension to the file count and yield one
   * volume of depth two, losing every frame but one per file.
   */
  void EnhancedFilesWithTopLevelGeometryAreSeparated()
  {
    const std::string directory = this->WriteSeries(2, [](mitk::DICOMMultiFrameTestObject& object, unsigned int file)
    {
      object.topLevelGeometry = true;
      object.zOffset = file * FRAME_COUNT * object.sliceSpacing;
      for (auto& frame : object.frames)
      {
        frame.slope = 1.0 + file;
      }
    });

    const auto images = this->LoadAll(directory);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The block is separated into one volume per file",
                                 std::size_t(2), images.size());

    for (const auto& image : images)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Each file loads as the complete volume it is",
                                   FRAME_COUNT, image->GetDimension(2));
      CPPUNIT_ASSERT_MESSAGE("The separation is recorded on every resulting block",
                             SplitReasonOf(image)->HasReason(
                               mitk::IOVolumeSplitReason::ReasonType::MultiFrameFileSeparated));
      CPPUNIT_ASSERT_MESSAGE("The slice-count net did not have to fire",
                             !SplitReasonOf(image)->HasReason(
                               mitk::IOVolumeSplitReason::ReasonType::FrameCountMismatch));
    }
  }

  /**
   * Single-frame functional-group instances of one series stack, as they did
   * before the frame model existed.
   *
   * The separation rule exists because ITK's multi-file branch keeps only one
   * frame per file. At one frame per file that is the correct result already, so
   * separating would turn an N-slice series into N single-slice volumes for no
   * gain. The frame model itself still applies: the values arrive under the
   * frame-relative key.
   */
  void SingleFrameEnhancedFilesStillStackIntoOneVolume()
  {
    const std::string directory = this->CaseDir();
    const std::string studyUID = "1.2.826.0.1.3680043.8.498.900041";
    const std::string seriesUID = "1.2.826.0.1.3680043.8.498.900042";
    constexpr unsigned int fileCount = 3;

    for (unsigned int file = 0; file < fileCount; ++file)
    {
      auto single = this->MakeEnhanced(1);
      single.topLevelGeometry = true;
      single.studyInstanceUID = studyUID;
      single.seriesInstanceUID = seriesUID;
      single.instanceNumber = file + 1;
      single.zOffset = file * single.sliceSpacing;
      single.frames.front().slope = 3.0;
      single.Write(directory, "single_enhanced_" + std::to_string(file) + ".dcm");
    }

    const auto images = this->LoadAll(directory);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Single-frame functional-group files are not separated",
                                 std::size_t(1), images.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every file contributes its one slice",
                                 fileCount, images.front()->GetDimension(2));
    // A volume that was never split may carry no reason property at all, so the
    // absence of the property counts as the absence of the reason.
    const auto reason = images.front()->GetConstProperty(
      mitk::PropertyKeyPathToPropertyName(mitk::IOMetaInformationPropertyConstants::VOLUME_SPLIT_REASON()));
    const bool separated =
      reason.IsNotNull()
      && mitk::IOVolumeSplitReason::FromJSON(nlohmann::json::parse(reason->GetValueAsString()))
           ->HasReason(mitk::IOVolumeSplitReason::ReasonType::MultiFrameFileSeparated);
    CPPUNIT_ASSERT_MESSAGE("Stacking them is not reported as a multi-frame separation", !separated);

    const auto property =
      this->TheOnlyProperty(images.front(), RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
    const auto* dicomProperty = AsDICOMProperty(property);
    CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);
    for (unsigned int z = 0; z < fileCount; ++z)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every slot carries its own file's frame-relative value",
                                   mitk::ConvertValueToDICOMStr(3.0),
                                   Trimmed(dicomProperty->GetValue(0, z, false, false)));
    }
  }

  /**
   * A frame-model file must reach the base class unmerged.
   *
   * Condense3DBlocks runs before the frame expansion and emits its 3D+t outputs
   * itself, so a condensed block would skip the expansion, the per-frame Pixel
   * Value Transformation and the slice-count net entirely. Only a file that
   * carries top-level geometry can get that far, which is what this builds.
   */
  void FrameModelFilesAreNotCondensedInto3DnT()
  {
    const std::string directory = this->CaseDir();
    const std::string studyUID = "1.2.826.0.1.3680043.8.498.900061";
    const std::string seriesUID = "1.2.826.0.1.3680043.8.498.900062";

    // Same geometry, different time point: the shape Condense3DBlocks merges.
    for (unsigned int timePoint = 0; timePoint < 2; ++timePoint)
    {
      auto object = this->MakeEnhanced();
      object.topLevelGeometry = true;
      object.studyInstanceUID = studyUID;
      object.seriesInstanceUID = seriesUID;
      object.instanceNumber = timePoint + 1;
      object.zOffset = 0.0;
      for (auto& frame : object.frames)
      {
        frame.slope = 2.0 + timePoint;
      }
      object.Write(directory, "timepoint_" + std::to_string(timePoint) + ".dcm");
    }

    const auto images = this->LoadAll(directory);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Each frame-model file becomes its own volume rather than a time step",
                                 std::size_t(2), images.size());

    for (const auto& image : images)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("One time step, because the block was not condensed",
                                   std::size_t(1), std::size_t(image->GetTimeSteps()));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Every frame of the file reaches the volume",
                                   FRAME_COUNT, image->GetDimension(2));

      const auto frames = image->GetConstProperty(
        mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
      CPPUNIT_ASSERT_MESSAGE("The frame model survived, so the expansion ran", frames.IsNotNull());

      const auto property = this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
      CPPUNIT_ASSERT_MESSAGE("The per-frame value is published", property.IsNotNull());
    }
  }

  /** (l) A multi-frame functional-group file among single-frame files of one
   *  series.
   *
   * The sorters split the two kinds apart before the frame model sees them,
   * on a value difference, whatever this test does to make them agree on SOP
   * Class UID, Modality and top-level geometry. So this pins the outcome rather
   * than the mechanism: the multi-frame file still loads as the complete volume
   * it is, exactly one volume carries the frame model, and no frame is lost.
   * `ExpandAndSeparate`'s remainder branch stays a net for a block the sorters
   * do hand over mixed.
   */
  void EnhancedFileIsSeparatedFromSingleFrameFiles()
  {
    const std::string directory = this->CaseDir();
    const std::string studyUID = "1.2.826.0.1.3680043.8.498.900031";
    const std::string seriesUID = "1.2.826.0.1.3680043.8.498.900032";

    auto enhanced = this->MakeEnhanced();
    enhanced.topLevelGeometry = true;
    enhanced.studyInstanceUID = studyUID;
    enhanced.seriesInstanceUID = seriesUID;
    enhanced.instanceNumber = 1;
    enhanced.Write(directory, "enhanced.dcm");

    for (unsigned int file = 0; file < 3; ++file)
    {
      auto single = this->MakeEnhanced(1);
      single.functionalGroups = false;
      single.sopClassUID = "1.2.840.10008.5.1.4.1.1.130";
      single.modality = "PT";
      single.studyInstanceUID = studyUID;
      single.seriesInstanceUID = seriesUID;
      single.instanceNumber = file + 2;
      single.zOffset = (file + 1) * single.sliceSpacing;
      single.frames.front().constant = static_cast<std::int16_t>(200 + file);
      single.Write(directory, "single_" + std::to_string(file) + ".dcm");
    }

    const auto images = this->LoadAll(directory);

    unsigned int withFrameModel = 0;
    unsigned int totalSlices = 0;
    for (const auto& image : images)
    {
      totalSlices += image->GetDimension(2);

      const auto frames = image->GetConstProperty(
        mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));
      if (frames.IsNotNull())
      {
        ++withFrameModel;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The multi-frame file loads as the complete volume it is",
                                     FRAME_COUNT, image->GetDimension(2));
      }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exactly one volume has the frame model", 1u, withFrameModel);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Every frame of every file reaches a volume",
                                 FRAME_COUNT + 3u, totalSlices);
  }

  /** A directory holding a multi-frame object with functional groups, a classic
   *  single-frame series and a multi-frame object without functional groups:
   *  the shape a routine PACS export has.
   *
   *  The frame-model flag is cache wide, so one Enhanced file anywhere in the
   *  directory sends every block through the frame-aware path. The blocks that
   *  have no frame model must come out exactly as they would without it: every
   *  frame present, one entry per file in the files property, and no per-slot
   *  frame record. Nothing may be lost or silently merged.
   */
  void MixedDirectoryLoadsEveryKindCompletely()
  {
    const std::string directory = this->CaseDir();
    const std::string study = "1.2.826.0.1.3680043.8.498.900040";
    constexpr unsigned int classicCount = 4;

    for (unsigned int file = 0; file < classicCount; ++file)
    {
      auto single = this->MakeEnhanced(1);
      single.functionalGroups = false;
      single.studyInstanceUID = study;
      single.seriesInstanceUID = study + ".1";
      single.instanceNumber = file + 1;
      single.zOffset = file * single.sliceSpacing;
      single.frames.front().constant = static_cast<std::int16_t>(300 + file);
      single.Write(directory, "classic_" + std::to_string(file) + ".dcm");
    }

    auto enhanced = this->MakeEnhanced();
    enhanced.studyInstanceUID = study;
    enhanced.seriesInstanceUID = study + ".2";
    for (unsigned int k = 0; k < FRAME_COUNT; ++k)
    {
      enhanced.frames[k].slope = 1.0 + k;
    }
    enhanced.Write(directory, "enhanced.dcm");

    auto plain = this->MakeEnhanced();
    plain.functionalGroups = false;
    plain.studyInstanceUID = study;
    plain.seriesInstanceUID = study + ".3";
    plain.Write(directory, "plain.dcm");

    const auto images = this->LoadAll(directory);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Each of the three series becomes its own volume",
                                 std::size_t(3), images.size());

    unsigned int enhancedVolumes = 0;
    unsigned int plainVolumes = 0;
    unsigned int classicVolumes = 0;
    unsigned int totalSlices = 0;

    for (const auto& image : images)
    {
      totalSlices += image->GetDimension(2);

      const auto files = image->GetConstProperty(
        mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES()));
      const auto* filesProperty = AsDICOMProperty(files);
      CPPUNIT_ASSERT_MESSAGE("Every volume names its files", nullptr != filesProperty);

      const auto frames = image->GetConstProperty(
        mitk::PropertyKeyPathToPropertyName(mitk::DICOMIOMetaInformationPropertyConstants::READER_FRAMES()));

      if (frames.IsNotNull())
      {
        ++enhancedVolumes;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The functional-group object keeps all its frames",
                                     FRAME_COUNT, image->GetDimension(2));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("One files entry per frame",
                                     std::size_t(FRAME_COUNT), filesProperty->GetAvailableSlices(0).size());

        const auto property =
          this->TheOnlyProperty(image, RescaleSlopeRelative(), "DICOM.0028.9145.[0].0028.1053");
        const auto* dicomProperty = AsDICOMProperty(property);
        CPPUNIT_ASSERT_MESSAGE("Rescale slope is a temporo-spatial property", nullptr != dicomProperty);
        for (unsigned int z = 0; z < FRAME_COUNT; ++z)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Frame " + std::to_string(z) + "'s slope at its own slot",
                                       mitk::ConvertValueToDICOMStr(1.0 + z),
                                       Trimmed(dicomProperty->GetValue(0, z, false, false)));
        }
      }
      else if (1 == filesProperty->GetAvailableSlices(0).size())
      {
        ++plainVolumes;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The plain multi-frame object keeps all its frames",
                                     FRAME_COUNT, image->GetDimension(2));
        for (unsigned int z = 0; z < FRAME_COUNT; ++z)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Plain multi-frame slice " + std::to_string(z),
                                       static_cast<short>(100 + z), this->PixelAt<short>(image, z));
        }
      }
      else
      {
        ++classicVolumes;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("The classic series keeps all its slices",
                                     classicCount, image->GetDimension(2));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("One files entry per slice",
                                     std::size_t(classicCount), filesProperty->GetAvailableSlices(0).size());
        for (unsigned int z = 0; z < classicCount; ++z)
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Classic slice " + std::to_string(z),
                                       static_cast<short>(300 + z), this->PixelAt<short>(image, z));
        }
      }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("One volume has the frame model", 1u, enhancedVolumes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One volume is a plain multi-frame object", 1u, plainVolumes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One volume is the classic series", 1u, classicVolumes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No frame of any kind is lost",
                                 classicCount + 2u * FRAME_COUNT, totalSlices);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMMultiFrameRead)
