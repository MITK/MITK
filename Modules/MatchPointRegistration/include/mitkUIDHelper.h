/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkUIDHelper_h
#define mitkUIDHelper_h

#include <string>

//MITK
#include <MitkMatchPointRegistrationExports.h>
#include <mitkMatchPointPropertyTags.h>

/**
 * \brief Utility functions for managing unique identifiers (UIDs) on DataNodes and BaseData objects.
 *
 * These helpers ensure that data nodes and data objects carry a persistent UID property
 * used by the MatchPoint registration framework to track provenance and associations.
 *
 * \sa mitk::MAPRegistrationWrapper, mitk::mitkMatchPointPropertyTags
 */
namespace mitk
{
  class DataNode;
  class BaseData;

  /** \brief Type alias for node/data UID strings. */
  typedef std::string NodeUIDType;

  /**
   * \brief Ensures that the given DataNode has a UID property ("matchpoint.UID").
   *
   * If the property does not yet exist, a new UID is generated and set on the node.
   *
   * \param[in] node Pointer to the DataNode to check/update.
   * \return The UID string associated with the node.
   * \pre \p node must point to a valid instance.
   * \throw mitk::Exception if \p node is nullptr.
   * \sa CheckUID(const mitk::DataNode*, const NodeUIDType&)
   */
  NodeUIDType MITKMATCHPOINTREGISTRATION_EXPORT EnsureUID(mitk::DataNode* node);

  /**
   * \brief Checks whether the UID property of the given DataNode matches the specified UID.
   *
   * \param[in] node Pointer to the DataNode to check.
   * \param[in] uid The UID string to compare against.
   * \return True if the node's "matchpoint.UID" property equals \p uid, false otherwise
   *         (including when the node is nullptr or the property does not exist).
   * \sa EnsureUID(mitk::DataNode*)
   */
  bool MITKMATCHPOINTREGISTRATION_EXPORT CheckUID(const mitk::DataNode* node, const NodeUIDType& uid);

  /**
   * \brief Ensures that the given BaseData has a UID property ("data.UID").
   *
   * If the property does not yet exist, a new UID is generated and set on the data object.
   *
   * \param[in] data Pointer to the BaseData to check/update.
   * \return The UID string associated with the data object.
   * \pre \p data must point to a valid instance.
   * \throw mitk::Exception if \p data is nullptr.
   * \sa CheckUID(const mitk::BaseData*, const NodeUIDType&)
   */
  NodeUIDType MITKMATCHPOINTREGISTRATION_EXPORT EnsureUID(mitk::BaseData* data);

  /**
   * \brief Checks whether the UID property of the given BaseData matches the specified UID.
   *
   * \param[in] data Pointer to the BaseData to check.
   * \param[in] uid The UID string to compare against.
   * \return True if the data's "data.UID" property equals \p uid, false otherwise
   *         (including when the data is nullptr or the property does not exist).
   * \sa EnsureUID(mitk::BaseData*)
   */
  bool MITKMATCHPOINTREGISTRATION_EXPORT CheckUID(const mitk::BaseData* data, const NodeUIDType& uid);
}

#endif
