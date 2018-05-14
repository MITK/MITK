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

#ifndef PeakImageMapper2D_H_HEADER_INCLUDED
#define PeakImageMapper2D_H_HEADER_INCLUDED

//MITK Rendering
#include <mitkCommon.h>
#include <mitkBaseRenderer.h>
#include <mitkVtkMapper.h>
#include <mitkPeakImage.h>
#include <vtkSmartPointer.h>

#define MITKPeakImageMapper2D_POLYDATAMAPPER vtkOpenGLPolyDataMapper

class vtkActor;
class mitkBaseRenderer;
class MITKPeakImageMapper2D_POLYDATAMAPPER;
class vtkCutter;
class vtkPlane;
class vtkPolyData;



namespace mitk {

struct IShaderRepository;

class PeakImageMapper2D : public VtkMapper
{

public:
    mitkClassMacro(PeakImageMapper2D, VtkMapper)
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
        vtkSmartPointer<vtkActor> m_PointActor;
        vtkSmartPointer<MITKPeakImageMapper2D_POLYDATAMAPPER> m_Mapper;

        itk::TimeStamp m_LastUpdateTime;
        LocalStorage();

        ~LocalStorage() override
        {
        }
    };

    /** \brief This member holds all three LocalStorages for the three 2D render windows. */
    mitk::LocalStorageHandler<LocalStorage> m_LocalStorageHandler;

protected:
    PeakImageMapper2D();
    ~PeakImageMapper2D() override;
    void GenerateDataForRenderer(mitk::BaseRenderer*) override;

private:
    vtkSmartPointer<vtkLookupTable> m_lut;
};


}//end namespace

#endif
