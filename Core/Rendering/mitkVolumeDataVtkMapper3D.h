/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED
#define MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkBaseRenderer.h"
#include "mitkImage.h"
#include <vtkVolumeProperty.h>

class vtkVolumeRayCastMapper;
class vtkVolumeTextureMapper2D;
class vtkVolume;
class vtkObject;
class vtkImageShiftScale;
class vtkImageChangeInformation;

namespace mitk {


//##Documentation
//## @brief Vtk-based mapper for VolumeData
//## @ingroup Mapper
class VolumeDataVtkMapper3D : public BaseVtkMapper3D
{
public:

  mitkClassMacro(VolumeDataVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::Image* GetInput();

  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);
  static void AbortCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);
protected:


  VolumeDataVtkMapper3D();


  virtual ~VolumeDataVtkMapper3D();

  virtual void GenerateData(mitk::BaseRenderer* renderer);

  vtkActor* m_Actor;
  vtkVolume* m_Volume; 
  vtkImageShiftScale* m_ImageCast;
  vtkImageChangeInformation* m_UnitSpacingImageFilter; 
  vtkVolumeProperty* m_VolumeProperty;
  //vtkVolumeRayCastMapper* m_VtkVolumeMapper;
	vtkVolumeTextureMapper2D* m_VtkVolumeMapper;

};

} // namespace mitk

#endif /* MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED */
