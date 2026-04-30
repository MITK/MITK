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

#include <MitkCoreExports.h>

namespace mitk
{

  /**
   * \ingroup IO
   *
   * \brief Records the reasons why a DICOM volume was split during loading.
   *
   * When DICOM series are loaded, the reader may need to split the input
   * files into multiple volumes. This class captures the specific reasons
   * for each split, along with optional detail strings providing additional
   * context (e.g., the detected distance inconsistency or number of
   * missing slices).
   *
   * Reasons are stored as a map from ReasonType to a detail string and can
   * be serialized to/from JSON for persistence.
   *
   * \sa IOVolumeSplitReason::ReasonType
   */
  class MITKCORE_EXPORT IOVolumeSplitReason
  {
  public:
    using Self = IOVolumeSplitReason;
    /** \brief Shared pointer type. */
    using Pointer = std::shared_ptr<IOVolumeSplitReason>;
    /** \brief Shared pointer to const type. */
    using ConstPointer = std::shared_ptr<const IOVolumeSplitReason>;

    /**
     * \brief Enumeration of possible reasons for splitting a DICOM volume.
     */
    enum class ReasonType
    {
      Unknown = 0,                ///< Unknown or unclassified reason.
      ValueSplitDifference,       ///< Split due to different values in splitting-relevant DICOM tags.
      ValueSortDistance,           ///< Split due to value distance of sort criterion being too large for relevant DICOM tag(s).
      ImagePostionMissing,        ///< Split because the image position tag was missing in one of the compared files.
      OverlappingSlices,          ///< Split because at least two input files overlap in world coordinate space.
      GantryTiltDifference,       ///< Split because the gantry tilts of at least two input files differ.
      SliceDistanceInconsistency, ///< Split because slice distances are inconsistent (heterogeneous z-spacing or missing slices). Details contain the detected inconsistency value.
      MissingSlices               ///< Split due to missing slices (a sub-class of SliceDistanceInconsistency). Details contain the assumed number of missing slices.
    };

    /**
     * \brief Add a split reason with optional detail text.
     * \param[in] type The reason type to add.
     * \param[in] detail Optional detail string providing context (e.g., numeric values).
     *
     * If the reason type already exists, its detail is overwritten.
     */
    void AddReason(ReasonType type, const std::string& detail = "");

    /**
     * \brief Remove a previously added split reason.
     * \param[in] type The reason type to remove. No-op if not present.
     */
    void RemoveReason(ReasonType type);

    /**
     * \brief Check whether any split reasons have been recorded.
     * \return \c true if at least one reason is present.
     */
    bool HasReasons() const;

    /**
     * \brief Check whether a specific reason type has been recorded.
     * \param[in] type The reason type to look for.
     * \return \c true if the given reason type is present.
     */
    bool HasReason(ReasonType type) const;

    /**
     * \brief Get the detail string for a specific reason type.
     * \param[in] type The reason type to query.
     * \return The detail string associated with the reason.
     * \throw mitk::Exception if the reason type is not present.
     */
    std::string GetReasonDetails(ReasonType type) const;

    /**
     * \brief Clone this instance and extend it with reasons from another instance.
     *
     * Creates a copy of this object, then adds any reason types from
     * \c otherReason that are not already present in this instance.
     * Existing reason types are not overwritten.
     *
     * \param[in] otherReason The other instance whose reasons to merge.
     * \return A new IOVolumeSplitReason containing the merged reasons.
     * \pre \c otherReason must point to a valid instance.
     */
    Pointer ExtendReason(ConstPointer otherReason) const;

    /**
     * \brief Serialize a split reason to JSON.
     * \param[in] reason The instance to serialize (must not be \c nullptr).
     * \return A JSON array representing the reasons and their details.
     * \throw mitk::Exception if \c reason is \c nullptr.
     */
    static nlohmann::json ToJSON(ConstPointer reason);

    /**
     * \brief Deserialize a split reason from JSON.
     * \param[in] j The JSON value to parse.
     * \return A new IOVolumeSplitReason populated from the JSON.
     */
    static Pointer FromJSON(const nlohmann::json& j);

    /**
     * \brief Convert a ReasonType to its string representation.
     * \param[in] reasonType The reason type to convert.
     * \return The string identifier (e.g., "missing_slices", "overlapping_slices").
     */
    static std::string TypeToString(ReasonType reasonType);

    /**
     * \brief Convert a string representation to a ReasonType.
     * \param[in] reasonStr The string identifier to parse.
     * \return The corresponding ReasonType, or ReasonType::Unknown if unrecognized.
     */
    static IOVolumeSplitReason::ReasonType StringToType(const std::string& reasonStr);

    /**
     * \brief Create a deep copy of this instance.
     * \return A new IOVolumeSplitReason with identical contents.
     */
    Pointer Clone() const;

    /**
     * \brief Create a new, empty IOVolumeSplitReason instance.
     * \return A shared pointer to the new instance.
     */
    static Pointer New();

  protected:
    /** \brief Map storing reason types and their associated detail strings. */
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


  /**
   * \brief Serialize an IOVolumeSplitReason (const) to a JSON object.
   * \param[out] j The JSON object to write to.
   * \param[in] reason The split reason to serialize.
   */
  MITKCORE_EXPORT void to_json(nlohmann::json& j, IOVolumeSplitReason::ConstPointer reason);

  /**
   * \brief Serialize an IOVolumeSplitReason to a JSON object.
   * \param[out] j The JSON object to write to.
   * \param[in] reason The split reason to serialize.
   */
  MITKCORE_EXPORT void to_json(nlohmann::json& j, IOVolumeSplitReason::Pointer reason);

  /**
   * \brief Deserialize an IOVolumeSplitReason from a JSON object.
   * \param[in] j The JSON value to parse.
   * \param[out] reason The IOVolumeSplitReason to populate.
   */
  MITKCORE_EXPORT void from_json(const nlohmann::json& j, IOVolumeSplitReason& reason);

}

#endif
