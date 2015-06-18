/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_
#define _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_

#include "mitkCommon.h"
#include "MitkMapperExtExports.h"
#include "mitkVtkMapper.h"
#include "mitkUnstructuredGrid.h"
#include "mitkBaseRenderer.h"

#include <vtkAssembly.h>
#include <vtkActor.h>
#include <vtkVolume.h>
#include <vtkDataSetTriangleFilter.h>

#include "vtkUnstructuredGridMapper.h"
#include <vtkUnstructuredGridVolumeRayCastMapper.h>

#include <vtkProjectedTetrahedraMapper.h>
#include <vtkUnstructuredGridVolumeZSweepMapper.h>



namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for UnstructuredGrid
//##
//## @ingroup Mapper
class MITKMAPPEREXT_EXPORT UnstructuredGridVtkMapper3D : public VtkMapper
{
public:

  mitkClassMacro(UnstructuredGridVtkMapper3D, VtkMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual const mitk::UnstructuredGrid* GetInput();

  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  void ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* renderer) override;

  LocalStorageHandler<BaseLocalStorage> m_LSH;

protected:

  UnstructuredGridVtkMapper3D();

  virtual ~UnstructuredGridVtkMapper3D();

  virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;
  virtual void ResetMapper( BaseRenderer* /*renderer*/ ) override;

  void SetProperties(mitk::BaseRenderer* renderer);

  vtkAssembly* m_Assembly;
  vtkActor* m_Actor;
  vtkActor* m_ActorWireframe;
  vtkVolume* m_Volume;

  vtkDataSetTriangleFilter* m_VtkTriangleFilter;

  vtkUnstructuredGridMapper* m_VtkDataSetMapper;
  vtkUnstructuredGridMapper* m_VtkDataSetMapper2;

  vtkUnstructuredGridVolumeRayCastMapper* m_VtkVolumeRayCastMapper;
  vtkProjectedTetrahedraMapper* m_VtkPTMapper;
  vtkUnstructuredGridVolumeZSweepMapper* m_VtkVolumeZSweepMapper;
};

} // namespace mitk

#endif /* _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_ */
