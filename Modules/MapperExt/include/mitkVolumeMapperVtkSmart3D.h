/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

    mitkNewMacro1Param(Self, mitk::DataNode::Pointer);

    itkCloneMacro(Self);

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    void ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer) override;
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

  protected:
    VolumeMapperVtkSmart3D(mitk::DataNode::Pointer datanode);
    ~VolumeMapperVtkSmart3D() override;

    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void createMapper(vtkImageData*);
    void createVolume();
    void createVolumeProperty();
    vtkImageData* GetInputImage();

    std::vector<vtkSmartPointer<vtkVolume>> m_Volume;
    std::vector<vtkSmartPointer<vtkImageChangeInformation>> m_ImageChangeInformation;
    std::vector<vtkSmartPointer<vtkSmartVolumeMapper>> m_SmartVolumeMapper;
    std::vector<vtkSmartPointer<vtkVolumeProperty>> m_VolumeProperty;

    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);
    void UpdateRenderMode(mitk::BaseRenderer *renderer);
  };

} // namespace mitk

#endif /* MITKVTKSMARTVOLUMEMAPPER_H_HEADER_INCLUDED */
