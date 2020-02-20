/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKGPUVOLUMEMAPPER3D_H_HEADER_INCLUDED
#define MITKGPUVOLUMEMAPPER3D_H_HEADER_INCLUDED

// MITK
#include "MitkMapperExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkVtkMapper.h"

// VTK
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>
#include <vtkVersionMacros.h>
#include <vtkVolumeProperty.h>

#include "vtkMitkGPUVolumeRayCastMapper.h"

namespace mitk
{
  /************************************************************************/
  /* Properties that influence the mapper are:
  *
  * - \b "level window": for the level window of the volume data
  * - \b "LookupTable" : for the lookup table of the volume data
  * - \b "TransferFunction" (mitk::TransferFunctionProperty): for the used transfer function of the volume data
  ************************************************************************/

  //##Documentation
  //## @brief Vtk-based mapper for VolumeData
  //##
  //## @ingroup Mapper
  class MITKMAPPEREXT_EXPORT GPUVolumeMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(GPUVolumeMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::Image *GetInput();

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    void ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer) override;
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** Returns true if this Mapper currently allows for Level-of-Detail rendering.
     * This reflects whether this Mapper currently invokes StartEvent, EndEvent, and
     * ProgressEvent on BaseRenderer. */
    bool IsLODEnabled(BaseRenderer *renderer = nullptr) const override;
    bool IsMIPEnabled(BaseRenderer *renderer = nullptr);
    bool IsGPUEnabled(BaseRenderer *renderer = nullptr);
    bool IsRAYEnabled(BaseRenderer *renderer = nullptr);

    void MitkRenderVolumetricGeometry(mitk::BaseRenderer *renderer) override;

  protected:
    GPUVolumeMapper3D();
    ~GPUVolumeMapper3D() override;

    bool IsRenderable(mitk::BaseRenderer *renderer);

    void InitCommon();
    void DeinitCommon();

    void InitCPU(mitk::BaseRenderer *renderer);
    void DeinitCPU(mitk::BaseRenderer *renderer);
    void GenerateDataCPU(mitk::BaseRenderer *renderer);

    bool InitGPU(mitk::BaseRenderer *renderer);
    void DeinitGPU(mitk::BaseRenderer *);
    void GenerateDataGPU(mitk::BaseRenderer *);

    bool InitRAY(mitk::BaseRenderer *renderer);
    void DeinitRAY(mitk::BaseRenderer *renderer);
    void GenerateDataRAY(mitk::BaseRenderer *renderer);

    void InitVtkMapper(mitk::BaseRenderer *renderer);

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void CreateDefaultTransferFunctions();
    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);

    vtkSmartPointer<vtkVolume> m_Volumenullptr;

    bool m_commonInitialized;
    vtkSmartPointer<vtkImageChangeInformation> m_UnitSpacingImageFilter;
    vtkSmartPointer<vtkPiecewiseFunction> m_DefaultOpacityTransferFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_DefaultGradientTransferFunction;
    vtkSmartPointer<vtkColorTransferFunction> m_DefaultColorTransferFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_BinaryOpacityTransferFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_BinaryGradientTransferFunction;
    vtkSmartPointer<vtkColorTransferFunction> m_BinaryColorTransferFunction;

    class LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
      // NO SMARTPOINTER HERE
      vtkRenderWindow *m_VtkRenderWindow;

      bool m_cpuInitialized;
      vtkSmartPointer<vtkVolume> m_VolumeCPU;
      vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> m_MapperCPU;
      vtkSmartPointer<vtkVolumeProperty> m_VolumePropertyCPU;

      bool m_gpuSupported;
      bool m_gpuInitialized;
      vtkSmartPointer<vtkVolume> m_VolumeGPU;
      vtkSmartPointer<vtkVolumeProperty> m_VolumePropertyGPU;

      bool m_raySupported;
      bool m_rayInitialized;
      vtkSmartPointer<vtkVolume> m_VolumeRAY;
      vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_MapperRAY;
      vtkSmartPointer<vtkVolumeProperty> m_VolumePropertyRAY;

      LocalStorage()
      {
        m_VtkRenderWindow = nullptr;

        m_cpuInitialized = false;

        m_gpuInitialized = false;
        m_gpuSupported = true; // assume initially gpu slicing is supported

        m_rayInitialized = false;
        m_raySupported = true; // assume initially gpu raycasting is supported
      }

      ~LocalStorage() override
      {
        if (m_cpuInitialized && m_MapperCPU && m_VtkRenderWindow)
          m_MapperCPU->ReleaseGraphicsResources(m_VtkRenderWindow);

        if (m_rayInitialized && m_MapperRAY && m_VtkRenderWindow)
          m_MapperRAY->ReleaseGraphicsResources(m_VtkRenderWindow);
      }
    };

    mitk::LocalStorageHandler<LocalStorage> m_LSH;
  };

} // namespace mitk

#endif /* MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED */
