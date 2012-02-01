/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
 Version:   $Revision: 17179 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/



#include "mitkFiberBundleXMapper3D.h"
#include <mitkProperties.h>
//#include <mitkFiberBundleInteractor.h>
//#include <mitkGlobalInteraction.h>

#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>

//not essential for mapper
#include <QTime>

mitk::FiberBundleXMapper3D::FiberBundleXMapper3D()
{
    m_lut = vtkLookupTable::New();
    m_lut->Build();
}


mitk::FiberBundleXMapper3D::~FiberBundleXMapper3D()
{

}


const mitk::FiberBundleX* mitk::FiberBundleXMapper3D::GetInput()
{
    MITK_INFO << "FiberBundleXxXXMapper3D() GetInput()";
    return static_cast<const mitk::FiberBundleX * > ( GetData() );
}


/*
 This method is called once the mapper gets new input,
 for UI rotation or changes in colorcoding this method is NOT called
 */
void mitk::FiberBundleXMapper3D::GenerateData(mitk::BaseRenderer *renderer)
{

    //MITK_INFO << "GENERATE DATA FOR FBX :)";
    //=====timer measurement====
    QTime myTimer;
    myTimer.start();
    //==========================


    mitk::FiberBundleX* FBX = dynamic_cast<mitk::FiberBundleX*> (this->GetData());
    if (FBX == NULL)
        return;

    vtkSmartPointer<vtkPolyData> FiberData = FBX->GetFiberPolyData();
    if (FiberData == NULL)
        return;


    FBXLocalStorage3D *localStorage = m_LSH.GetLocalStorage(renderer);
    localStorage->m_FiberMapper->SetInput(FiberData);

    if ( FiberData->GetPointData()->GetNumberOfArrays() > 0 )
        localStorage->m_FiberMapper->SelectColorArray( FBX->GetCurrentColorCoding() );

    localStorage->m_FiberMapper->ScalarVisibilityOn();
    localStorage->m_FiberMapper->SetScalarModeToUsePointFieldData();
    localStorage->m_FiberActor->SetMapper(localStorage->m_FiberMapper);
//    localStorage->m_FiberActor->GetProperty()->SetOpacity(0.999);
    localStorage->m_FiberMapper->SetLookupTable(m_lut);


    // set Opacity
    float tmpopa;
    this->GetDataNode()->GetOpacity(tmpopa, NULL);
    localStorage->m_FiberActor->GetProperty()->SetOpacity((double) tmpopa);

    // set color
    if (FBX->GetCurrentColorCoding() != NULL){
//        localStorage->m_FiberMapper->SelectColorArray("");
        localStorage->m_FiberMapper->SelectColorArray(FBX->GetCurrentColorCoding());
        MITK_DEBUG << "MapperFBX: " << FBX->GetCurrentColorCoding();

        if(FBX->GetCurrentColorCoding() == FBX->COLORCODING_CUSTOM) {
            float temprgb[3];
            this->GetDataNode()->GetColor( temprgb, NULL );
            double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
            localStorage->m_FiberActor->GetProperty()->SetColor(trgb);
        }
    }


    localStorage->m_FiberAssembly->AddPart(localStorage->m_FiberActor);
    localStorage->m_LastUpdateTime.Modified();
    //since this method is called after generating all necessary data for fiber visualization, all modifications are represented so far.

    //====timer measurement========
    MITK_INFO << "Execution Time GenerateData() (nmiliseconds): " << myTimer.elapsed();
    //=============================

}



void mitk::FiberBundleXMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
    if ( !this->IsVisible( renderer ) )
        return;

    // Calculate time step of the input data for the specified renderer (integer value)
    // this method is implemented in mitkMapper
    this->CalculateTimeStep( renderer );

    //check if updates occured in the node or on the display
    FBXLocalStorage3D *localStorage = m_LSH.GetLocalStorage(renderer);
    const DataNode *node = this->GetDataNode();
    if ( (localStorage->m_LastUpdateTime < node->GetMTime())
         || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
         || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
    {
        MITK_INFO << "UPDATE NEEDED FOR _ " << renderer->GetName();
        this->GenerateData(renderer);
    }

}


void mitk::FiberBundleXMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{

    //  MITK_INFO << "FiberBundleXxXXMapper3D()SetDefaultProperties";


    //MITK_INFO << "FiberBundleMapperX3D SetDefault Properties(...)";
    //   node->AddProperty( "DisplayChannel", mitk::IntProperty::New( true ), renderer, overwrite );
    node->AddProperty( "LineWidth", mitk::IntProperty::New( true ), renderer, overwrite );
    node->AddProperty( "opacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
    //  node->AddProperty( "VertexOpacity_1", mitk::BoolProperty::New( false ), renderer, overwrite);
    //  node->AddProperty( "Set_FA_VertexAlpha", mitk::BoolProperty::New( false ), renderer, overwrite);
    //  node->AddProperty( "pointSize", mitk::FloatProperty::New(0.5), renderer, overwrite);
    //  node->AddProperty( "setShading", mitk::IntProperty::New(1), renderer, overwrite);
    //  node->AddProperty( "Xmove", mitk::IntProperty::New( 0 ), renderer, overwrite);
    //  node->AddProperty( "Ymove", mitk::IntProperty::New( 0 ), renderer, overwrite);
    //  node->AddProperty( "Zmove", mitk::IntProperty::New( 0 ), renderer, overwrite);
    //  node->AddProperty( "RepPoints", mitk::BoolProperty::New( false ), renderer, overwrite);
    //  node->AddProperty( "TubeSides", mitk::IntProperty::New( 8 ), renderer, overwrite);
    //  node->AddProperty( "TubeRadius", mitk::FloatProperty::New( 0.15 ), renderer, overwrite);
    //  node->AddProperty( "TubeOpacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);

    node->AddProperty( "pickable", mitk::BoolProperty::New( true ), renderer, overwrite);

    Superclass::SetDefaultProperties(node, renderer, overwrite);



}

vtkProp* mitk::FiberBundleXMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    //MITK_INFO << "FiberBundleXxXXMapper3D()GetVTKProp";
    //this->GenerateData();
    return m_LSH.GetLocalStorage(renderer)->m_FiberAssembly;

}

void mitk::FiberBundleXMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{

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

