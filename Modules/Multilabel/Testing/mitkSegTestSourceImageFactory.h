/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegTestSourceImageFactory_h
#define mitkSegTestSourceImageFactory_h

#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <string>

namespace mitk
{
  namespace test
  {
    /**
     * \brief Build an mitk::Image whose property list carries the
     * IOD-mandated identity tags a derived SEG (or parametric map)
     * would inherit from a real DICOM source.
     *
     * Populates:
     *  - Patient: (0010,0010), (0010,0020), (0010,0030), (0010,0040)
     *  - Study: (0020,000D), (0020,0010)
     *  - Series: (0020,000E) (the source's own series UID; the derived
     *    seg's series UID is its own and must NOT equal this)
     *  - Frame of Reference: (0020,0052)
     *  - Per-slice (0008,0018) SOPInstance + (0008,0016) SOPClass UIDs
     *    (TemporoSpatialStringProperty) so SegSourceImageRelationRule::
     *    Connect has real per-slice items to bind.
     *
     * \note This helper does NOT attach a SegSourceImageRelationRule::
     * Connect. The caller is responsible for calling Connect(seg, image)
     * before saving in strict mode (the writer's strict-mode gate
     * requires per-slice source items to be non-empty).
     */
    inline mitk::Image::Pointer BuildSourceImageWithDICOMIdentity(
      const std::string &patientName = "WorkflowTest^Patient",
      const std::string &patientID = "WF-PID-001",
      const std::string &studyID = "WF-Study-001",
      const std::string &studyInstanceUID = "1.2.826.0.1.3680043.10.999.10.1",
      const std::string &seriesInstanceUID = "1.2.826.0.1.3680043.10.999.10.2",
      const std::string &frameOfReferenceUID = "1.2.826.0.1.3680043.10.999.10.3",
      unsigned int sliceCount = 3)
    {
      auto image = mitk::Image::New();
      unsigned int dim[3] = {4u, 4u, sliceCount};
      image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);

      auto setStr = [&](unsigned int g, unsigned int e, const std::string &value) {
        image->SetProperty(GeneratePropertyNameForDICOMTag(g, e).c_str(),
                           TemporoSpatialStringProperty::New(value));
      };

      setStr(0x0010, 0x0010, patientName);
      setStr(0x0010, 0x0020, patientID);
      setStr(0x0010, 0x0030, "19700101");
      setStr(0x0010, 0x0040, "O");
      setStr(0x0020, 0x000D, studyInstanceUID);
      setStr(0x0020, 0x0010, studyID);
      setStr(0x0020, 0x000E, seriesInstanceUID);
      setStr(0x0020, 0x0052, frameOfReferenceUID);

      auto perSliceInstance = TemporoSpatialStringProperty::New();
      auto perSliceClass = TemporoSpatialStringProperty::New();
      for (unsigned int s = 0; s < sliceCount; ++s)
      {
        perSliceInstance->SetValue(0, s,
          "1.2.826.0.1.3680043.10.999.10.4." + std::to_string(s));
        perSliceClass->SetValue(0, s, "1.2.840.10008.5.1.4.1.1.2");
      }
      image->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0018).c_str(),
                         perSliceInstance);
      image->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0016).c_str(),
                         perSliceClass);

      return image;
    }
  }
}

#endif
