/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestSegmentation_h
#define mitkRestSegmentation_h

#include <MitkSegmentationRESTExports.h>

#include <mitkColorProperty.h>
#include <mitkDataNode.h>
#include <mitkLabelSetImage.h>
#include <mitkRestRoute.h>
#include <mitkRestTime.h>

#include <nlohmann/json.hpp>

#include <charconv>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace mitk
{
  class IPreferences;
  struct IRestServerService;

  /**
   * \brief A node as clients name it: {uid, name}; null for no node.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestNode(DataNode* node,const IRestServerService& server);

  /**
   * \brief A color from a JSON value, which must be a string written like "#ff8800".
   *
   * \param path The request path, for errors.
   * \throw RestException with status 400 for any other value.
   */
  MITKSEGMENTATIONREST_EXPORT Color ParseRestColorOrThrow(const nlohmann::json& json, const std::string& path);

  /**
   * \brief Whether the request asks for the voxels, volumes and centers of
   *        labels, which take a pass over the segmentation: the query
   *        parameter "measure" is true.
   *
   * \return False without the parameter or with false.
   * \throw RestException with status 400 for any other value.
   */
  MITKSEGMENTATIONREST_EXPORT bool IsRestMeasureRequested(const RestRequest& request);

  /**
   * \brief A label as clients read it: {value, name, color, visible, locked, group}.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestLabel(const Label* label, MultiLabelSegmentation::GroupIndexType group);

  /**
   * \brief The labels of all groups of the segmentation, in the order of the groups.
   *
   * With a selection, each label also has voxels, volume_ml and center at the
   * time point of the selection, or time_steps with them for all time steps;
   * that takes a pass over the segmentation. The values are null for a time
   * step at which a tool writes into the segmentation, because the measure
   * never waits for it; the center is null for a label without voxels.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestLabels(
    MultiLabelSegmentation* segmentation, const std::optional<RestTimeSelection>& measureAt = std::nullopt);

  /**
   * \brief A group as clients read it: {index, name, display_name, labels}.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestGroup(
    MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType group);

  /** \brief All groups of the segmentation as DescribeRestGroup() gives them, in order. */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestGroups(MultiLabelSegmentation* segmentation);

  /**
   * \brief What the preferences of the Segmentation view demand of its user,
   *        which the routes do not enforce: {label_set_preset,
   *        label_suggestions_enforced}.
   *
   * Without the suggested names, which can be hundreds.
   *
   * \param preferences The preferences node of the Segmentation view.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestRestrictions(const IPreferences& preferences);

  /**
   * \brief The label names that the view suggests, and takes alone if the
   *        suggestions are enforced: {names, enforced}.
   *
   * Reads the label suggestion preferences of the application.
   */
  MITKSEGMENTATIONREST_EXPORT nlohmann::json DescribeRestLabelSuggestions();

  /**
   * \brief Refuses a body with a field that \p subject does not have, which a
   *        client that misspelled one would read as done.
   *
   * \param fields The fields that \p subject has.
   * \param path The request path, for errors.
   * \throw RestException with status 400 if the body is no object or has
   *        another field.
   */
  MITKSEGMENTATIONREST_EXPORT void ThrowIfUnknownRestField(
    const nlohmann::json& body, const std::vector<std::string>& fields, const std::string& subject, const std::string& path);

  /**
   * \brief Reads a number of the type \p T that a path names.
   *
   * Unlike std::stoul(), takes no sign, white space or trailing text, and no
   * value beyond the type, all of which would name another label or group
   * than the path does.
   *
   * \return No value for text that is no such number.
   */
  template <typename T>
  std::optional<T> ParseRestNumber(const std::string& text)
  {
    T value = 0;
    const auto* end = text.data() + text.size();
    const auto [parsed, error] = std::from_chars(text.data(), end, value);

    if (error != std::errc() || parsed != end)
      return std::nullopt;

    return value;
  }

  /**
   * \brief The label that the path parameter "value" of the request names.
   *
   * \throw RestException with status 400 if the value is no whole number of
   *        the label value type, and with status 404 and code
   *        LABEL_NOT_FOUND if the segmentation has no such label.
   */
  MITKSEGMENTATIONREST_EXPORT Label::Pointer GetRestLabel(MultiLabelSegmentation* segmentation, const RestRequest& request);

  /**
   * \brief The index of the group that the path parameter "index" of the request names.
   *
   * \throw RestException with status 400 if the index is no whole number,
   *        and with status 404 and code GROUP_NOT_FOUND if the segmentation
   *        has no such group.
   */
  MITKSEGMENTATIONREST_EXPORT MultiLabelSegmentation::GroupIndexType GetRestGroupIndex(
    const MultiLabelSegmentation* segmentation, const RestRequest& request);
}

#endif
