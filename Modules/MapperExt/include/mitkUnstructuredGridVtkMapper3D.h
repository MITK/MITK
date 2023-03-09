/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridVtkMapper3D_h
#define mitkUnstructuredGridVtkMapper3D_h

#include "MitkMapperExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkUnstructuredGrid.h"
#include "mitkVtkMapper.h"

#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkVolume.h>

#include "vtkUnstructuredGridMapper.h"
#include <vtkUnstructuredGridVolumeRayCastMapper.h>

#include <vtkProjectedTetrahedraMapper.h>
#include <vtkUnstructuredGridVolumeZSweepMapper.h>

namespace mitk
{
  //##Documentation
  //## @brief Vtk-based mapper for UnstructuredGrid
  //##
  //## @ingroup Mapper
  class MITKMAPPEREXT_EXPORT UnstructuredGridVtkMapper3D : public VtkMapper
  {
  public:
    mitkClassMacro(UnstructuredGridVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      virtual const mitk::UnstructuredGrid *GetInput();

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    void ApplyProperties(vtkActor * /*actor*/, mitk::BaseRenderer *renderer) override;

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    UnstructuredGridVtkMapper3D();

    ~UnstructuredGridVtkMapper3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    void ResetMapper(BaseRenderer * /*renderer*/) override;

    void SetProperties(mitk::BaseRenderer *renderer);

    vtkAssembly *m_Assembly;
    vtkActor *m_Actor;
    vtkActor *m_ActorWireframe;
    vtkVolume *m_Volume;

    vtkDataSetTriangleFilter *m_VtkTriangleFilter;

    vtkUnstructuredGridMapper *m_VtkDataSetMapper;
    vtkUnstructuredGridMapper *m_VtkDataSetMapper2;

    vtkUnstructuredGridVolumeRayCastMapper *m_VtkVolumeRayCastMapper;
    vtkProjectedTetrahedraMapper *m_VtkPTMapper;
    vtkUnstructuredGridVolumeZSweepMapper *m_VtkVolumeZSweepMapper;
  };

} // namespace mitk

#endif
