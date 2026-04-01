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
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>
#include <vtkVersionMacros.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageData.h>
#include <vtkImageChangeInformation.h>

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

    /** \brief Apply visual properties to the actor.
     *
     * \param[in] actor    The VTK actor to apply properties to.
     * \param[in] renderer The renderer context.
     */
    void ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer) override;

    /** \brief Set default rendering properties for volume visualization.
     *
     * \param[in] node      The data node to configure.
     * \param[in] renderer  The renderer context, or \c nullptr for all renderers.
     * \param[in] overwrite If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

  protected:
    VolumeMapperVtkSmart3D();
    ~VolumeMapperVtkSmart3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void createMapper(vtkImageData*);
    void createVolume();
    void createVolumeProperty();
    vtkImageData* GetInputImage();

    vtkSmartPointer<vtkVolume> m_Volume;
    vtkSmartPointer<vtkImageChangeInformation> m_ImageChangeInformation;
    vtkSmartPointer<vtkSmartVolumeMapper> m_SmartVolumeMapper;
    vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;

    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);
    void UpdateRenderMode(mitk::BaseRenderer *renderer);
  };

} // namespace mitk

#endif
