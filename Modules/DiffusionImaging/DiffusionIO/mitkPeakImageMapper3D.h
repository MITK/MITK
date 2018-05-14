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

#ifndef PeakImageMapper3D_H_HEADER_INCLUDED
#define PeakImageMapper3D_H_HEADER_INCLUDED

//MITK Rendering
#include <mitkCommon.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <mitkPeakImage.h>
#include <vtkSmartPointer.h>

#define MITKPeakImageMapper3D_POLYDATAMAPPER vtkOpenGLPolyDataMapper

class vtkActor;
class mitkBaseRenderer;
class MITKPeakImageMapper3D_POLYDATAMAPPER;
class vtkCutter;
class vtkPlane;
class vtkPolyData;



namespace mitk {

struct IShaderRepository;

class PeakImageMapper3D : public VtkMapper
{

public:
    mitkClassMacro(PeakImageMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitk::PeakImage* GetInput();
    void Update(mitk::BaseRenderer * renderer) override;
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );
    void UpdateVtkTransform(mitk::BaseRenderer *renderer) override;
    vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

    class  LocalStorage : public mitk::Mapper::BaseLocalStorage
    {
    public:
        vtkSmartPointer<vtkActor> m_Actor;
        vtkSmartPointer<MITKPeakImageMapper3D_POLYDATAMAPPER> m_Mapper;
        vtkSmartPointer<vtkPropAssembly> m_Assembly;

        itk::TimeStamp m_LastUpdateTime;
        LocalStorage();

        ~LocalStorage() override
        {
        }
    };

    mitk::LocalStorageHandler<LocalStorage> m_LocalStorageHandler;

protected:
    PeakImageMapper3D();
    ~PeakImageMapper3D() override;
    void GenerateDataForRenderer(mitk::BaseRenderer*) override;

private:
    vtkSmartPointer<vtkLookupTable> m_lut;
};


}//end namespace

#endif
