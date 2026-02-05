/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkErrorResponse.h"

namespace mitk
{
  ErrorResponse::Json ErrorResponse::Create(
    const std::string& code,
    const std::string& title,
    const std::string& detail,
    int status,
    const std::string& instance)
  {
    Json error;
    error["type"] = "https://docs.mitk.org/api/errors/" + code;
    error["code"] = code;
    error["title"] = title;
    error["message"] = detail;
    error["status"] = status;

    if (!instance.empty())
    {
      error["instance"] = instance;
    }

    Json response;
    response["error"] = error;
    return response;
  }

  ErrorResponse::Json ErrorResponse::NodeNotFound(const std::string& uid, const std::string& instance)
  {
    return Create(
      CODE_NODE_NOT_FOUND,
      "Node Not Found",
      "No node found with UID '" + uid + "'",
      404,
      instance);
  }

  ErrorResponse::Json ErrorResponse::PropertyNotFound(
    const std::string& propertyKey,
    const std::string& nodeUid,
    const std::string& instance)
  {
    return Create(
      CODE_PROPERTY_NOT_FOUND,
      "Property Not Found",
      "Property '" + propertyKey + "' not found on node '" + nodeUid + "'",
      404,
      instance);
  }

  ErrorResponse::Json ErrorResponse::DataStorageNotAvailable(const std::string& instance)
  {
    return Create(
      CODE_DATASTORAGE_NOT_AVAILABLE,
      "DataStorage Not Available",
      "No DataStorage is currently connected to the REST API server",
      503,
      instance);
  }

  ErrorResponse::Json ErrorResponse::InvalidRequest(const std::string& detail, const std::string& instance)
  {
    return Create(
      CODE_INVALID_REQUEST,
      "Invalid Request",
      detail,
      400,
      instance);
  }

  ErrorResponse::Json ErrorResponse::InternalError(const std::string& detail, const std::string& instance)
  {
    return Create(
      CODE_INTERNAL_ERROR,
      "Internal Server Error",
      detail,
      500,
      instance);
  }

  ErrorResponse::Json ErrorResponse::PropertyProtected(const std::string& propertyKey, const std::string& instance)
  {
    return Create(
      CODE_PROPERTY_PROTECTED,
      "Property Protected",
      "Property '" + propertyKey + "' cannot be deleted",
      400,
      instance);
  }

  ErrorResponse::Json ErrorResponse::NotImplemented(const std::string& feature, const std::string& instance)
  {
    return Create(
      CODE_NOT_IMPLEMENTED,
      "Not Implemented",
      "Feature '" + feature + "' is not yet implemented",
      501,
      instance);
  }

  ErrorResponse::Json ErrorResponse::NodeHasChildren(int childrenCount, const std::string& instance)
  {
    Json response = Create(
      CODE_NODE_HAS_CHILDREN,
      "Node Has Children",
      "Node has " + std::to_string(childrenCount) + " children. Use ?recursive=true to delete.",
      409,
      instance);

    response["error"]["children_count"] = childrenCount;
    return response;
  }

  ErrorResponse::Json ErrorResponse::NoData(const std::string& nodeUid, const std::string& instance)
  {
    return Create(
      CODE_NO_DATA,
      "No Data",
      "Node '" + nodeUid + "' has no data attached",
      404,
      instance);
  }

  ErrorResponse::Json ErrorResponse::UnsupportedFormat(const std::string& detail, const std::string& instance)
  {
    return Create(
      CODE_UNSUPPORTED_FORMAT,
      "Unsupported Format",
      detail,
      415,
      instance);
  }

  ErrorResponse::Json ErrorResponse::SerializationError(const std::string& detail, const std::string& instance)
  {
    return Create(
      CODE_SERIALIZATION_ERROR,
      "Serialization Error",
      detail,
      500,
      instance);
  }

  ErrorResponse::Json ErrorResponse::FileNotFound(const std::string& filePath, const std::string& instance)
  {
    return Create(
      CODE_FILE_NOT_FOUND,
      "File Not Found",
      "Referenced file path does not exist: " + filePath,
      422,
      instance);
  }

  ErrorResponse::Json ErrorResponse::FileReadError(
    const std::string& filePath,
    const std::string& detail,
    const std::string& instance)
  {
    return Create(
      CODE_FILE_READ_ERROR,
      "File Read Error",
      "Cannot read file '" + filePath + "': " + detail,
      422,
      instance);
  }

  ErrorResponse::Json ErrorResponse::TransferModeNotAvailable(
    const std::string& requestedMode,
    const std::vector<std::string>& availableModes,
    const std::string& instance)
  {
    Json response = Create(
      CODE_TRANSFER_MODE_NOT_AVAILABLE,
      "Transfer Mode Not Available",
      "Requested transfer mode '" + requestedMode + "' is not supported",
      406,
      instance);

    Json modesArray = Json::array();
    for (const auto& mode : availableModes)
    {
      modesArray.push_back(mode);
    }
    response["error"]["available_modes"] = modesArray;
    return response;
  }
}
