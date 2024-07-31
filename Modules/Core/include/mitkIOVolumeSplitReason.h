/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOVolumeSplitReason_h
#define mitkIOVolumeSplitReason_h

#include <memory>
#include <nlohmann/json.hpp>
#include <mitkCommon.h>

#include "MitkCoreExports.h"

namespace mitk
{

  class MITKCORE_EXPORT IOVolumeSplitReason
  {
  public:
    using Self = IOVolumeSplitReason;
    using Pointer = std::shared_ptr<IOVolumeSplitReason>;
    using ConstPointer = std::shared_ptr<const IOVolumeSplitReason>;

    enum class ReasonType
    {
      Unknown = 0,
      ValueSplitDifference, //*< split due to different values in splitting relevant dicom tags
      ValueSortDistance, //*< split due value distance of sort criterion too large for relevant dicom tag(s)
      ImagePostionMissing, //*< split because image position tag was missing in one of the compared files
      OverlappingSlices,  //*< split because at least two input files are overlapping in world coordinate space
      GantryTiltDifference, //*< split because the gantry tilts of at least two input files were different
      SliceDistanceInconsistency, //*< split because the distance between slices were inconsistent.
                                  // This can either be evoked by volumes with heterogeneous z spacing or by missing slices.
                                  // Details for this reason will contain the detected slice distance inconsistency
      MissingSlices //*< Indicates that is a split was done due to missing slices. (It is a sub class of SliceDistanceInconsistency
                    // as all SliceDistanceInconsistency with a positive distance inconsistency greater then one times the slice
                    // thickness are deemed also missing slices as split reason. This sub class was introduced to make it easier
                    // for parsing applications to react on this important split reason.
                    // Details for this reason will contain the assumed/detected number of missing slices
    };

    void AddReason(ReasonType type, const std::string& detail = "");
    void RemoveReason(ReasonType type);

    bool HasReasons() const;
    bool HasReason(ReasonType type) const;
    std::string GetReasonDetails(ReasonType type) const;

    /** This methods generates a clone of this instances and extends the clone by the reason types and details
      provided by another IOVolumeSplitReason instance.
      @remark If the other instance contains a reason type that is already existing it will be ignored.
      Therefor only new types and details will be added to the extension.
      @pre otherReason must point to a valid instance.
      @return Pointer to the cloned and extended instance.*/
    Pointer ExtendReason(ConstPointer otherReason) const;

    static nlohmann::json ToJSON(ConstPointer);
    static Pointer FromJSON(const nlohmann::json& j);

    static std::string TypeToString(ReasonType reasonType);
    static IOVolumeSplitReason::ReasonType StringToType(const std::string& reasonStr);

    Pointer Clone() const;
    static Pointer New();

  protected:
    using ReasonMapType = std::map<ReasonType, std::string>;
    ReasonMapType m_ReasonMap;
  };

  template<typename BasicJsonType>
  inline void to_json(BasicJsonType& j, const IOVolumeSplitReason::ReasonType& e)
  {
    static_assert(std::is_enum<IOVolumeSplitReason::ReasonType>::value,
                  "IOVolumeSplitReason::ReasonType"
                  " must be an enum!");
    j = IOVolumeSplitReason::TypeToString(e);
  }

  template<typename BasicJsonType>
  inline void from_json(const BasicJsonType& j, IOVolumeSplitReason::ReasonType& e)
  {
    static_assert(std::is_enum<IOVolumeSplitReason::ReasonType>::value,
                  "IOVolumeSplitReason::ReasonType"
                  " must be an enum!");
    e = IOVolumeSplitReason::StringToType(j.template get<std::string>());
  }


  MITKCORE_EXPORT void to_json(nlohmann::json& j, IOVolumeSplitReason::ConstPointer reason);
  MITKCORE_EXPORT void to_json(nlohmann::json& j, IOVolumeSplitReason::Pointer reason);

  MITKCORE_EXPORT void from_json(const nlohmann::json& j, IOVolumeSplitReason& reason);

}

#endif
