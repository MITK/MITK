/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOVolumeSplitReason_h
#define mitkIOVolumeSplitReason_h

#include "itkLightObject.h"
#include "mitkCommon.h"

#include "MitkCoreExports.h"

namespace mitk
{

  class MITKCORE_EXPORT IOVolumeSplitReason : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(IOVolumeSplitReason, itk::LightObject);
    itkFactorylessNewMacro(IOVolumeSplitReason);
    itkCloneMacro(IOVolumeSplitReason);

    enum class ReasonType
    {
      Unkown = 0,
      ValueSplitDifference = 2, //*< split due to different values in splitting relevant dicom tags
      ValueSortDistance = 3, //*< split due value distance of sort criterion to large for relevant dicom tag(s)
      ImagePostionMissing = 4, //*< split because image position tag was missing in one of the compared files
      OverlappingSlices = 8,  //*< split because at least two input files are overlapping in world coordinate space
      GantryTiltDifference = 16, //*< split because the gantry tilts of at least two input files were different
      SliceDistanceInconsistency = 32, //*< split because the distance between slices were inconsistent.
                                       // This can either be evoked by volumes with heterogeneous z spacing or by missing slices.
                                       // Details for this reason will contain the detected slice distance inconsistency
      MissingSlices = 33 //*< Indicates that is a split was done due to missing slices. (It is a sub class of SliceDistanceInconsistency
                         // as all SliceDistanceInconsistency with a positive distance inconsistency greater then one times the slice
                         // thickness are deemed also missing slices as split reason. This sub class was introduced to make it easier
                         // for parsing applications to react on this important split reason.
                         // Details for this reason will contain the assumed/detected number of missing slices
    };

    void AddReason(ReasonType type, std::string_view detail = "");
    void RemoveReason(ReasonType type);

    bool ReasonExists() const;
    bool ReasonExists(ReasonType type) const;
    std::string GetReasonDetails(ReasonType type) const;

    Pointer ExtendReason(const Self* otherReason) const;

    static std::string SerializeToJSON(const IOVolumeSplitReason*);
    static Pointer DeserializeFromJSON(const std::string& reasonStr);

    static std::string TypeToString(const IOVolumeSplitReason::ReasonType& reasonType);
    static IOVolumeSplitReason::ReasonType StringToType(const std::string& reasonStr);

  protected:
    mitkCloneMacro(IOVolumeSplitReason);

    IOVolumeSplitReason();
    ~IOVolumeSplitReason() override;
    IOVolumeSplitReason(const IOVolumeSplitReason& other);

  private:
    IOVolumeSplitReason& operator=(const IOVolumeSplitReason& other);

    using ReasonMapType = std::map<ReasonType, std::string>;
    ReasonMapType m_ReasonMap;
  };

}

#endif
