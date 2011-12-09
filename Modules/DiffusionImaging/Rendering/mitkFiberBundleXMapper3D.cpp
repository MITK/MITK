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
#include <vtkLookupTable.h>

//not essential for mapper
#include <QTime>

mitk::FiberBundleXMapper3D::FiberBundleXMapper3D()
    :m_FiberMapperGLSP(vtkSmartPointer<vtkOpenGLPolyDataMapper>::New()),
      m_FiberMapperGLWP(vtkOpenGLPolyDataMapper::New()),
      m_FiberActorSP(vtkSmartPointer<vtkOpenGLActor>::New()),
      m_FiberActorWP(vtkOpenGLActor::New()),
      m_FiberAssembly(vtkPropAssembly::New())
{

}


mitk::FiberBundleXMapper3D::~FiberBundleXMapper3D()
{
    m_FiberAssembly->Delete();
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
void mitk::FiberBundleXMapper3D::GenerateData()
{
    //MITK_INFO << "GENERATE DATA FOR FBX :)";
    //=====timer measurement====
    QTime myTimer;
    myTimer.start();
    //==========================

    //  mitk::FiberBundleX::Pointer FBX = dynamic_cast< mitk::FiberBundleX* > (this->GetData());

    mitk::FiberBundleX* FBX = dynamic_cast<mitk::FiberBundleX*> (this->GetData());
    if (FBX == NULL) {
        return;
    }
    //todo smartpointer
    vtkPolyData* FiberData = FBX->GetFiberPolyData();

    if (FiberData == NULL) {
        return;
    }


    m_FiberMapperGLSP->SetInput(FiberData);
    //  m_FiberMapperGLWP->SetInput(FiberData);


    if ( FiberData->GetPointData()->GetNumberOfArrays() > 0 )
    {

        m_FiberMapperGLSP->SelectColorArray( FBX->GetCurrentColorCoding() );
    }
    
    m_FiberMapperGLSP->ScalarVisibilityOn();
    //    m_FiberMapperGLWP->ScalarVisibilityOn();
    m_FiberMapperGLSP->SetScalarModeToUsePointFieldData();
    //    m_FiberMapperGLWP->SetScalarModeToUsePointFieldData();



    m_FiberActorSP->SetMapper(m_FiberMapperGLSP);
    //  m_FiberActorWP->SetMapper(m_FiberMapperGLWP);

    m_FiberActorSP->GetProperty()->SetOpacity(1.0);
    //  m_FiberActorWP->GetProperty()->SetOpacity(1.0);

    if (FBX->GetCurrentColorCoding() != NULL){
        m_FiberMapperGLSP->SelectColorArray(FBX->GetCurrentColorCoding());
        MITK_INFO << "MapperFBX: " << FBX->GetCurrentColorCoding();
    }
    m_FiberAssembly->AddPart(m_FiberActorSP);

    //since this method is called after generating all necessary data for fiber visualization, all modifications are represented so far.
    FBX->setFBXModificationDone();
    //====timer measurement========
    MITK_INFO << "Execution Time GenerateData() (nmiliseconds): " << myTimer.elapsed();
    //=============================

}



void mitk::FiberBundleXMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
    if ( !this->IsVisible( renderer ) )
    {
        return;
    }

    //MITK_INFO << "FiberBundleXxXXMapper3D()DataForRenderer";
    //ToDo do update checks
    mitk::FiberBundleX* FBX = dynamic_cast<mitk::FiberBundleX*> (this->GetData());
    if(FBX->isFiberBundleXModified())
        this->GenerateData();
}


void mitk::FiberBundleXMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{

    //  MITK_INFO << "FiberBundleXxXXMapper3D()SetDefaultProperties";


    //MITK_INFO << "FiberBundleMapperX3D SetDefault Properties(...)";
    //   node->AddProperty( "DisplayChannel", mitk::IntProperty::New( true ), renderer, overwrite );
    //  node->AddProperty( "LineWidth", mitk::IntProperty::New( true ), renderer, overwrite );
    //  node->AddProperty( "ColorCoding", mitk::IntProperty::New( 0 ), renderer, overwrite);
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
    return m_FiberAssembly;

}

void mitk::FiberBundleXMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
    //  MITK_INFO << "FiberBundleXXXXMapper3D ApplyProperties(renderer)";
}

void mitk::FiberBundleXMapper3D::UpdateVtkObjects()
{
    //  MITK_INFO << "FiberBundleXxxXMapper3D UpdateVtkObjects()";


}

void mitk::FiberBundleXMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{



}



