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

class vtkPolyDataMapper;
class vtkImageData;
class vtkLookupTable;
class vtkVolumeProperty;
class vtkVolume;
class vtkSmartVolumeMapper;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;

namespace mitk
{
  class MultiLabelSegmentationGroupMapping;
  class IPreferences;

  /** \brief Mapper to resample and display 2D slices of a 3D labelset image.
   *
   * Properties that can be set for labelset images and influence this mapper are:
   *
   *   - \b "labelset.contour.active": (BoolProperty) whether to show only the active label as a contour or not
   *   - \b "labelset.contour.width": (FloatProperty) line width of the contour

   * The default properties are:

   *   - \b "labelset.contour.active", mitk::BoolProperty::New( true ), renderer, overwrite )
   *   - \b "labelset.contour.width", mitk::FloatProperty::New( 2.0 ), renderer, overwrite )

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

    /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
    /**
       * To render axial, coronal, and sagittal, the mapper is called three times.
       * For performance reasons, the corresponding data for each view is saved in the
       * internal helper class LocalStorage. This allows rendering n views with just
       * 1 mitkMapper using n vtkMapper.
       * */
    class MITKMULTILABEL_EXPORT LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief Assembly of all VTK actors used for rendering. */
      vtkSmartPointer<vtkPropAssembly> m_Actors;

      /** \brief Map from group images to their respective rendering pipelines. */
      std::map<const Image*, std::unique_ptr<MultiLabelSegmentationGroupMapping>> m_GroupPipelines;

      /** \brief Color transfer function for label rendering. */
      vtkSmartPointer<vtkColorTransferFunction> m_TransferFunction;
      /** \brief Opacity transfer function for label rendering. */
      vtkSmartPointer<vtkPiecewiseFunction> m_OpacityTransferFunction;
      /** \brief Faded opacity transfer function for non-highlighted labels. */
      vtkSmartPointer<vtkPiecewiseFunction> m_FadedOpacityTransferFunction;

      /** \brief Indicates if highlighting is in use and therefore the faded
       * pipeline should be used for non-highlighted labels.
       */
      bool m_UseFadedPipeline;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastDataUpdateTime;
      /** \brief Timestamp of last update of a property. */
      itk::TimeStamp m_LastPropertyUpdateTime;

      /** \brief The last time step that was updated. */
      mitk::TimeStepType m_LastUpdateTimeStep;

      /** \brief Look up table for label colors. */
      mitk::LookupTable::Pointer m_LabelLookupTable;

      /** \brief Indicates if GPU is available for the mapper.
       *
       * True: Yes, mapper will work. False: No, mapper will not render something.
       * If optional has no value it means that no check was done so far.
       */
      std::optional<bool> m_GPUCheckSuccessfull;
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

    /** \brief Does the actual resampling, without rendering the image yet.
      * All the data is generated inside this method. The vtkProp (or Actor)
      * is filled with content (i.e. the resliced image).
      *
      * After generation, a 4x4 transformation matrix(t) of the current slice is obtained
      * from the vtkResliceImage object via GetReslicesAxis(). This matrix is
      * applied to each textured plane (actor->SetUserTransform(t)) to transform everything
      * to the actual 3D position (cf. the following image).
      *
      * \image html cameraPositioning3D.png
      *
      */
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

    /** \brief Update the volume mapping for the given outdated groups.
      * \param localStorage The local storage to update.
      * \param outdatedData The outdated group data to process.
      */
    void UpdateVolumeMapping(LocalStorage* localStorage, const OutdatedGroupVectorType& outdatedData);

    /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LSH;
  };

} // namespace mitk

#endif
