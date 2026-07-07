/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitknnInteractiveBoundingBoxHelpers.h"

#include <mitkBaseGeometry.h>
#include <mitkImageReadAccessor.h>
#include <mitkLog.h>
#include <mitkPixelType.h>
#include <mitkSegTool2D.h>
#include <mitkSlicedGeometry3D.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

bool mitk::nnInteractive::ComputeStrokeBoundingBox(const Image* paintingSlice2D,
                                                    const Image* referenceImage,
                                                    InteractionBoundingBox& outBoundingBox)
{
  if (paintingSlice2D == nullptr || referenceImage == nullptr)
    return false;

  const auto sliceGeom = paintingSlice2D->GetGeometry();
  const auto refGeom = referenceImage->GetGeometry();
  if (sliceGeom == nullptr || refGeom == nullptr)
    return false;

  if (paintingSlice2D->GetPixelType().GetSize() != 1)
  {
    MITK_WARN << "nnInteractive: ComputeStrokeBoundingBox expects a uint8 slice.";
    return false;
  }

  const auto uDim = static_cast<int>(paintingSlice2D->GetDimension(0));
  const auto vDim = static_cast<int>(paintingSlice2D->GetDimension(1));
  if (uDim <= 0 || vDim <= 0)
    return false;

  // Find the 2D bounds of non-zero pixels in the painting slice.
  ImageReadAccessor srcAcc(paintingSlice2D);
  const auto* data = static_cast<const unsigned char*>(srcAcc.GetData());

  int uMin = uDim;
  int uMax = -1;
  int vMin = vDim;
  int vMax = -1;
  for (int v = 0; v < vDim; ++v)
  {
    const auto* row = data + static_cast<std::size_t>(v) * uDim;
    for (int u = 0; u < uDim; ++u)
    {
      if (row[u] != 0)
      {
        if (u < uMin) uMin = u;
        if (u > uMax) uMax = u;
        if (v < vMin) vMin = v;
        if (v > vMax) vMax = v;
      }
    }
  }

  if (uMax < uMin)
    return false; // No non-zero pixels in the slice.

  // Map each painted pixel-corner of the 2D bounds to MITK index space.
  // Using the four corners of the half-open 2D box catches the entire
  // pixel area (not just centres).
  auto makeCorner = [](double u, double v) {
    Point3D p;
    p[0] = u;
    p[1] = v;
    p[2] = 0.0;
    return p;
  };
  const std::array<Point3D, 4> corners2D = {
    makeCorner(uMin - 0.5, vMin - 0.5),
    makeCorner(uMax + 0.5, vMin - 0.5),
    makeCorner(uMax + 0.5, vMax + 0.5),
    makeCorner(uMin - 0.5, vMax + 0.5)
  };

  std::array<double, 3> idxMinD = { std::numeric_limits<double>::max(),
                                    std::numeric_limits<double>::max(),
                                    std::numeric_limits<double>::max() };
  std::array<double, 3> idxMaxD = { std::numeric_limits<double>::lowest(),
                                    std::numeric_limits<double>::lowest(),
                                    std::numeric_limits<double>::lowest() };

  for (const auto& c2D : corners2D)
  {
    Point3D world;
    sliceGeom->IndexToWorld(c2D, world);
    Point3D refIndex;
    refGeom->WorldToIndex(world, refIndex);
    for (int a = 0; a < 3; ++a)
    {
      idxMinD[a] = std::min(idxMinD[a], refIndex[a]);
      idxMaxD[a] = std::max(idxMaxD[a], refIndex[a]);
    }
  }

  // Reference image dimensions in MITK (X, Y, Z) order.
  const auto* dims = referenceImage->GetDimensions();
  const std::array<int, 3> dimMitk = {
    static_cast<int>(dims[0]),
    static_cast<int>(dims[1]),
    referenceImage->GetDimension() > 2 ? static_cast<int>(dims[2]) : 1
  };

  // Pad by 2 voxels per side on axes where the stroke spans a non-trivial
  // range so VTK reslicing's nearest-neighbour-ish sampling at the
  // staircase boundary still has somewhere to write. On axes where every
  // mapped corner lands within the same reference voxel (axis-aligned
  // slicing axis), pick that integer voxel directly; floor/ceil would be
  // sensitive to floating-point precision and could snap to the wrong
  // neighbour when the reference image has rotated direction cosines.
  // The 0.5-voxel threshold (in reference index units) is the largest span
  // that still rounds to a single voxel along that axis, which is what an
  // axis-aligned plane should produce up to FP noise from the
  // IndexToWorld/WorldToIndex round-trip; using a per-voxel threshold
  // rather than an absolute one avoids regressing into the padded branch
  // on reference volumes with very fine spacing.
  // Final ranges are clamped to the reference's extent.
  constexpr double axisAlignedThreshold = 0.5;
  std::array<std::array<int, 2>, 3> rangeMitk{};
  for (int a = 0; a < 3; ++a)
  {
    const bool axisAligned = (idxMaxD[a] - idxMinD[a]) < axisAlignedThreshold;
    int lo;
    int hi;
    if (axisAligned)
    {
      const int sliceIdx = static_cast<int>(std::round((idxMinD[a] + idxMaxD[a]) * 0.5));
      lo = sliceIdx;
      hi = sliceIdx + 1;
    }
    else
    {
      lo = static_cast<int>(std::floor(idxMinD[a])) - 2;
      hi = static_cast<int>(std::ceil(idxMaxD[a])) + 2;
    }
    lo = std::max(0, lo);
    hi = std::min(dimMitk[a], hi);
    if (hi <= lo)
      return false;
    rangeMitk[a] = { lo, hi };
  }

  // nnInteractive's interaction_bbox is (Z, Y, X) -- reverse of MITK.
  outBoundingBox[0] = rangeMitk[2];
  outBoundingBox[1] = rangeMitk[1];
  outBoundingBox[2] = rangeMitk[0];
  return true;
}

mitk::Image::Pointer mitk::nnInteractive::BuildBoundingBoxMaskImage(const Image* paintingSlice2D,
                                                                    const PlaneGeometry* slicingPlane,
                                                                    const Image* referenceImage,
                                                                    const InteractionBoundingBox& boundingBox)
{
  if (paintingSlice2D == nullptr || slicingPlane == nullptr || referenceImage == nullptr)
    return nullptr;

  const auto refGeom = referenceImage->GetGeometry();
  if (refGeom == nullptr)
    return nullptr;

  // Convert the nnInteractive (Z, Y, X) bounding box into MITK (X, Y, Z) dims.
  const unsigned int dims[3] = {
    static_cast<unsigned int>(boundingBox[2][1] - boundingBox[2][0]),
    static_cast<unsigned int>(boundingBox[1][1] - boundingBox[1][0]),
    static_cast<unsigned int>(boundingBox[0][1] - boundingBox[0][0])
  };

  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    return nullptr;

  const auto uint8Type = MakePixelType<unsigned char, unsigned char, 1>();
  auto out = Image::New();
  out->Initialize(uint8Type, 3, dims);
  out->AllocateZeroedVolume();

  // Configure the first slice's PlaneGeometry with the reference's cloned
  // IndexToWorldTransform (which already encodes spacing in its column
  // magnitudes -- no separate SetSpacing needed) and shift its origin so
  // the small volume's (0,0,0) voxel coincides with the bounding box's
  // first voxel in world space. Then call InitializeEvenlySpaced on the
  // sliced geometry so the parent transform and the per-slice plane
  // geometries are derived consistently from the configured first slice
  // (otherwise GetPlaneGeometry(z) would return planes with a default
  // origin, lazily generated from a stale first slice).
  auto* slicedGeom = out->GetSlicedGeometry(0);
  auto* planeGeom = slicedGeom->GetPlaneGeometry(0);
  planeGeom->SetIndexToWorldTransform(refGeom->GetIndexToWorldTransform()->Clone());

  Point3D originIndex;
  originIndex[0] = boundingBox[2][0];
  originIndex[1] = boundingBox[1][0];
  originIndex[2] = boundingBox[0][0];
  Point3D originWorld;
  refGeom->IndexToWorld(originIndex, originWorld);
  planeGeom->SetOrigin(originWorld);

  slicedGeom->InitializeEvenlySpaced(planeGeom, dims[2]);

  // WriteSliceToVolume uses VTK reslicing to place the 2D painting slice
  // at its plane's world position into the volume. With our small volume
  // only containing the single slice layer, the slice is written into the
  // volume's single layer along its thin axis.
  SegTool2D::WriteSliceToVolume(out, slicingPlane, paintingSlice2D, 0);

  return out;
}
