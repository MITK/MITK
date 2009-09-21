/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_
#define _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkUnstructuredGrid.h"
#include "mitkBaseRenderer.h"

#include <vtkAssembly.h>
#include <vtkActor.h>
#include <vtkVolume.h>
#include <vtkDataSetTriangleFilter.h>

#include "vtkUnstructuredGridMapper.h"
#include <vtkUnstructuredGridVolumeRayCastMapper.h>

#if (VTK_MAJOR_VERSION >= 5)
  #include <vtkProjectedTetrahedraMapper.h>
  #include <vtkUnstructuredGridVolumeZSweepMapper.h>
#endif


namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for UnstructuredGrid
//##
//## @ingroup Mapper
class MITKEXT_CORE_EXPORT UnstructuredGridVtkMapper3D : public BaseVtkMapper3D
{
public:

  mitkClassMacro(UnstructuredGridVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::UnstructuredGrid* GetInput();

  virtual vtkProp* GetVtkProp(mitk::BaseRenderer* renderer);
  

protected:
  
  UnstructuredGridVtkMapper3D();

  virtual ~UnstructuredGridVtkMapper3D();

  virtual void GenerateData();
  virtual void GenerateData(mitk::BaseRenderer* renderer);
  
  void SetProperties(mitk::BaseRenderer* renderer);

  vtkAssembly* m_Assembly;
  vtkActor* m_Actor;
  vtkActor* m_ActorWireframe;
  vtkVolume* m_Volume;
  
  vtkDataSetTriangleFilter* m_VtkTriangleFilter;

  vtkUnstructuredGridMapper* m_VtkDataSetMapper;
  vtkUnstructuredGridMapper* m_VtkDataSetMapper2;

  vtkUnstructuredGridVolumeRayCastMapper* m_VtkVolumeRayCastMapper;
  
  #if (VTK_MAJOR_VERSION >= 5)
  vtkProjectedTetrahedraMapper* m_VtkPTMapper;
  vtkUnstructuredGridVolumeZSweepMapper* m_VtkVolumeZSweepMapper;
  #endif
};

} // namespace mitk

#endif /* _MITK_UNSTRUCTURED_GRID_VTK_MAPPER_3D_H_ */
