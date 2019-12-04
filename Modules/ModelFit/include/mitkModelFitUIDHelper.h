/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkModelFitUIDHelper_h
#define mitkModelFitUIDHelper_h

#include <string>

//MITK
#include <mitkDataNode.h>

#include "MitkModelFitExports.h"

namespace mitk
{
  class BaseData;
  class DataStorage;

  typedef std::string NodeUIDType;

  /** Helper that searches for the node in the storage that has a data object with the given uid. Returns NULL if no node was found.*/
  DataNode::Pointer MITKMODELFIT_EXPORT GetNodeByModelFitUID(const mitk::DataStorage* storage, const NodeUIDType& uid);

  /** Gets the UID of the passed data. The UID is stored in the property "data.uid". If this property does not exist the method will
  check the existance of a property for the DICOM tag series instance UID (0x0020, 0x000e) and return the value as UID. If this does also
  not exist, a UID will generated, the property "data.uid" will be added with a new UID and the new UID will be returned.
  @pre Passed data is a valid pointer.*/
  NodeUIDType MITKMODELFIT_EXPORT EnsureModelFitUID(mitk::BaseData* data);
  /** Convinience function that calls the overloaded EnsureModelFitUID() with the data of the passed node.
  @pre Passed node is a valid pointer.*/
  NodeUIDType MITKMODELFIT_EXPORT EnsureModelFitUID(mitk::DataNode* node);

  /** Helper that checks if the data has the passed UID. Following strategy will be used for the check.
   1. Checks existance of property "data.uid" and if its content equals the passed uid.
   2. If 1 fails, it checks the existance of a property for the DICOM tag series instance UID (0x0020, 0x000e) and the equality of its value.
   3. If 2 also fails, return will be false.*/
  bool MITKMODELFIT_EXPORT CheckModelFitUID(const mitk::BaseData* data, const NodeUIDType& uid);
  /** Convinience helper function that calls the overloaded CheckModelFitUID() with the data of the passed node.*/
  bool MITKMODELFIT_EXPORT CheckModelFitUID(const mitk::DataNode* node, const NodeUIDType& uid);


}

#endif

