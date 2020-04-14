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
#include <vtkImageExtractComponents.h>

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

    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

    virtual void ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer) override;
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    void setClippingPlanes(vtkPlanes* planes);

  protected:
    VolumeMapperVtkSmart3D();
    virtual ~VolumeMapperVtkSmart3D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    void createMapper(vtkImageData*);
    void createVolume();
    void createVolumeProperty();
    vtkImageData* GetInputImage();
    void initialize();

    vtkSmartPointer<vtkVolume> m_Volume;
    vtkSmartPointer<vtkSmartVolumeMapper> m_SmartVolumeMapper;
    vtkSmartPointer<vtkVolumeProperty> m_VolumeProperty;
    vtkSmartPointer<vtkImageExtractComponents> m_VectorComponentExtractor;

    int m_LastTimeStep;
    int m_LastComponent;

    void UpdateTransferFunctions(mitk::BaseRenderer *renderer);
    void UpdateRenderMode(mitk::BaseRenderer *renderer);

    vtkSmartPointer<vtkPiecewiseFunction> m_BinaryOpacityTransferFunction;
    vtkSmartPointer<vtkPiecewiseFunction> m_BinaryGradientTransferFunction;
  };

} // namespace mitk

#endif /* MITKVTKSMARTVOLUMEMAPPER_H_HEADER_INCLUDED */