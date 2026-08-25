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

  protected:
    VolumeMapperVtkSmart3D();
    ~VolumeMapperVtkSmart3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void createMapper(vtkImageData*);
    void createVolume();
    void createVolumeProperty();
    vtkImageData* GetInputImage();

    vtkSmartPointer<vtkVolume> m_Volume;
    vtkSmartPointer<vtkTransform> m_DataToWorld;
    vtkSmartPointer<vtkSmartVolumeMapper> m_SmartVolumeMapper;
    vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;

    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);
    void UpdateRenderMode(mitk::BaseRenderer *renderer);
  };

} // namespace mitk

#endif
