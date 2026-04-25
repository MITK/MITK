/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveBoundingBox_h
#define mitknnInteractiveBoundingBox_h

#include <mitkImage.h>
#include <mitkPlaneGeometry.h>

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

  // Computes a tight axis-aligned bounding box in MITK index space around
  // the non-zero footprint of the 2D painting slice, by mapping the slice's
  // non-zero 2D bounds through its own geometry into the reference image's
  // index space. Pads by a couple of voxels to absorb VTK reslicing
  // boundary effects on axes the stroke spans non-trivially. Works for any
  // slice orientation (axial / coronal / sagittal / oblique).
  //
  // Returns false if the slice has no non-zero pixels or no usable geometry.
  // On success, outBoundingBox is in nnInteractive's (Z, Y, X) order.
  bool ComputeStrokeBoundingBox(const Image* paintingSlice2D,
                                const Image* referenceImage,
                                InteractionBoundingBox& outBoundingBox);

  // Builds a small 3D uint8 mitk::Image sized exactly to the given bounding
  // box and transfers the 2D painting slice into it using MITK's plane-aware
  // VTK reslicing (SegTool2D::WriteSliceToVolume). The returned image's
  // geometry is derived from the reference image: its origin sits at the
  // box's first voxel and its spacing/direction match the reference, so
  // numpy sees it with shape matching the box sizes in nnInteractive's
  // (Z, Y, X) order.
  //
  // The source's non-zero pixels become 1; zero pixels stay 0. Designed for
  // forwarding to nnInteractive's add_*_interaction(..., interaction_bbox=).
  Image::Pointer BuildBoundingBoxMaskImage(const Image* paintingSlice2D,
                                           const PlaneGeometry* slicingPlane,
                                           const Image* referenceImage,
                                           const InteractionBoundingBox& boundingBox);
}

#endif
