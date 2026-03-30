/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNodeQueryParams_h
#define mitkNodeQueryParams_h

#include <optional>
#include <string>
#include <vector>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * \brief Enumeration for hierarchy filter used in node queries.
   *
   * Controls whether all nodes or only top-level (root) nodes are returned
   * from a GET /datastorage/nodes request.
   *
   * \sa NodeQueryParams
   */
  enum class Hierarchy
  {
    All,      ///< \brief Return all nodes in the DataStorage.
    Toplevel  ///< \brief Return only root nodes (nodes with no parent).
  };

  /**
   * \brief Enumeration for property scope in REST API queries.
   *
   * Determines which property lists are searched when filtering or
   * returning properties. Node properties come from the DataNode's
   * property list; data properties come from the BaseData's property list.
   *
   * \sa PropertyQueryParams, NodeQueryParams
   */
  enum class PropertyScope
  {
    All,   ///< \brief Both node and data properties (node takes precedence on conflicts).
    Node,  ///< \brief Only properties from the DataNode's property list.
    Data   ///< \brief Only properties from the BaseData's property list.
  };

  /**
   * \brief A single property filter criterion with key, value, and optional negation.
   *
   * Supports exact match, negation (!=), and wildcard prefix/suffix patterns.
   * For example:
   *   - Exact match: \c filter.name=CT_Scan
   *   - Not equal: \c filter.visible!=true
   *   - Wildcard prefix: \c filter.name=CT*
   *   - Wildcard suffix: \c filter.name=*Scan
   *
   * \sa NodeQueryParams
   */
  struct MITKRESTAPI_EXPORT PropertyFilter
  {
    std::string key;               ///< \brief The property key to filter on.
    std::string value;             ///< \brief The value to match against (may contain wildcards).
    bool negated = false;          ///< \brief If \c true, the filter uses != (not-equal) semantics.
  };

  /**
   * \brief Sort specification for node query results.
   *
   * Defines the field to sort by and the sort direction. A descending sort
   * is indicated by a minus prefix in the query string (e.g., \c sort=-timestamp).
   *
   * \sa NodeQueryParams
   */
  struct MITKRESTAPI_EXPORT SortSpec
  {
    std::string field;             ///< \brief The field name to sort by (e.g., "name", "timestamp").
    bool ascending = true;         ///< \brief If \c true, sort in ascending order; otherwise descending.
  };

  /**
   * \brief Query parameters for the GET /datastorage/nodes endpoint.
   *
   * Encapsulates all supported query parameters including pagination, hierarchy
   * filtering, system field filters, property filters, field selection, and sorting.
   *
   * \sa DataStorageBridge::GetNodes, PropertyFilter, SortSpec, Hierarchy
   */
  struct MITKRESTAPI_EXPORT NodeQueryParams
  {
    int limit = 50;    ///< \brief Maximum number of results to return (valid range: 1--1000).
    int offset = 0;    ///< \brief Number of results to skip for pagination.

    Hierarchy hierarchy = Hierarchy::All;  ///< \brief Hierarchy filter mode.

    std::optional<std::string> path;       ///< \brief Filter by path-based lookup.
    std::optional<std::string> dataType;   ///< \brief Filter by MITK data type name.
    std::optional<std::string> parentUid;  ///< \brief Filter by parent node UID (use "null" for root nodes).

    std::vector<PropertyFilter> propertyFilters;  ///< \brief Property-based filter criteria.

    std::optional<std::string> context;    ///< \brief Renderer context name for property lookups.
    PropertyScope propertyScope = PropertyScope::All;  ///< \brief Scope for property filter evaluation.

    std::vector<std::string> fields;       ///< \brief Fields to include in the response (empty means all fields).

    std::optional<SortSpec> sort;          ///< \brief Optional sort specification.
  };

  /**
   * \brief Query parameters for the GET /datastorage/nodes/{uid}/properties endpoint.
   *
   * Controls which properties are returned, including scope filtering,
   * renderer context selection, and optional content inclusion.
   *
   * \sa DataStorageBridge::GetNodeProperties, PropertyScope
   */
  struct MITKRESTAPI_EXPORT PropertyQueryParams
  {
    std::optional<std::string> context;    ///< \brief Renderer context name for property lookup.
    PropertyScope scope = PropertyScope::All;  ///< \brief Property scope filter.
    bool includeContent = true;            ///< \brief If \c true, include property values in the response.
    std::vector<std::string> names;        ///< \brief Specific property names to return (empty means all properties).
  };
}

#endif
