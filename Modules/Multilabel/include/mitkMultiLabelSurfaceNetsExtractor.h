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
class vtkPolyData;
class vtkPolyDataNormals;
class vtkSurfaceNets3D;

namespace mitk
{
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
   * The class also encapsulates a workaround for a vtkSurfaceNets3D bug in VTK 9.5.2,
   * where the boundary cache may be reused without populating the local newScalars,
   * causing TransformMeshType to dereference null in the smoothed path.
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
     * \brief Per-label variant. Returns one polydata per label, restricted to that
     * label's exterior faces (vtkSurfaceNets3D OUTPUT_STYLE_SELECTED).
     *
     * Smoothing is applied across all labels in the input list, so the per-label
     * output keeps consistent boundaries with adjacent labels. Labels with no
     * boundary in the group are absent from the returned map.
     */
    std::map<LabelValueType, vtkSmartPointer<vtkPolyData>> ExtractPerLabel(
      vtkImageData* groupImage,
      const std::vector<LabelValueType>& labelValues);

  private:
    void ConfigureLabels(const std::vector<LabelValueType>& labelValues);

    vtkSmartPointer<vtkSurfaceNets3D> m_SurfaceNets;
    vtkSmartPointer<vtkPolyDataNormals> m_NormalsFilter;
    bool m_Smoothing;
  };

} // namespace mitk

#endif
