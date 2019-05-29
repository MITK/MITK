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
