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
   * @brief Enumeration for hierarchy filter (GET /datastorage/nodes).
   *
   * Per API specification section 8.2:
   * - all: Return all nodes
   * - toplevel: Return only root nodes (parent_uid is null)
   */
  enum class MITKRESTAPI_EXPORT Hierarchy
  {
    All,      ///< All nodes in DataStorage
    Toplevel  ///< Only root nodes (no parent)
  };

  /**
   * @brief Enumeration for property scope.
   *
   * Per API specification section 5.3:
   * - all: Both node and data properties (node takes precedence)
   * - node: Only properties from DataNode's property list
   * - data: Only properties from BaseData's property list
   */
  enum class MITKRESTAPI_EXPORT PropertyScope
  {
    All,   ///< Both node and data properties
    Node,  ///< Only node properties
    Data   ///< Only data properties
  };

  /**
   * @brief Property filter with value and optional negation.
   *
   * Per API specification section 5.3:
   * - Exact match: filter.name=CT_Scan
   * - Not equal: filter.visible!=true
   * - Wildcard prefix: filter.name=CT*
   * - Wildcard suffix: filter.name=*Scan
   */
  struct MITKRESTAPI_EXPORT PropertyFilter
  {
    std::string key;
    std::string value;
    bool negated = false;  ///< true for != operator
  };

  /**
   * @brief Sort specification.
   *
   * Per API specification section 5.3:
   * - sort=name (ascending)
   * - sort=-timestamp (descending, minus prefix)
   */
  struct MITKRESTAPI_EXPORT SortSpec
  {
    std::string field;
    bool ascending = true;
  };

  /**
   * @brief Query parameters for GET /datastorage/nodes.
   *
   * Per API specification section 8.2.
   */
  struct MITKRESTAPI_EXPORT NodeQueryParams
  {
    // Pagination
    int limit = 50;    ///< Maximum number of results (1-1000)
    int offset = 0;    ///< Number of results to skip

    // Hierarchy filter
    Hierarchy hierarchy = Hierarchy::All;

    // System field filters
    std::optional<std::string> path;       ///< Path-based lookup
    std::optional<std::string> dataType;   ///< Filter by MITK data type
    std::optional<std::string> parentUid;  ///< Filter by parent node UID (use "null" for root)

    // Property filters (filter.{property}=value)
    std::vector<PropertyFilter> propertyFilters;

    // Property filter context
    std::optional<std::string> context;    ///< Renderer context for property lookup
    PropertyScope propertyScope = PropertyScope::All;

    // Field selection
    std::vector<std::string> fields;       ///< Fields to include in response (empty = all)

    // Sorting
    std::optional<SortSpec> sort;
  };

  /**
   * @brief Query parameters for GET /datastorage/nodes/{uid}/properties.
   *
   * Per API specification section 8.5.
   */
  struct MITKRESTAPI_EXPORT PropertyQueryParams
  {
    std::optional<std::string> context;    ///< Renderer context
    PropertyScope scope = PropertyScope::All;
    bool includeContent = true;            ///< Include property values (content parameter)
    std::vector<std::string> names;        ///< Specific property names to return (empty = all)
  };
}

#endif
