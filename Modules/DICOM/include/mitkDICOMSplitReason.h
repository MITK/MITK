/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSplitReason_h
#define mitkDICOMSplitReason_h

#include "itkLightObject.h"
#include "mitkCommon.h"

#include "MitkDICOMExports.h"

namespace mitk
{

  class MITKDICOM_EXPORT DICOMSplitReason : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(DICOMSplitReason, itk::LightObject);
    itkFactorylessNewMacro(DICOMSplitReason);
    itkCloneMacro(DICOMSplitReason);

    enum class ReasonType
    {
      Unkown = 0,
      ValueSplitDifference = 2, //*< split due to different values in splitting relevant dicom tags
      ValueSortDistance = 3, //*< split due value distance of sort criterion to large for relevant dicom tag(s)
      ImagePostionMissing = 4, //*< split because image position tag was missing in one of the compared files
      OverlappingSlices = 8,  //*< split because at least two input files are overlapping in world coordinate space
      GantryTiltDifference = 16, //*< split because the gantry tilts of at least two input files were different
      SliceDistanceInconsistency = 32 //*< split because the distance between slices were inconsistent.
                                        // This can either be evoked by volumes with heterogeneous z spacing or by missing slices.
    };

    void AddReason(ReasonType type, std::string_view detail = "");
    void RemoveReason(ReasonType type);

    bool ReasonExists() const;
    bool ReasonExists(ReasonType type) const;
    std::string GetReasonDetails(ReasonType type) const;

    Pointer ExtendReason(const Self* otherReason) const;

    static std::string SerializeToJSON(const DICOMSplitReason*);
    static std::string TypeToString(const DICOMSplitReason::ReasonType& reasonType);

  protected:
    mitkCloneMacro(DICOMSplitReason);

    DICOMSplitReason();
    ~DICOMSplitReason() override;
    DICOMSplitReason(const DICOMSplitReason& other);

  private:
    DICOMSplitReason& operator=(const DICOMSplitReason& other);

    using ReasonMapType = std::map<ReasonType, std::string>;
    ReasonMapType m_ReasonMap;
  };

}

#endif
