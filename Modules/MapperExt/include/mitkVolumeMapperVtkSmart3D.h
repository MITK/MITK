/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeMapperVtkSmart3D_h
#define mitkVolumeMapperVtkSmart3D_h

// MITK
#include <MitkMapperExtExports.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkVersionMacros.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageData.h>

namespace mitk
{

  /** \brief VTK-based mapper for volume rendering of 3D image data.
   *
   * Uses vtkSmartVolumeMapper which automatically selects the best volume
   * rendering method (GPU ray casting, software ray casting, etc.) based on
   * hardware capabilities. Transfer functions for color and opacity are
   * configured from the DataNode's TransferFunction property.
   *
   * \sa ImageVtkMapper2D, VtkMapper
   * \ingroup Mapper
   */
  class MITKMAPPEREXT_EXPORT VolumeMapperVtkSmart3D : public VtkMapper
  {
  public:
    mitkClassMacro(VolumeMapperVtkSmart3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \brief Get the VTK volume prop for 3D rendering.
     *
     * \param[in] renderer The renderer context.
     * \return The VTK volume prop.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    /** \brief Set default rendering properties for volume visualization.
     *
     * \param[in] node      The data node to configure.
     * \param[in] renderer  The renderer context, or \c nullptr for all renderers.
     * \param[in] overwrite If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Place the volume, without applying its spacing a second time.
     *
     * The ray caster derives the shading gradient, the sample distance and the
     * extinction per unit length from the input image's spacing, so this mapper
     * leaves the real spacing on the image rather than resetting it. IndexToWorld
     * carries that same spacing, so the base class transform would scale the
     * volume by it twice; this override divides it back out.
     *
     * \param[in] renderer The renderer context.
     */
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;

    /** \brief The VTK objects one 3D render window renders this volume with.
     *
     * A single mapper instance serves every renderer that shows the node, and
     * every property it reads is renderer-scoped. Holding the objects it writes
     * those reads into per renderer is what keeps two 3D windows from
     * overwriting each other's visibility, placement and shading.
     */
    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      /** \brief The prop handed to the renderer. */
      vtkSmartPointer<vtkVolume> m_Volume;
      /** \brief Places the volume, with the image spacing divided back out. */
      vtkSmartPointer<vtkTransform> m_DataToWorld;
      /** \brief The ray caster. */
      vtkSmartPointer<vtkSmartVolumeMapper> m_SmartVolumeMapper;
      /** \brief Transfer functions and shading coefficients. */
      vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;

      LocalStorage();
      ~LocalStorage() override;
    };

  protected:
    VolumeMapperVtkSmart3D();
    ~VolumeMapperVtkSmart3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    vtkImageData* GetInputImage();

    void UpdateTransferFunctions(mitk::BaseRenderer *renderer, LocalStorage *localStorage);
    void UpdateRenderMode(mitk::BaseRenderer *renderer, LocalStorage *localStorage);

    mitk::LocalStorageHandler<LocalStorage> m_LSH;
  };

} // namespace mitk

#endif
