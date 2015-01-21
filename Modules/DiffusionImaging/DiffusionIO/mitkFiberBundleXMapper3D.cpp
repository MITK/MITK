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



#include "mitkFiberBundleXMapper3D.h"
#include <mitkProperties.h>

#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkDepthSortPolyData.h>
#include <vtkCamera.h>
#include <vtkTubeFilter.h>

mitk::FiberBundleXMapper3D::FiberBundleXMapper3D()
    : m_TubeRadius(0.0)
    , m_TubeSides(15)
    , m_LineWidth(1)
{
    m_lut = vtkLookupTable::New();
    m_lut->Build();
}


mitk::FiberBundleXMapper3D::~FiberBundleXMapper3D()
{

}


const mitk::FiberBundleX* mitk::FiberBundleXMapper3D::GetInput()
{
    return static_cast<const mitk::FiberBundleX * > ( GetDataNode()->GetData() );
}


/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::FiberBundleXMapper3D::InternalGenerateData(mitk::BaseRenderer *renderer)
{
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*> (GetDataNode()->GetData());
    if (fiberBundle == NULL)
        return;

    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    if (fiberPolyData == NULL)
        return;

    fiberPolyData->GetPointData()->AddArray(fiberBundle->GetFiberColors());
    float tmpopa;
    this->GetDataNode()->GetOpacity(tmpopa, NULL);
    FBXLocalStorage3D *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

    if (m_TubeRadius>0.0)
    {
        vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
        tubeFilter->SetInputData(fiberPolyData);
        tubeFilter->SetNumberOfSides(m_TubeSides);
        tubeFilter->SetRadius(m_TubeRadius);
        tubeFilter->Update();
        fiberPolyData = tubeFilter->GetOutput();
    }

    if (tmpopa<1)
    {
        vtkSmartPointer<vtkDepthSortPolyData> depthSort = vtkSmartPointer<vtkDepthSortPolyData>::New();
        depthSort->SetInputData( fiberPolyData );
        depthSort->SetCamera( renderer->GetVtkRenderer()->GetActiveCamera() );
        depthSort->SetDirectionToFrontToBack();
        depthSort->Update();
        localStorage->m_FiberMapper->SetInputConnection(depthSort->GetOutputPort());
    }
    else
    {
        localStorage->m_FiberMapper->SetInputData(fiberPolyData);
    }

    localStorage->m_FiberMapper->SelectColorArray("FIBER_COLORS");
    localStorage->m_FiberMapper->ScalarVisibilityOn();
    localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
    localStorage->m_FiberActor->SetMapper(localStorage->m_FiberMapper);
    localStorage->m_FiberMapper->SetLookupTable(m_lut);

    // set Opacity
    localStorage->m_FiberActor->GetProperty()->SetOpacity((double) tmpopa);
    localStorage->m_FiberActor->GetProperty()->SetLineWidth(m_LineWidth);

    localStorage->m_FiberAssembly->AddPart(localStorage->m_FiberActor);
    localStorage->m_LastUpdateTime.Modified();
}



void mitk::FiberBundleXMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
    bool visible = true;
    GetDataNode()->GetVisibility(visible, renderer, "visible");
    if ( !visible ) return;

    const DataNode* node = this->GetDataNode();
    FBXLocalStorage3D* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
    mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>(node->GetData());

    // did any rendering properties change?
    float tubeRadius = 0;
    node->GetFloatProperty("TubeRadius", tubeRadius);
    if (m_TubeRadius!=tubeRadius)
    {
        m_TubeRadius = tubeRadius;
        fiberBundle->RequestUpdate3D();
    }

    int tubeSides = 0;
    node->GetIntProperty("TubeSides", tubeSides);
    if (m_TubeSides!=tubeSides)
    {
        m_TubeSides = tubeSides;
        fiberBundle->RequestUpdate3D();
    }

    int lineWidth = 0;
    node->GetIntProperty("LineWidth", lineWidth);
    if (m_LineWidth!=lineWidth)
    {
        m_LineWidth = lineWidth;
        fiberBundle->RequestUpdate3D();
    }

    if (localStorage->m_LastUpdateTime>=fiberBundle->GetUpdateTime3D())
        return;

    // Calculate time step of the input data for the specified renderer (integer value)
    // this method is implemented in mitkMapper
    this->CalculateTimeStep( renderer );
    this->InternalGenerateData(renderer);
}


void mitk::FiberBundleXMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
    Superclass::SetDefaultProperties(node, renderer, overwrite);
    node->AddProperty( "LineWidth", mitk::IntProperty::New( true ), renderer, overwrite );
    node->AddProperty( "opacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
    node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
    node->AddProperty( "pickable", mitk::BoolProperty::New( true ), renderer, overwrite);

    node->AddProperty( "TubeRadius",mitk::FloatProperty::New( 0.0 ), renderer, overwrite);
    node->AddProperty( "TubeSides",mitk::IntProperty::New( 15 ), renderer, overwrite);
}

vtkProp* mitk::FiberBundleXMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    return m_LocalStorageHandler.GetLocalStorage(renderer)->m_FiberAssembly;
}

void mitk::FiberBundleXMapper3D::UpdateVtkObjects()
{

}

void mitk::FiberBundleXMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{

}

mitk::FiberBundleXMapper3D::FBXLocalStorage3D::FBXLocalStorage3D()
{
    m_FiberActor = vtkSmartPointer<vtkActor>::New();
    m_FiberMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_FiberAssembly = vtkSmartPointer<vtkPropAssembly>::New();
}

