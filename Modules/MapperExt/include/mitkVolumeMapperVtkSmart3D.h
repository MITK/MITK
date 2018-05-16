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

#ifndef MITKVTKSMARTVOLUMEMAPPER_H_HEADER_INCLUDED
#define MITKVTKSMARTVOLUMEMAPPER_H_HEADER_INCLUDED

// MITK
#include "MitkMapperExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkVtkMapper.h"

// VTK
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>
#include <vtkVersionMacros.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageData.h>
#include <vtkImageChangeInformation.h>

class vtkRenderingOpenGL2ObjectFactory;
class vtkRenderingVolumeOpenGL2ObjectFactory;

namespace mitk
{

  //##Documentation
  //## @brief Vtk-based mapper for VolumeData
  //##
  //## @ingroup Mapper
  class MITKMAPPEREXT_EXPORT VolumeMapperVtkSmart3D : public VtkMapper
  {
  public:
    mitkClassMacro(VolumeMapperVtkSmart3D, VtkMapper);

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    void ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer) override;
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

    vtkSmartPointer<vtkRenderingOpenGL2ObjectFactory> m_RenderingOpenGL2ObjectFactory;
    vtkSmartPointer<vtkRenderingVolumeOpenGL2ObjectFactory> m_RenderingVolumeOpenGL2ObjectFactory;
    
    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);
    void UpdateRenderMode(mitk::BaseRenderer *renderer);
  };

} // namespace mitk

#endif /* MITKVTKSMARTVOLUMEMAPPER_H_HEADER_INCLUDED */
