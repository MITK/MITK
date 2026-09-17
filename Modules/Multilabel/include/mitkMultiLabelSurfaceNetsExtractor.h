/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSurfaceNetsExtractor_h
#define mitkMultiLabelSurfaceNetsExtractor_h

#include <MitkMultilabelExports.h>
#include <mitkLabelSetImage.h>

#include <vtkSmartPointer.h>

#include <map>
#include <vector>

class vtkImageData;
class vtkMatrix4x4;
class vtkPolyData;
class vtkPolyDataNormals;
class vtkSurfaceNets3D;

namespace mitk
{
  class BaseGeometry;
  /**
   * \brief Extracts label-boundary polygonal surfaces from a multi-label segmentation
   * group image using vtkSurfaceNets3D.
   *
   * Used by MultiLabelSegmentationVtkMapper3D for live 3D rendering and by the
   * "Convert to polygon model" action for one-shot mitk::Surface generation.
   * vtkSurfaceNets3D extracts all labels in a single threaded pass and shares cells
   * between adjacent regions, producing gap-free, label-aware meshes.
   *
   * Output cells carry a 2-component "BoundaryLabels" cell-data array that
   * vtkSurfaceNets3D writes; component 0 holds the foreground label of each face
   * (background or higher labels go to component 1).
   *
   * The class also encapsulates a workaround for a vtkSurfaceNets3D caching hazard
   * (present up to at least VTK 9.7): label or input changes alone do not invalidate
   * its boundary cache, and the reused cache crashes the smoothed triangle path.
   */
  class MITKMULTILABEL_EXPORT MultiLabelSurfaceNetsExtractor
  {
  public:
    using LabelValueType = MultiLabelSegmentation::LabelValueType;

    MultiLabelSurfaceNetsExtractor();
    ~MultiLabelSurfaceNetsExtractor();

    MultiLabelSurfaceNetsExtractor(const MultiLabelSurfaceNetsExtractor&) = delete;
    MultiLabelSurfaceNetsExtractor& operator=(const MultiLabelSurfaceNetsExtractor&) = delete;

    /**
     * \brief Set whether constrained smoothing is applied.
     *
     * When on (default) the output is smoothed triangles; when off the output is the
     * raw axis-aligned voxel-boundary quads.
     */
    void SetSmoothing(bool smoothing);
    bool GetSmoothing() const;

    /**
     * \brief Extract the boundary surface for the given labels of a single group image.
     *
     * \param groupImage  vtkImageData of one group of a MultiLabelSegmentation
     *                    (typically obtained via Image::GetVtkImageData(timeStep)).
     * \param labelValues The labels to include. An empty list returns an empty mesh.
     * \return            A fresh polydata; safe to retain.
     */
    vtkSmartPointer<vtkPolyData> Extract(
      vtkImageData* groupImage,
      const std::vector<LabelValueType>& labelValues);

    /**
     * \brief Per-label variant. Returns one polydata per label, restricted to the
     * cells bounding that label (both towards background and towards other labels).
     *
     * The surface net is extracted once for all labels in the input list and split
     * per label by vtkSurfaceNetsAtlas, so the per-label output keeps consistent
     * boundaries with adjacent labels. Component 0 of "BoundaryLabels" is always the
     * label itself and normals point outward. Labels with no boundary in the group
     * are absent from the returned map.
     */
    std::map<LabelValueType, vtkSmartPointer<vtkPolyData>> ExtractPerLabel(
      vtkImageData* groupImage,
      const std::vector<LabelValueType>& labelValues);

    /**
     * \brief Build the image-local-frame to world-frame transform for the polydata produced by Extract.
     *
     * vtkSurfaceNets3D operates on the raw vtkImageData (origin (0,0,0), identity direction,
     * image spacing already applied) and returns polydata in that local frame. The returned
     * 4x4 matrix is the [direction | origin] transform that places those coordinates back
     * into world space. The live 3D mapper applies it as the actor's UserMatrix; consumers
     * of the polydata that bypass the rendering pipeline (e.g. the convert-to-surface action)
     * should bake it into the polydata via vtkTransformFilter.
     */
    static vtkSmartPointer<vtkMatrix4x4> GetImageToWorldMatrix(const BaseGeometry* geometry);

  private:
    void UpdateSurfaceNets(vtkImageData* groupImage, const std::vector<LabelValueType>& labelValues);

    vtkSmartPointer<vtkSurfaceNets3D> m_SurfaceNets;
    vtkSmartPointer<vtkPolyDataNormals> m_NormalsFilter;
    bool m_Smoothing;
  };

} // namespace mitk

#endif
