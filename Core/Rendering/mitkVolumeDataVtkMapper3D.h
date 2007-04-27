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
class vtkFixedPointVolumeRayCastMapper;
class vtkVolumeTextureMapper2D;
class vtkVolumeMapper;
class vtkVolume;
class vtkObject;
class vtkImageShiftScale;
class vtkImageChangeInformation;
class vtkLODProp3D;
class vtkImageResample;

namespace mitk {


//##Documentation
//## @brief Vtk-based mapper for VolumeData
//##
//## @ingroup Mapper
class VolumeDataVtkMapper3D : public BaseVtkMapper3D
{
public:
  mitkClassMacro(VolumeDataVtkMapper3D, BaseVtkMapper3D);

  itkNewMacro(Self);

  virtual const mitk::Image* GetInput();

  virtual void ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer);
  static void AbortCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);
  static void EndCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);
  static void StartCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);

protected:


  VolumeDataVtkMapper3D();


  virtual ~VolumeDataVtkMapper3D();

  virtual void GenerateData(mitk::BaseRenderer* renderer);

  vtkActor* m_Actor;
  vtkImageShiftScale* m_ImageCast;
  vtkImageChangeInformation* m_UnitSpacingImageFilter; 
  vtkVolumeProperty* m_VolumeProperty;
  vtkVolumeProperty* m_VolumeProperty2;
  vtkVolumeTextureMapper2D* m_T2DMapper;
  vtkVolumeTextureMapper2D* m_T2DMapperHi;
  vtkVolumeRayCastMapper* m_HiResMapper;
#if (VTK_MAJOR_VERSION >= 5)
  vtkFixedPointVolumeRayCastMapper* m_FPRCMapper;
  int m_FPRCID;
#endif
  vtkImageResample* m_Resampler; 
  vtkLODProp3D* m_VolumeLOD;

  int m_LowResID;
  int m_MedResID;
  int m_HiResID;
  bool m_Firstcall;

};

} // namespace mitk

#endif /* MITKVOLUMEDATAVTKMAPPER3D_H_HEADER_INCLUDED */
