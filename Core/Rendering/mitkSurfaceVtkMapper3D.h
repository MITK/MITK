/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273
#define MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkSurface.h"
#include "mitkBaseRenderer.h"

class vtkActor;
class vtkPolyDataMapper;

namespace mitk {

//##ModelId=3E70F60202EA
//##Documentation
//## @brief Vtk-based mapper for Surface
//## @ingroup Mapper
class SurfaceVtkMapper3D : public BaseVtkMapper3D
{
public:

  mitkClassMacro(SurfaceVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  //##ModelId=3E70F60301D5
  virtual const mitk::Surface* GetInput();

  //##ModelId=3E70F60301E9
  virtual void GenerateData();

  //##ModelId=3EF19FA803BF
  //##Documentation
  //## @brief Called by BaseRenderer when an update is required
  virtual void Update(mitk::BaseRenderer* renderer);
  //##ModelId=3EF1B44001D5
  virtual void Update();

  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);

protected:
  //##ModelId=3E70F60301F2
  virtual void GenerateOutputInformation();
  //##ModelId=3E70F60301F4
  SurfaceVtkMapper3D();

  //##ModelId=3E70F60301F5
  virtual ~SurfaceVtkMapper3D();

  //##ModelId=3E70F60301C9
  vtkActor* m_Actor;

  //##ModelId=3E70F60301CA
  vtkPolyDataMapper* m_VtkPolyDataMapper;
};

} // namespace mitk

#endif /* MITKSURFACEDATAVTKMAPPER3D_H_HEADER_INCLUDED_C1907273 */
