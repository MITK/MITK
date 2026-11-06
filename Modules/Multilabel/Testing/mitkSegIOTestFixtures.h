/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegIOTestFixtures_h
#define mitkSegIOTestFixtures_h

#include <mitkArbitraryTimeGeometry.h>
#include <mitkCoreServices.h>
#include <mitkDICOMCodeSequenceWithModifiers.h>
#include <mitkFileReaderRegistry.h>
#include <mitkFileReaderSelector.h>
#include <mitkIMimeTypeProvider.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkLog.h>
#include <mitkLogBackendBase.h>
#include <mitkProperties.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkSegTestSourceImageFactory.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

namespace mitk
{
  namespace test
  {
    struct RichSegmentationOptions
    {
      /** 4D with an ArbitraryTimeGeometry of at least 3 non-uniform time points whose voxels differ. */
      bool dynamic;
      /** Label names of at most 64 ASCII characters without backslash (DICOM SEG round trips). */
      bool dicomLegalNames;
      /** One additional label without voxels. */
      bool withEmptyLabel;
      /** One additional group without labels. */
      bool withEmptyGroup;
    };

    /** Name of the typed (IntProperty) custom property the rich fixture sets on the segmentation. */
    inline const std::string& RichFixtureCustomPropertyName()
    {
      static const std::string name = "mitk.test.custom";
      return name;
    }

    /** Name of the typed (IntProperty) custom property the rich fixture sets on every label. */
    inline const std::string& RichFixtureLabelCustomPropertyName()
    {
      static const std::string name = "mitk.test.label.custom";
      return name;
    }

    namespace detail
    {
      constexpr unsigned int RichSizeX = 10;
      constexpr unsigned int RichSizeY = 8;
      constexpr unsigned int RichSizeZ = 4;
      const std::string RichCodingScheme = "99MITKTEST";

      /** Inclusive index ranges. */
      struct VoxelBox
      {
        unsigned int x0, x1, y0, y1, z0, z1;
      };

      struct RichLabelSpec
      {
        Label::PixelType value;
        MultiLabelSegmentation::GroupIndexType group;
        std::string name;
        std::string dicomLegalName;
        std::array<float, 3> color;
        float opacity;
        bool visible;
        bool locked;
        Label::AlgorithmType algorithmType;
        /** Empty for a label without voxels. */
        std::vector<VoxelBox> boxes;
      };

      /**
       * Group 0 is unnamed, group 1 named. Values are neither contiguous nor
       * inserted in ascending order, and interleave across the groups. The
       * labels of one group never overlap; the two groups do.
       */
      inline std::vector<RichLabelSpec> RichLabelSpecs()
      {
        using AT = Label::AlgorithmType;
        return {
          { 7, 0, "Liver", "Liver", { 0.8f, 0.1f, 0.1f }, 0.1f, true, false, AT::MANUAL,
            { { 0, 2, 0, 3, 0, 1 } } },
          { 2, 0, "L\xC3\xA4sion \xCE\xB1", "Lesion alpha", { 0.1f, 0.7f, 0.2f }, 0.2f, false, true, AT::AUTOMATIC,
            { { 3, 5, 1, 4, 1, 2 } } },
          { 12, 0, "Quote \" back\\slash {braces}", "Quote \" {braces}", { 0.2f, 0.3f, 0.9f }, 0.3f, true, true,
            AT::SEMIAUTOMATIC, { { 6, 9, 2, 5, 2, 3 } } },
          { 30, 1, "A label name that is deliberately longer than the sixty-four characters DICOM allows",
            "A label name kept within the sixty-four characters DICOM allows", { 0.9f, 0.6f, 0.1f }, 0.4f, false,
            false, AT::MANUAL, { { 1, 4, 4, 7, 0, 3 } } },
          { 4, 1, "Kidney", "Kidney", { 0.5f, 0.2f, 0.6f }, 0.5f, true, false, AT::AUTOMATIC,
            { { 5, 7, 0, 2, 0, 2 } } },
          { 21, 1, "Spleen", "Spleen", { 0.1f, 0.6f, 0.6f }, 0.7f, false, true, AT::SEMIAUTOMATIC,
            { { 8, 9, 5, 7, 1, 3 } } },
        };
      }

      inline RichLabelSpec RichEmptyLabelSpec()
      {
        return { 40, 0, "Empty label", "Empty label", { 0.4f, 0.4f, 0.4f }, 0.9f, false, false, Label::AlgorithmType::MANUAL, {} };
      }

      inline DICOMCodeSequence RichCode(const std::string& value, const std::string& meaning)
      {
        return DICOMCodeSequence(value, RichCodingScheme, meaning);
      }

      inline DICOMCodeSequenceWithModifiers RichCodeWithModifiers(const std::string& value,
                                                                  const std::string& meaning,
                                                                  unsigned int modifierCount)
      {
        DICOMCodeSequenceWithModifiers code(value, RichCodingScheme, meaning);
        for (unsigned int i = 0; i < modifierCount; ++i)
        {
          const auto suffix = std::string(1, static_cast<char>('A' + i));
          code.AddModifier(RichCode(value + "M" + suffix, meaning + " modifier " + suffix));
        }
        return code;
      }

      inline Label::Pointer BuildRichLabel(const RichLabelSpec& spec, bool dicomLegalNames)
      {
        const auto v = std::to_string(spec.value);

        auto label = Label::New(spec.value, dicomLegalNames ? spec.dicomLegalName : spec.name);
        Color color;
        color.Set(spec.color[0], spec.color[1], spec.color[2]);
        label->SetColor(color);
        label->SetOpacity(spec.opacity);
        label->SetVisible(spec.visible);
        label->SetLocked(spec.locked);
        label->SetDescription("Description of label " + v);
        label->SetTrackingID("TID-" + v);
        label->SetTrackingUID("1.2.826.0.1.3680043.10.999.40." + v);
        label->SetAlgorithmType(spec.algorithmType);
        label->SetAlgorithmName("Algorithm " + v);
        label->SetSegmentedPropertyCategory(RichCode("C" + v, "Category " + v));
        label->SetSegmentedPropertyType(RichCodeWithModifiers("T" + v, "Type " + v, 2));
        label->SetAnatomicRegion(RichCodeWithModifiers("R" + v + "A", "Region " + v + " A", 2), 0);
        label->SetAnatomicRegion(RichCodeWithModifiers("R" + v + "B", "Region " + v + " B", 2), 1);
        label->SetPrimaryAnatomicStructure(RichCodeWithModifiers("P" + v, "Structure " + v, 1), 0);
        label->SetProperty(RichFixtureLabelCustomPropertyName(), IntProperty::New(10 * spec.value));
        return label;
      }

      /** Sets the voxels of the box to value; in later time steps the box shrinks along y so every time step differs. */
      inline void PaintRichBox(Image* groupImage, const VoxelBox& box, Label::PixelType value)
      {
        ImageWriteAccessor accessor(groupImage);
        auto* voxels = static_cast<Label::PixelType*>(accessor.GetData());

        for (unsigned int t = 0; t < groupImage->GetTimeSteps(); ++t)
        {
          const auto lastY = std::max(box.y0, box.y1 > t ? box.y1 - t : 0u);
          for (auto z = box.z0; z <= box.z1; ++z)
            for (auto y = box.y0; y <= lastY; ++y)
              for (auto x = box.x0; x <= box.x1; ++x)
                voxels[((t * RichSizeZ + z) * RichSizeY + y) * RichSizeX + x] = value;
        }
      }

      /**
       * Rotated, anisotropic and shifted, so a reader that drops any part of
       * the index-to-world transform is caught. The DICOM identity tags and
       * per-slice source UIDs are those of BuildSourceImageWithDICOMIdentity.
       */
      inline Image::Pointer BuildRichSourceImage(bool dynamic)
      {
        auto image3D = Image::New();
        unsigned int dimensions[3] = { RichSizeX, RichSizeY, RichSizeZ };
        image3D->Initialize(MakeScalarPixelType<Label::PixelType>(), 3, dimensions);

        Vector3D axis;
        FillVector3D(axis, 1.0, 2.0, 3.0);
        axis.Normalize();
        Vector3D spacing;
        FillVector3D(spacing, 0.8, 1.3, 2.5);
        Vector3D origin;
        FillVector3D(origin, -12.5, 30.25, 7.0);

        auto indexToWorld = AffineTransform3D::New();
        indexToWorld->Rotate3D(axis, 0.3);
        indexToWorld->Scale(spacing, true);
        indexToWorld->SetOffset(origin);
        image3D->GetGeometry()->SetIndexToWorldTransform(indexToWorld);

        Image::Pointer image = image3D;
        if (dynamic)
        {
          auto timeGeometry = ArbitraryTimeGeometry::New();
          timeGeometry->ClearAllGeometries();
          timeGeometry->AppendNewTimeStepClone(image3D->GetGeometry(), 0.0, 1.0);
          timeGeometry->AppendNewTimeStepClone(image3D->GetGeometry(), 1.0, 2.5);
          timeGeometry->AppendNewTimeStepClone(image3D->GetGeometry(), 2.5, 7.0);
          timeGeometry->UpdateBoundingBox();

          image = Image::New();
          image->Initialize(MakeScalarPixelType<Label::PixelType>(), *timeGeometry);
        }

        const auto identitySource = BuildSourceImageWithDICOMIdentity("Rich^Fixture", "RICH-PID-001", "RICH-Study-001",
          "1.2.826.0.1.3680043.10.999.41.1", "1.2.826.0.1.3680043.10.999.41.2", "1.2.826.0.1.3680043.10.999.41.3",
          RichSizeZ);
        image->SetPropertyList(identitySource->GetPropertyList()->Clone());
        return image;
      }
    }

    /**
     * Builds a segmentation in which every attribute a segmentation format
     * might carry is set, distinct per label, and away from its default, so
     * that a reader that drops, defaults or mixes up any of them is caught
     * by CompareSegmentations.
     *
     * Two groups of three labels (group 0 unnamed, group 1 named); values
     * neither contiguous nor inserted in order; per label colour, opacity,
     * visibility, lock, description, tracking ID and UID, algorithm type and
     * name, category, type with two modifiers, two anatomic regions with two
     * modifiers each, a primary anatomic structure and a typed custom
     * property. The segmentation carries DICOM identity tags, a source-image
     * relation, a typed custom property, the unlabeled lock and a rotated,
     * anisotropic, shifted geometry.
     */
    inline MultiLabelSegmentation::Pointer BuildRichSegmentation(const RichSegmentationOptions& options)
    {
      const auto source = detail::BuildRichSourceImage(options.dynamic);

      auto segmentation = MultiLabelSegmentation::New();
      segmentation->Initialize(source);
      SegSourceImageRelationRule::Connect(segmentation, source);
      segmentation->SetProperty(RichFixtureCustomPropertyName(), IntProperty::New(42));
      segmentation->SetUnlabeledLabelLock(true);

      segmentation->AddGroup();
      segmentation->SetGroupName(1, "Organs at risk");

      auto specs = detail::RichLabelSpecs();
      if (options.withEmptyLabel)
        specs.push_back(detail::RichEmptyLabelSpec());

      for (const auto& spec : specs)
      {
        segmentation->AddLabel(detail::BuildRichLabel(spec, options.dicomLegalNames), spec.group, true, false);
        for (const auto& box : spec.boxes)
          detail::PaintRichBox(segmentation->GetGroupImage(spec.group), box, spec.value);
      }

      if (options.withEmptyGroup)
      {
        const auto group = segmentation->AddGroup();
        segmentation->SetGroupName(group, "Empty group");
      }

      return segmentation;
    }

    /**
     * Reads path with the registered reader of the given description. Unlike
     * IOUtil::Load, which turns every exception into a plain mitk::Exception,
     * this lets the reader's own exception through, so tests can assert its
     * type.
     */
    inline std::vector<BaseData::Pointer> ReadWithReader(const std::string& path, const std::string& readerDescription)
    {
      FileReaderSelector selector(path);
      for (const auto& item : selector.Get())
      {
        if (item.GetDescription() == readerDescription)
        {
          auto* reader = item.GetReader();
          reader->SetInput(path);
          return reader->Read();
        }
      }
      CPPUNIT_FAIL("No reader \"" + readerDescription + "\" accepts " + path);
      return {};
    }

    /**
     * Like ReadWithReader, but without asking the reader for its confidence first.
     * FileReaderSelector drops a reader that rates the file Unsupported, which is
     * what a reader does with a file it cannot open or parse; this reaches the
     * reader's own handling of such files.
     */
    inline std::vector<BaseData::Pointer> ReadWithReaderIgnoringConfidence(const std::string& path,
                                                                           const std::string& readerDescription)
    {
      CoreServicePointer<IMimeTypeProvider> mimeTypeProvider(CoreServices::GetMimeTypeProvider());
      FileReaderRegistry registry;
      for (const auto& mimeType : mimeTypeProvider->GetMimeTypesForFile(path))
      {
        for (const auto& reference : FileReaderRegistry::GetReferences(mimeType))
        {
          if (reference.GetProperty(IFileReader::PROP_DESCRIPTION()).ToString() == readerDescription)
          {
            auto* reader = registry.GetReader(reference);
            reader->SetInput(path);
            return reader->Read();
          }
        }
      }
      CPPUNIT_FAIL("No reader \"" + readerDescription + "\" is registered for " + path);
      return {};
    }

    /**
     * Rewrites the header of the attached NRRD file at path, leaving its data
     * untouched: edit receives the header lines and may change, add or remove
     * any of them. This turns a valid file into a malformed one.
     */
    inline void EditNrrdHeader(const std::string& path, const std::function<void(std::vector<std::string>&)>& edit)
    {
      std::string content;
      {
        std::ifstream input(path, std::ios::binary);
        content.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
      }

      const auto headerEnd = content.find("\n\n");
      CPPUNIT_ASSERT_MESSAGE("No NRRD header found in " + path, headerEnd != std::string::npos);

      std::vector<std::string> lines;
      std::size_t lineStart = 0;
      while (lineStart <= headerEnd)
      {
        const auto lineEnd = content.find('\n', lineStart);
        lines.push_back(content.substr(lineStart, lineEnd - lineStart));
        lineStart = lineEnd + 1;
      }

      edit(lines);

      std::ofstream output(path, std::ios::binary | std::ios::trunc);
      for (const auto& line : lines)
        output << line << '\n';
      output << '\n';
      output.write(content.data() + headerEnd + 2, content.size() - headerEnd - 2);
    }

    /** Reads a JSON file, e.g. the meta file of a segmentation stack, to inspect or edit it. */
    inline nlohmann::json ReadJsonFile(const std::string& path)
    {
      std::ifstream input(path);
      CPPUNIT_ASSERT_MESSAGE("Cannot open " + path, input.is_open());
      return nlohmann::json::parse(input);
    }

    inline void WriteJsonFile(const std::string& path, const nlohmann::json& content)
    {
      std::ofstream output(path, std::ios::trunc);
      CPPUNIT_ASSERT_MESSAGE("Cannot write " + path, output.is_open());
      output << content.dump(2);
    }

    /** Replaces the value of the NRRD header field key (the part after ":="); fails if the key is absent. */
    inline void SetNrrdHeaderValue(std::vector<std::string>& lines, const std::string& key, const std::string& value)
    {
      const auto prefix = key + ":=";
      for (auto& line : lines)
      {
        if (line.compare(0, prefix.size(), prefix) == 0)
        {
          line = prefix + value;
          return;
        }
      }
      CPPUNIT_FAIL("NRRD header has no key " + key);
    }

    /** Removes the NRRD header field key; fails if the key is absent. */
    inline void RemoveNrrdHeaderKey(std::vector<std::string>& lines, const std::string& key)
    {
      const auto prefix = key + ":=";
      const auto finding = std::find_if(lines.begin(), lines.end(),
        [&prefix](const std::string& line) { return line.compare(0, prefix.size(), prefix) == 0; });
      CPPUNIT_ASSERT_MESSAGE("NRRD header has no key " + key, finding != lines.end());
      lines.erase(finding);
    }

    /**
     * Returns "label <value>: <key>" for every label property under DICOM.0062.0002., the
     * keys that MITK versions before 2025.12 stamped on every label.
     */
    inline std::vector<std::string> FindOutdatedLabelProperties(const MultiLabelSegmentation& segmentation)
    {
      const std::string prefix = "DICOM.0062.0002.";
      std::vector<std::string> result;
      for (const auto value : segmentation.GetAllLabelValues())
      {
        for (const auto& key : segmentation.GetLabel(value)->GetPropertyKeys())
        {
          if (key.compare(0, prefix.size(), prefix) == 0)
            result.push_back("label " + std::to_string(value) + ": " + key);
        }
      }
      return result;
    }

    /**
     * Records, while it lives, the warnings whose text contains a fragment. Filtering by a
     * fragment such as a property key keeps a case from counting the unrelated warnings a
     * read also emits, without pinning the rest of the wording.
     */
    class WarningRecorder : public LogBackendBase
    {
    public:
      explicit WarningRecorder(const std::string& fragment) : m_Fragment(fragment)
      {
        RegisterBackend(this);
      }

      ~WarningRecorder() override
      {
        UnregisterBackend(this);
      }

      WarningRecorder(const WarningRecorder&) = delete;
      WarningRecorder& operator=(const WarningRecorder&) = delete;

      void ProcessMessage(const LogMessage& message) override
      {
        if (LogLevel::Warn == message.Level && std::string::npos != message.Message.find(m_Fragment))
          m_Warnings.push_back(message.Message);
      }

      OutputType GetOutputType() const override
      {
        return OutputType::Other;
      }

      const std::vector<std::string>& GetWarnings() const
      {
        return m_Warnings;
      }

    private:
      std::string m_Fragment;
      std::vector<std::string> m_Warnings;
    };
  }
}

#endif
