/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLesionPropagation_h
#define mitkLesionPropagation_h

// registration ontology module
#include <MitkRegistrationOntologyExports.h>

// mitk core
#include <mitkDataNode.h>

// matchpoint ontology
#include <mapSimpleOntologyCore.h>
#include <mapSimpleOntology.h>

namespace mitk
{
  /**
   * \brief Find the closest segmentation mask for lesion propagation.
   *
   * Uses the MatchPoint registration ontology to locate the segmentation mask
   * that is closest to the given lesion in the registration graph.
   *
   * \note Currently a stub/placeholder implementation.
   */
  MITKREGISTRATIONONTOLOGY_EXPORT void FindClosestSegmentationMask();
} // namespace mitk

#endif
