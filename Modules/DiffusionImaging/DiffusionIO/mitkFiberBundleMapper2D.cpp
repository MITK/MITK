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

#include "mitkFiberBundleMapper2D.h"
#include <mitkBaseRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCamera.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTubeFilter.h>
#include <mitkPlaneGeometry.h>
#include <mitkSliceNavigationController.h>
#include <mitkIShaderRepository.h>
#include <mitkShaderProperty.h>
#include <mitkCoreServices.h>

mitk::FiberBundleMapper2D::FiberBundleMapper2D()
    : m_LineWidth(1)
{
    m_lut = vtkLookupTable::New();
    m_lut->Build();

}

mitk::FiberBundleMapper2D::~FiberBundleMapper2D()
{
}


mitk::FiberBundle* mitk::FiberBundleMapper2D::GetInput()
{
    return dynamic_cast< mitk::FiberBundle * > ( GetDataNode()->GetData() );
}



void mitk::FiberBundleMapper2D::Update(mitk::BaseRenderer * renderer)
{
        bool visible = true;
        GetDataNode()->GetVisibility(visible, renderer, "visible");
        if ( !visible )
            return;

        // Calculate time step of the input data for the specified renderer (integer value)
        // this method is implemented in mitkMapper
        this->CalculateTimeStep( renderer );

        //check if updates occured in the node or on the display
        FBXLocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

        //set renderer independent shader properties
        const DataNode::Pointer node = this->GetDataNode();
        float thickness = 2.0;
        if(!this->GetDataNode()->GetPropertyValue("Fiber2DSliceThickness",thickness))
            MITK_INFO << "FIBER2D SLICE THICKNESS PROPERTY ERROR";

        bool fiberfading = false;
        if(!this->GetDataNode()->GetPropertyValue("Fiber2DfadeEFX",fiberfading))
            MITK_INFO << "FIBER2D SLICE FADE EFX PROPERTY ERROR";

        float fiberOpacity;
        this->GetDataNode()->GetOpacity(fiberOpacity, NULL);
        node->SetFloatProperty("shader.mitkShaderFiberClipping.fiberThickness",thickness);
        node->SetIntProperty("shader.mitkShaderFiberClipping.fiberFadingON",fiberfading);
        node->SetFloatProperty("shader.mitkShaderFiberClipping.fiberOpacity",fiberOpacity);

        mitk::FiberBundle* fiberBundle = this->GetInput();
        if (fiberBundle==NULL)
            return;

        int lineWidth = 0;
        node->GetIntProperty("LineWidth", lineWidth);
        if (m_LineWidth!=lineWidth)
        {
            m_LineWidth = lineWidth;
            fiberBundle->RequestUpdate2D();
        }

        if ( localStorage->m_LastUpdateTime<renderer->GetCurrentWorldPlaneGeometryUpdateTime() || localStorage->m_LastUpdateTime<fiberBundle->GetUpdateTime2D() )
        {
            this->UpdateShaderParameter(renderer);
            this->GenerateDataForRenderer( renderer );
        }
}

void mitk::FiberBundleMapper2D::UpdateShaderParameter(mitk::BaseRenderer * renderer)
{
    //get information about current position of views
    mitk::SliceNavigationController::Pointer sliceContr = renderer->GetSliceNavigationController();
    mitk::PlaneGeometry::ConstPointer planeGeo = sliceContr->GetCurrentPlaneGeometry();

    //generate according cutting planes based on the view position
    float planeNormal[3];
    planeNormal[0] = planeGeo->GetNormal()[0];
    planeNormal[1] = planeGeo->GetNormal()[1];
    planeNormal[2] = planeGeo->GetNormal()[2];

    float tmp1 = planeGeo->GetOrigin()[0] * planeNormal[0];
    float tmp2 = planeGeo->GetOrigin()[1] * planeNormal[1];
    float tmp3 = planeGeo->GetOrigin()[2] * planeNormal[2];
    float thickness = tmp1 + tmp2 + tmp3; //attention, correct normalvector

    DataNode::Pointer node = this->GetDataNode();
    node->SetFloatProperty("shader.mitkShaderFiberClipping.slicingPlane.w",thickness,renderer);
    node->SetFloatProperty("shader.mitkShaderFiberClipping.slicingPlane.x",planeNormal[0],renderer);
    node->SetFloatProperty("shader.mitkShaderFiberClipping.slicingPlane.y",planeNormal[1],renderer);
    node->SetFloatProperty("shader.mitkShaderFiberClipping.slicingPlane.z",planeNormal[2],renderer);
}

// vtkActors and Mappers are feeded here
void mitk::FiberBundleMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    mitk::FiberBundle* fiberBundle = this->GetInput();

    //the handler of local storage gets feeded in this method with requested data for related renderwindow
    FBXLocalStorage *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

    mitk::DataNode* node = this->GetDataNode();
    if ( node == NULL )
        return;

    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    if (fiberPolyData == NULL)
        return;

    fiberPolyData->GetPointData()->AddArray(fiberBundle->GetFiberColors());
    localStorage->m_FiberMapper->ScalarVisibilityOn();
    localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
    localStorage->m_FiberMapper->SetLookupTable(m_lut);  //apply the properties after the slice was set
    localStorage->m_PointActor->GetProperty()->SetOpacity(0.999);
    localStorage->m_FiberMapper->SelectColorArray("FIBER_COLORS");

    localStorage->m_FiberMapper->SetInputData(fiberPolyData);
    localStorage->m_PointActor->SetMapper(localStorage->m_FiberMapper);
    localStorage->m_PointActor->GetProperty()->ShadingOn();
    localStorage->m_PointActor->GetProperty()->SetLineWidth(m_LineWidth);

    // Applying shading properties
    this->ApplyShaderProperties(renderer);

    // We have been modified => save this for next Update()
    localStorage->m_LastUpdateTime.Modified();
}


vtkProp* mitk::FiberBundleMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    this->Update(renderer);
    return m_LocalStorageHandler.GetLocalStorage(renderer)->m_PointActor;
}


void mitk::FiberBundleMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
    Superclass::SetDefaultProperties(node, renderer, overwrite);
    node->SetProperty("shader",mitk::ShaderProperty::New("mitkShaderFiberClipping"));

    // Shaders
    IShaderRepository* shaderRepo = CoreServices::GetShaderRepository();
    if (shaderRepo)
    {
        shaderRepo->AddDefaultProperties(node, renderer, overwrite);
    }

    //add other parameters to propertylist
    node->AddProperty( "Fiber2DSliceThickness", mitk::FloatProperty::New(1.0f), renderer, overwrite );
    node->AddProperty( "Fiber2DfadeEFX", mitk::BoolProperty::New(true), renderer, overwrite );
    node->AddProperty( "color", mitk::ColorProperty::New(1.0,1.0,1.0), renderer, overwrite);
}


mitk::FiberBundleMapper2D::FBXLocalStorage::FBXLocalStorage()
{
    m_PointActor = vtkSmartPointer<vtkActor>::New();
    m_FiberMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
}
