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

#ifndef MITKNODEIDENTIFIER_H
#define MITKNODEIDENTIFIER_H

// semantic relation module
#include "mitkSemanticTypes.h"

// mitk core
#include <mitkDataNode.h>

namespace NodeIdentifier
{

  SemanticTypes::CaseID GetCaseIDFromData(const mitk::DataNode* dataNode);
  SemanticTypes::ID GetIDFromData(const mitk::DataNode* dataNode);

} // namespace NodeIdentifier

#endif // MITKNODEIDENTIFIER_H
