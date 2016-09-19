/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __DOSE_NODE_HELPER_H
#define __DOSE_NODE_HELPER_H

#include "mitkIsoDoseLevelCollections.h"
#include "mitkDataNode.h"
#include "MitkDicomRTExports.h"

namespace mitk
{
  void MITKDICOMRT_EXPORT ConfigureNodeAsDoseNode(mitk::DataNode* node, mitk::DoseValueAbs referenceDose);
}

#endif
