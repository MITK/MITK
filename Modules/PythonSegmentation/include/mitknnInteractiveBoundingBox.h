/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveBoundingBox_h
#define mitknnInteractiveBoundingBox_h

#include <array>

namespace mitk::nnInteractive
{
  // Axis-aligned bounding box in nnInteractive's coordinate system, which
  // is the MITK index space axis-reversed: first entry is along MITK's Z,
  // then Y, then X. This mirrors the existing AddPointInteraction
  // convention where [index[2], index[1], index[0]] is passed to Python.
  // Passed verbatim to session.add_*_interaction's interaction_bbox=
  // parameter.
  using InteractionBoundingBox = std::array<std::array<int, 2>, 3>;
}

#endif
