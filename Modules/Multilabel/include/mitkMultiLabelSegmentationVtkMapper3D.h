/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelSegmentationVtkMapper3D_h
#define mitkMultiLabelSegmentationVtkMapper3D_h

// MITK
#include <MitkMultilabelExports.h>
#include <mitkCommon.h>

// MITK Rendering
#include <mitkBaseRenderer.h>
#include <mitkExtractSliceFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkVtkMapper.h>

// VTK
#include <vtkSmartPointer.h>

class vtkActor;
class vtkImageData;
class vtkLookupTable;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkSurfaceNets3D;

namespace mitk
{
  class MultiLabelSegmentationGroupMapping;
  class IPreferences;

  /** \brief 3D mapper for mitk::MultiLabelSegmentation.
   *
   * Renders all label groups of a multi-label segmentation as smooth polygonal
   * surfaces, using vtkSurfaceNets3D to extract a single shared mesh per group
   * (one execution covers all labels in that group). The internal constrained
   * smoothing filter preserves sharp inter-label boundaries while removing voxel
   * staircasing.
   *
   * Properties consumed:
   *   - "visible" (BoolProperty)
   *   - "opacity" (FloatProperty)
   *   - "org.mitk.multilabel.3D.hide" (BoolProperty)
   *   - "/org.mitk.views.segmentation" -> "activate 3D rendering" preference
   *   - LabelHighlightGuard properties to fade non-highlighted labels
   *
   * \ingroup Mapper
   */
  class MITKMULTILABEL_EXPORT MultiLabelSegmentationVtkMapper3D : public VtkMapper
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacro(MultiLabelSegmentationVtkMapper3D, VtkMapper);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** \brief Checks whether this mapper needs to update itself and generate
     * data. */
    void Update(mitk::BaseRenderer *renderer) override;

    /** \brief Get the VTK prop for MITK-VTK rendering pipeline.
     * \param renderer The renderer to get the prop for.
     * \return The vtkProp for rendering.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Per-renderer cache of VTK actors and the surface-extraction state. */
    class MITKMULTILABEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Assembly of all VTK actors used for rendering (one actor per group). */
      vtkSmartPointer<vtkPropAssembly> m_Actors;

      /** \brief Map from group images to their respective rendering pipelines. */
      std::map<const Image*, std::unique_ptr<MultiLabelSegmentationGroupMapping>> m_GroupPipelines;

      /** \brief Lookup table keyed by integer label value, shared by all per-group mappers.
       *
       * Replaces the previous color/opacity transfer-function trio. Highlighting is encoded
       * here directly via the alpha channel: highlighted labels are forced opaque and
       * non-highlighted labels are faded.
       */
      vtkSmartPointer<vtkLookupTable> m_VtkLookupTable;

      /** \brief Indicates whether label highlighting is active.
       *
       * Kept as a flag so CheckForOutdatedGroups can detect highlight-mode toggles, even
       * though there is no longer a separate faded actor.
       */
      bool m_UseFadedPipeline;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastDataUpdateTime;
      /** \brief Timestamp of last update of a property. */
      itk::TimeStamp m_LastPropertyUpdateTime;

      /** \brief The last time step that was updated. */
      mitk::TimeStepType m_LastUpdateTimeStep;

      /** \brief Look up table for label colors (cloned from the segmentation). */
      mitk::LookupTable::Pointer m_LabelLookupTable;

      /** \brief Whether 3D rendering is preferred. */
      bool m_3DRenderingPreference;

      /** \brief Pointer to the segmentation preferences. */
      IPreferences* m_SegPreferences;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default destructor of the local storage. */
      ~LocalStorage() override;
    };

    /** \brief Get the LocalStorage corresponding to the current renderer. */
    LocalStorage* GetLocalStorage(mitk::BaseRenderer* renderer);

    /** \brief Set the default properties for multilabel segmentation rendering.
     * \param node The data node to set the properties on.
     * \param renderer The renderer for renderer-specific properties, or nullptr for global properties.
     * \param overwrite If true, existing properties will be overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = nullptr, bool overwrite = false);

  protected:
    /** \brief Default constructor. */
    MultiLabelSegmentationVtkMapper3D();
    /** \brief Default destructor. */
    ~MultiLabelSegmentationVtkMapper3D() override;

    /** \brief Drives surface re-extraction and lookup-table refresh for all outdated groups. */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /** \brief Generates the look up table that should be used.
      */
    void UpdateLookupTable(LocalStorage* localStorage);

    /** \brief Type for a vector of outdated group indices paired with their images. */
    using OutdatedGroupVectorType = std::vector<std::pair<mitk::MultiLabelSegmentation::GroupIndexType, const mitk::Image*>>;
    /** \brief Check if groups are outdated or obsolete.
      *
      * Obsolete groups will be removed. Outdated groups will be indicated
      * in the output as such. New groups will be added to the local storage and also
      * marked as outdated.
      *
      * \param ls The local storage to check.
      * \param seg The multi-label segmentation to check against.
      * \param fadedPipelineChanged Whether the faded pipeline state has changed.
      * \return A vector of outdated group index / image pairs.
      */
    OutdatedGroupVectorType CheckForOutdatedGroups(mitk::MultiLabelSegmentationVtkMapper3D::LocalStorage* ls,
      mitk::MultiLabelSegmentation* seg, bool fadedPipelineChanged);

    /** \brief Update the surface mapping for the given outdated groups.
      * \param localStorage The local storage to update.
      * \param outdatedData The outdated group data to process.
      */
    void UpdateSurfaceMapping(LocalStorage* localStorage, const OutdatedGroupVectorType& outdatedData);

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;
  };

} // namespace mitk

#endif
