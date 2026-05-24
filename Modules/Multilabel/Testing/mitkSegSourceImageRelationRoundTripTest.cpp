/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMSegPropertyTestHelpers.h"

#include <mitkBaseProperty.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkPropertyRelationRuleBase.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itksys/SystemTools.hxx>

#include <string>
#include <vector>

// Round-trip coverage for SegSourceImageRelationRule properties through
// each non-DICOM segmentation IO that MITK still ships. The DICOM SEG
// round-trip is exercised by mitkDICOMSegmentationIORegressionTest and
// mitkDICOMSegmentationIOWriterTest; this suite pins the remaining
// channels so a regression in the property serializer cannot silently
// strip rule connections.

namespace
{
  using Rule = mitk::SegSourceImageRelationRule;

  std::string DICOMKey(unsigned int g, unsigned int e)
  {
    return mitk::GeneratePropertyNameForDICOMTag(g, e);
  }

  // Build a single-label seg with one rule connection covering three
  // slices. Mirrors the construction pattern in the DICOM SEG writer
  // tests so the same assertions can target the rule's surface.
  mitk::MultiLabelSegmentation::Pointer BuildSegWithRule(
    const std::string& sourceSeriesUID,
    const std::vector<std::string>& sourceInstanceUIDs,
    const std::string& sourceClassUID = "1.2.840.10008.5.1.4.1.1.2")
  {
    auto geometryImage = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, static_cast<unsigned int>(sourceInstanceUIDs.size())};
    geometryImage->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);

    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(geometryImage);

    auto label = mitk::Label::New();
    label->SetName("L");
    label->SetValue(1);
    seg->AddLabel(label, 0, true, true);

    // Foreground voxel so the seg has writable frames in DICOM SEG paths.
    auto groupImage = seg->GetGroupImage(0);
    mitk::ImageWriteAccessor writeAccessor(groupImage);
    auto* pixels = static_cast<mitk::Label::PixelType*>(writeAccessor.GetData());
    const auto dims = groupImage->GetDimensions();
    const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
    for (unsigned int z = 0; z < dims[2]; ++z)
      pixels[z * sliceSize] = 1;

    auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
    auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
    for (std::size_t i = 0; i < sourceInstanceUIDs.size(); ++i)
    {
      perSliceInstance->SetValue(0, static_cast<int>(i), sourceInstanceUIDs[i]);
      perSliceClass->SetValue(0, static_cast<int>(i), sourceClassUID);
    }
    auto provider = mitk::PropertyList::New();
    provider->SetProperty(DICOMKey(0x0008, 0x0018).c_str(), perSliceInstance);
    provider->SetProperty(DICOMKey(0x0008, 0x0016).c_str(), perSliceClass);
    provider->SetProperty(DICOMKey(0x0020, 0x000e).c_str(),
                          mitk::TemporoSpatialStringProperty::New(sourceSeriesUID));

    auto rule = Rule::New();
    rule->Connect(seg, provider.GetPointer());

    return seg;
  }

  // Asserts the FULL observable state of a single rule connection after
  // a round-trip. Goes beyond "GetSourceImageRelations returns something"
  // — that high-level enumeration is reconstructed from a subset of the
  // properties the rule writes, so a property-list serializer that drops
  // sibling sub-properties (purpose tag Code Meaning, for example) would
  // still let GetSourceImageRelations succeed. This helper probes every
  // property the rule materialises so a drop-on-serialize regression
  // surfaces here rather than at a downstream tag-tree consumer.
  void AssertRelationPreserved(const mitk::MultiLabelSegmentation* loaded,
                               const std::string& expectedSeriesUID,
                               const std::vector<std::string>& expectedInstanceUIDs,
                               const std::string& expectedClassUID)
  {
    const auto relations = Rule::GetSourceImageRelations(loaded);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Round-trip must preserve exactly one rule connection",
                                 std::size_t{1}, relations.size());
    const auto& relation = relations.front();

    CPPUNIT_ASSERT_MESSAGE("Round-trip must preserve the relation UID",
                           !relation.relationUID.empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source series UID must round-trip",
                                 expectedSeriesUID, relation.sourceSeriesInstanceUID);

    CPPUNIT_ASSERT_MESSAGE("Per-slice instance UID property must round-trip non-null",
                           relation.instanceUIDsPerSlice.IsNotNull());
    const auto slices = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Per-slice instance-UID cardinality must round-trip",
                                 expectedInstanceUIDs.size(), slices.size());
    for (std::size_t i = 0; i < expectedInstanceUIDs.size(); ++i)
    {
      const auto value = relation.instanceUIDsPerSlice->GetValue(0, static_cast<int>(i));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Per-slice instance UID content must round-trip",
                                   expectedInstanceUIDs[i], value);
    }

    // classUIDsPerSlice must also survive with the same cardinality and
    // content. The pre-sharpening shape only checked IsNotNull, which
    // would have let a serializer collapse the per-slice content to a
    // single (slice 0) entry without the test noticing.
    CPPUNIT_ASSERT_MESSAGE("Per-slice class UID property must round-trip non-null",
                           relation.classUIDsPerSlice.IsNotNull());
    const auto classSlices = relation.classUIDsPerSlice->GetAvailableSlices(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Per-slice class-UID cardinality must round-trip",
                                 expectedInstanceUIDs.size(), classSlices.size());
    for (std::size_t i = 0; i < expectedInstanceUIDs.size(); ++i)
    {
      const auto value = relation.classUIDsPerSlice->GetValue(0, static_cast<int>(i));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Per-slice class UID content must round-trip",
                                   expectedClassUID, value);
    }

    // Purpose-of-Reference Code Sequence Code Meaning is written by the
    // base SourceImageRelationRule::Connect_datalayer under the rule's
    // own lowercase key "0040.a170". The .mitk persistence path
    // normalises the element id to uppercase via
    // DICOMTagPathToPersistenceNameTemplate (std::uppercase), landing
    // at "0040.A170". The stack format serializes property names
    // verbatim through JSON, so it preserves the rule's original
    // lowercase form. Both forms are valid round-trip outputs from
    // their respective IO paths; the test accepts either.
    const std::string upperKey = "DICOM.0008.2112.[0].0040.A170.[0].0008.0104";
    const std::string lowerKey = "DICOM.0008.2112.[0].0040.a170.[0].0008.0104";
    auto purposeProp = loaded->GetConstProperty(upperKey);
    if (purposeProp.IsNull())
      purposeProp = loaded->GetConstProperty(lowerKey);
    CPPUNIT_ASSERT_MESSAGE("Purpose-of-Reference Code Meaning property "
                           "must round-trip non-null at either case form",
                           purposeProp.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Purpose-of-Reference Code Meaning must carry the "
                           "canonical seg-source purpose tag",
                           mitk::test::PropertyScalarValueEquals(
                             purposeProp.GetPointer(),
                             Rule::CanonicalPurposeTag()));
  }
}

class mitkSegSourceImageRelationRoundTripTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSegSourceImageRelationRoundTripTestSuite);
  MITK_TEST(StackWriterReaderPreservesRule);
  MITK_TEST(NativeMitkIoPreservesRule);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

public:
  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-SegRelationRoundTrip-XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

  void StackWriterReaderPreservesRule()
  {
    const std::string seriesUID = "1.2.826.0.1.3680043.10.999.20.1";
    const std::vector<std::string> instanceUIDs = {
      "1.2.826.0.1.3680043.10.999.20.2.0",
      "1.2.826.0.1.3680043.10.999.20.2.1",
      "1.2.826.0.1.3680043.10.999.20.2.2"
    };
    auto seg = BuildSegWithRule(seriesUID, instanceUIDs);

    const auto path = m_TempPath + mitk::IOUtil::GetDirectorySeparator() + "stack.mitklabel.json";
    mitk::IOUtil::Save(seg, path);

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Stack reader yields exactly one BaseData", loaded.size() == 1);
    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Stack reader yields a MultiLabelSegmentation", loadedSeg != nullptr);

    AssertRelationPreserved(loadedSeg, seriesUID, instanceUIDs, "1.2.840.10008.5.1.4.1.1.2");
  }

  void NativeMitkIoPreservesRule()
  {
    const std::string seriesUID = "1.2.826.0.1.3680043.10.999.22.1";
    const std::vector<std::string> instanceUIDs = {
      "1.2.826.0.1.3680043.10.999.22.2.0",
      "1.2.826.0.1.3680043.10.999.22.2.1",
      "1.2.826.0.1.3680043.10.999.22.2.2"
    };
    auto seg = BuildSegWithRule(seriesUID, instanceUIDs);

    const auto path = m_TempPath + mitk::IOUtil::GetDirectorySeparator() + "native.nrrd";
    mitk::IOUtil::Save(seg, path);

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE(".mitk reader yields exactly one BaseData", loaded.size() == 1);
    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE(".mitk reader yields a MultiLabelSegmentation", loadedSeg != nullptr);

    AssertRelationPreserved(loadedSeg, seriesUID, instanceUIDs, "1.2.840.10008.5.1.4.1.1.2");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSegSourceImageRelationRoundTrip)
