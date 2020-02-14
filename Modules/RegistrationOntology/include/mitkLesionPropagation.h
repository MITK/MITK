/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLESIONPROPAGATION_H
#define MITKLESIONPROPAGATION_H

// registration ontology module
#include "MitkRegistrationOntologyExports.h"

// mitk core
#include <mitkDataNode.h>

// matchpoint ontology
#include <mapSimpleOntologyCore.h>
#include <mapSimpleOntology.h>

namespace mitk
{
  MITKREGISTRATIONONTOLOGY_EXPORT void FindClosestSegmentationMask();
} // namespace mitk

#endif // MITKLESIONPROPAGATION_H
