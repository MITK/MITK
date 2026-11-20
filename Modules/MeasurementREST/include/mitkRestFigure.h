/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestFigure_h
#define mitkRestFigure_h

#include <MitkMeasurementRESTExports.h>

#include <mitkColorProperty.h>
#include <mitkDataNode.h>
#include <mitkIRestServerService.h>
#include <mitkNodePredicateBase.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureVisibilityProperty.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <optional>
#include <string>

class QmitkPulseTimers;

namespace mitk
{
  class DataStorage;

  /**
   * \brief The figures of the Measurement view: planar figures, except those
   *        of helper objects and contour markers, which belong to other
   *        components.
   */
  MITKMEASUREMENTREST_EXPORT NodePredicateBase::Pointer GetRestFigurePredicate();

  /** \brief A figure of the types the API draws, line, circle and rectangle, or null for any other type. */
  MITKMEASUREMENTREST_EXPORT PlanarFigure::Pointer CreateRestFigure(const std::string& type);

  /**
   * \brief A name like the Measurement view gives, "Circle", "Circle 2", ...,
   *        that no data has yet.
   *
   * \param type The type of the figure, not empty.
   */
  MITKMEASUREMENTREST_EXPORT std::string CreateRestFigureName(const DataStorage* dataStorage, const std::string& type);

  /** \brief The look of a figure, as the API shows and changes it. */
  struct MITKMEASUREMENTREST_EXPORT RestFigureLook
  {
    mitk::Color Color;
    float LineWidth = 0.0f;
    PlanarFigureVisibilityProperty::Visibility Label = PlanarFigureVisibilityProperty::Visibility::Never;
    bool ShowName = false;
    bool ShowMeasurements = false;
    bool ShowIn3D = false;
    bool Pulse = false;
  };

  /** \brief Reads the look of a node; properties that it lacks keep the values of the given look. */
  MITKMEASUREMENTREST_EXPORT RestFigureLook ReadRestFigureLook(const DataNode* node, RestFigureLook look);

  /** \brief The look that the mappers give a figure, for nodes that lack some of its properties. */
  MITKMEASUREMENTREST_EXPORT const RestFigureLook& GetDefaultRestFigureLook();

  /**
   * \brief A look as clients read it: {color, line_width, label: {show, name,
   *        measurements}, show_in_3d, pulse}.
   */
  MITKMEASUREMENTREST_EXPORT nlohmann::json ToRestJson(const RestFigureLook& look);

  /** \brief What a request changes of the look of a figure; a field without a value stays as it is. */
  struct MITKMEASUREMENTREST_EXPORT RestFigureLookChange
  {
    std::optional<mitk::Color> Color;
    std::optional<float> LineWidth;
    std::optional<PlanarFigureVisibilityProperty::Visibility> Label;
    std::optional<bool> ShowName;
    std::optional<bool> ShowMeasurements;
    std::optional<bool> ShowIn3D;
    std::optional<bool> Pulse;

    /** \brief How long a pulse that is switched on lasts; it lasts until switched off without. */
    std::optional<std::chrono::milliseconds> PulseDuration;
  };

  /** \brief Whether the key is a field of the look that a request may change. */
  MITKMEASUREMENTREST_EXPORT bool IsRestFigureLookField(const std::string& key);

  /**
   * \brief Reads the look fields of a request body, before anything changes,
   *        so that an invalid request changes nothing.
   *
   * Fields other than the look fields are not looked at; see
   * IsRestFigureLookField().
   *
   * \param path The request path, for errors.
   * \throw RestException with status 400 for a field of the wrong type or
   *        outside its range, an unknown field of "label", or pulse_seconds
   *        without "pulse": true.
   */
  MITKMEASUREMENTREST_EXPORT RestFigureLookChange ParseRestFigureLookChange(const nlohmann::json& body, const std::string& path);

  /**
   * \brief Changes the look of a node. Runs in the GUI thread.
   *
   * \param pulseTimers Ends a pulse after its duration; without timers, a
   *        pulse lasts until it is switched off.
   */
  MITKMEASUREMENTREST_EXPORT void ApplyRestFigureLookChange(
    DataNode* node, const RestFigureLookChange& change, QmitkPulseTimers* pulseTimers);

  /**
   * \brief A figure as the API describes it. The uids are left to ToRestJson(),
   *        as only the handler's thread may look them up.
   */
  struct MITKMEASUREMENTREST_EXPORT RestFigureDescription
  {
    DataNodeHandle Node;

    /** \brief The image that the figure marks, its parent in the Data Manager. */
    DataNodeHandle Image;

    nlohmann::json Json;
  };

  /**
   * \brief Describes a planar figure node: {name, type, closed, points,
   *        measurements} and its look. Runs in the GUI thread.
   *
   * \param node A node with a PlanarFigure.
   */
  MITKMEASUREMENTREST_EXPORT RestFigureDescription DescribeRestFigure(
    const IRestServerService& server, const DataStorage* dataStorage, DataNode* node);

  /** \brief The description with the uid of the figure and of its image, null for an image that it has not. */
  MITKMEASUREMENTREST_EXPORT nlohmann::json ToRestJson(const IRestServerService& server, const RestFigureDescription& figure);
}

#endif
