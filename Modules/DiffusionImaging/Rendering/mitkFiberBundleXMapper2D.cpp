/*
 *  mitkFiberBundleMapper2D.cpp
 *  mitk-all
 *
 *  Created by HAL9000 on 1/17/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mitkFiberBundleXMapper2D.h"
#include <mitkBaseRenderer.h>


#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
//#include <vtkPropAssembly.h>

//#include <vtkPainterPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCamera.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>

//#include <mitkGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <mitkSliceNavigationController.h>

#include <mitkShaderRepository.h>
#include <mitkShaderProperty.h>
#include <mitkStandardFileLocations.h>

#include <QCoreApplication>
#include <QFile>

mitk::FiberBundleXMapper2D::FiberBundleXMapper2D()
{
    m_lut = vtkLookupTable::New();
    m_lut->Build();

}

mitk::FiberBundleXMapper2D::~FiberBundleXMapper2D()
{
}


mitk::FiberBundleX* mitk::FiberBundleXMapper2D::GetInput()
{
    return dynamic_cast< mitk::FiberBundleX * > ( GetData() );
}



void mitk::FiberBundleXMapper2D::Update(mitk::BaseRenderer * renderer)
{

    if ( !this->IsVisible( renderer ) )
    {
        return;
    }


    // Calculate time step of the input data for the specified renderer (integer value)
    // this method is implemented in mitkMapper
    this->CalculateTimeStep( renderer );

    //check if updates occured in the node or on the display
    FBXLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
    const DataNode *node = this->GetDataNode();
    if ( (localStorage->m_LastUpdateTime < node->GetMTime())
         || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
         || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
    {
        //    MITK_INFO << "UPDATE NEEDED FOR _ " << renderer->GetName();
        this->GenerateDataForRenderer( renderer );
    }

    if ((localStorage->m_LastUpdateTime < renderer->GetDisplayGeometry()->GetMTime()) ) //was the display geometry modified? e.g. zooming, panning)
    {

        this->UpdateShaderParameter(renderer);

    }

}

void mitk::FiberBundleXMapper2D::UpdateShaderParameter(mitk::BaseRenderer * renderer)
{
    FBXLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

    //get information about current position of views
    mitk::SliceNavigationController::Pointer sliceContr = renderer->GetSliceNavigationController();
    mitk::PlaneGeometry::ConstPointer planeGeo = sliceContr->GetCurrentPlaneGeometry();

    //generate according cutting planes based on the view position
    float sliceN[3], planeOrigin[3];


    // since shader uses camera coordinates, transform origin and normal from worldcoordinates to cameracoordinates


    planeOrigin[0] = (float) planeGeo->GetOrigin()[0];
    planeOrigin[1] = (float) planeGeo->GetOrigin()[1];
    planeOrigin[2] = (float) planeGeo->GetOrigin()[2];

    sliceN[0] = planeGeo->GetNormal()[0];
    sliceN[1] = planeGeo->GetNormal()[1];
    sliceN[2] = planeGeo->GetNormal()[2];


    float tmp1 = planeOrigin[0] * sliceN[0];
    float tmp2 = planeOrigin[1] * sliceN[1];
    float tmp3 = planeOrigin[2] * sliceN[2];
    float d1 = tmp1 + tmp2 + tmp3; //attention, correct normalvector


    float plane1[4];
    plane1[0] = sliceN[0];
    plane1[1] = sliceN[1];
    plane1[2] = sliceN[2];
    plane1[3] = d1;

    float thickness = 2.0;
    if(!this->GetDataNode()->GetPropertyValue("Fiber2DSliceThickness",thickness))
        MITK_INFO << "FIBER2D SLICE THICKNESS PROPERTY ERROR";


    bool fiberfading = false;
    if(!this->GetDataNode()->GetPropertyValue("Fiber2DfadeEFX",fiberfading))
        MITK_INFO << "FIBER2D SLICE FADE EFX PROPERTY ERROR";


    int fiberfading_i = 1;
    if (!fiberfading)
        fiberfading_i = 0;

    // set Opacity
    float fiberOpacity;
    this->GetDataNode()->GetOpacity(fiberOpacity, NULL);

    localStorage->m_PointActor->GetProperty()->AddShaderVariable("slicingPlane",4, plane1);
    localStorage->m_PointActor->GetProperty()->AddShaderVariable("fiberThickness",1, &thickness);
    localStorage->m_PointActor->GetProperty()->AddShaderVariable("fiberFadingON",1,  &fiberfading_i);
    localStorage->m_PointActor->GetProperty()->AddShaderVariable("fiberOpacity", 1, &fiberOpacity);


}

// ALL RAW DATA FOR VISUALIZATION IS GENERATED HERE.
// vtkActors and Mappers are feeded here
void mitk::FiberBundleXMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{

    //the handler of local storage gets feeded in this method with requested data for related renderwindow
    FBXLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

    //this procedure is depricated,
    //not needed after initializaton anymore
    mitk::DataNode* node = this->GetDataNode();
    if ( node == NULL )
    {
        MITK_INFO << "check DATANODE: ....[Fail] ";
        return;
    }
    ///////////////////////////////////

    ///THIS GET INPUT
    mitk::FiberBundleX* fbx = this->GetInput();

    localStorage->m_PointMapper->ScalarVisibilityOn();
    localStorage->m_PointMapper->SetScalarModeToUsePointFieldData();
    localStorage->m_PointMapper->SetLookupTable(m_lut);  //apply the properties after the slice was set
    localStorage->m_PointActor->GetProperty()->SetOpacity(0.999);

    // set color
    if (fbx->GetCurrentColorCoding() != NULL){
//        localStorage->m_PointMapper->SelectColorArray("");
        localStorage->m_PointMapper->SelectColorArray(fbx->GetCurrentColorCoding());
        MITK_INFO << "MapperFBX 2D: " << fbx->GetCurrentColorCoding();

        if(fbx->GetCurrentColorCoding() == fbx->COLORCODING_CUSTOM){
            float temprgb[3];
            this->GetDataNode()->GetColor( temprgb, NULL );
            double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
            localStorage->m_PointActor->GetProperty()->SetColor(trgb);
        }
    }



    localStorage->m_PointMapper->SetInput(fbx->GetFiberPolyData());
    localStorage->m_PointActor->SetMapper(localStorage->m_PointMapper);
    localStorage->m_PointActor->GetProperty()->ShadingOn();

    // Applying shading properties
    {
        mitk::ShaderRepository::GetGlobalShaderRepository()->ApplyProperties(this->GetDataNode(),localStorage->m_PointActor,renderer, localStorage->m_LastUpdateTime);
    }



    this->UpdateShaderParameter(renderer);


    // We have been modified => save this for next Update()
    localStorage->m_LastUpdateTime.Modified();


}


vtkProp* mitk::FiberBundleXMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{

    //MITK_INFO << "FiberBundleMapper2D GetVtkProp(renderer)";
    this->Update(renderer);
    return m_LSH.GetLocalStorage(renderer)->m_PointActor;

}


void mitk::FiberBundleXMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{    //add shader to datano


    //####### load shader from file #########
    QString applicationDir = QCoreApplication::applicationDirPath();

    if (applicationDir.endsWith("bin"))
        applicationDir.append("/");
    else if (applicationDir.endsWith("MacOS"))
    {
        //on osx, check if path for installer or MITK development is needed
        applicationDir.append("/");
        QFile f( applicationDir+"FiberTrackingLUTBaryCoords.bin" );
        if( !f.exists() ) // if file does not exist, then look in MITK development build directory
            applicationDir.append("../../../");
    }else
        applicationDir.append("\\..\\");

    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
    mitk::ShaderRepository::Pointer shaderRepository = mitk::ShaderRepository::GetGlobalShaderRepository();
    shaderRepository->LoadShader(mitk::StandardFileLocations::GetInstance()->FindFile("mitkShaderFiberClipping.xml"));



    //####################################################################
    node->SetProperty("shader",mitk::ShaderProperty::New("mitkShaderFiberClipping"));
    mitk::ShaderRepository::GetGlobalShaderRepository()->AddDefaultProperties(node,renderer,overwrite);


    //add other parameters to propertylist
    node->AddProperty( "Fiber2DSliceThickness", mitk::FloatProperty::New(2.0f), renderer, overwrite );
    node->AddProperty( "Fiber2DfadeEFX", mitk::BoolProperty::New(true), renderer, overwrite );


    Superclass::SetDefaultProperties(node, renderer, overwrite);
}





// following methods are essential, they actually call the GetVtkProp() method
// which returns the desired actors
void mitk::FiberBundleXMapper2D::MitkRenderOverlay(BaseRenderer* renderer)
{
    //   MITK_INFO << "FiberBundleMapper2D MitkRenderOVerlay(renderer)";
    if ( this->IsVisible(renderer)==false )
        return;

    if ( this->GetVtkProp(renderer)->GetVisibility() )
    {
        this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
    }
}

void mitk::FiberBundleXMapper2D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
    //  MITK_INFO << "FiberBundleMapper2D MitkRenderOpaqueGeometry(renderer)";
    if ( this->IsVisible( renderer )==false )
        return;

    if ( this->GetVtkProp(renderer)->GetVisibility() )
        this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
}
void mitk::FiberBundleXMapper2D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
    //  MITK_INFO << "FiberBundleMapper2D MitkRenderTranslucentGeometry(renderer)";
    if ( this->IsVisible(renderer)==false )
        return;

    //TODO is it possible to have a visible BaseRenderer AND an invisible VtkRenderer???
    if ( this->GetVtkProp(renderer)->GetVisibility() )
        this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());

}
void mitk::FiberBundleXMapper2D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
    //  MITK_INFO << "FiberBundleMapper2D MitkRenderVolumentricGeometry(renderer)";
    if(IsVisible(renderer)==false)
        return;

    //TODO is it possible to have a visible BaseRenderer AND an invisible VtkRenderer???
    if ( GetVtkProp(renderer)->GetVisibility() )
        this->GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());

}

mitk::FiberBundleXMapper2D::FBXLocalStorage::FBXLocalStorage()
{
    m_PointActor = vtkSmartPointer<vtkActor>::New();
    m_PointMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

}
