/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBaseVtkMapper3D.h"
#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkAnnotationProperty.h"
#include "mitkVtkPropRenderer.h"

#include <vtkProp3D.h>
#include <vtkLODProp3D.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>
#include <vtkPropAssembly.h>
#include <vtkFollower.h>
#include <vtkVectorText.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>


namespace mitk
{

BaseVtkMapper3D::BaseVtkMapper3D()
{
}


BaseVtkMapper3D::~BaseVtkMapper3D()
{
}

void BaseVtkMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *mapper)
{
  mapper->SetImmediateModeRendering(mitk::VtkPropRenderer::useImmediateModeRendering());
}


void BaseVtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  vtkLinearTransform * vtktransform = GetDataTreeNode()->GetVtkTransform(this->GetTimestep());
  
  vtkProp3D *prop = dynamic_cast<vtkProp3D*>( GetVtkProp(renderer) );
  if(prop)
    prop->SetUserTransform(vtktransform);
}


void BaseVtkMapper3D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible( renderer )==false ) 
    return;
  
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

void BaseVtkMapper3D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false ) 
    return;
  
 /* if(dynamic_cast<vtkLODProp3D*>(m_Prop3D) != NULL)
  {
    if(  dynamic_cast<BoolProperty*>(GetDataTreeNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())==NULL ||  
         dynamic_cast<BoolProperty*>(GetDataTreeNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false)    
       return;
  }*/
  
  if ( this->GetVtkProp(renderer)->GetVisibility() )
//BUG (#1551) changed VTK_MINOR_VERSION FROM 3 to 2 cause RenderTranslucentGeometry was changed in minor version 2
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
#else
    this->GetVtkProp(renderer)->RenderTranslucentGeometry(renderer->GetVtkRenderer());
#endif

}

//for VTK 5.2 support
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
void BaseVtkMapper3D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false) 
    return;

  /* if(dynamic_cast<vtkLODProp3D*>(m_Prop3D) != NULL)
  {
  if(  dynamic_cast<BoolProperty*>(GetDataTreeNode()->
  GetProperty("volumerendering",renderer).GetPointer())==NULL ||  
  dynamic_cast<BoolProperty*>(GetDataTreeNode()->
  GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false)    
  return;
  }*/

  if ( GetVtkProp(renderer)->GetVisibility() )
    GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());

}
#endif

void BaseVtkMapper3D::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false ) 
    return;
  
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }

  // Render annotations as overlay
/*
  m_LabelActorCollection->InitTraversal();
  vtkProp3D *labelActor;
  for ( m_LabelActorCollection->InitTraversal(); 
        (labelActor = m_LabelActorCollection->GetNextProp3D()); )
  {
    if ( labelActor->GetVisibility() )
      labelActor->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
  */
}


void BaseVtkMapper3D::ApplyProperties(vtkActor* actor, BaseRenderer* renderer)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  this->GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  this->GetOpacity(rgba[3], renderer);

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
#else
  actor->GetProperty()->SetColor(rgba);
  actor->GetProperty()->SetOpacity(rgba[3]);
#endif


  // Add annotations to assembly, if any (camera (renderer) must be present)
  if ( renderer != NULL )
  {
    // Check whether one or more AnnotationProperty objects have been defined for
    // this node. Check both renderer specific and global property lists, since
    // properties in both should be considered.
    const PropertyList::PropertyMap *rendererProperties = this->GetDataTreeNode()->GetPropertyList( renderer )->GetMap();
    const PropertyList::PropertyMap *globalProperties = this->GetDataTreeNode()->GetPropertyList( NULL )->GetMap();

    // Add clipping planes (if any)
/*
    m_LabelActorCollection->RemoveAllItems();

    PropertyList::PropertyMap::const_iterator it;
    for ( it = rendererProperties->begin(); it != rendererProperties->end(); ++it )
    {
      this->CheckForAnnotationProperty( (*it).second.first.GetPointer(), renderer );
    }

    for ( it = globalProperties->begin(); it != globalProperties->end(); ++it )
    {
      this->CheckForAnnotationProperty( (*it).second.first.GetPointer(), renderer );
    }
    */
  } 
}

/*
void BaseVtkMapper3D::CheckForAnnotationProperty( mitk::BaseProperty *property, BaseRenderer *renderer )
{
  AnnotationProperty *annotationProperty =
    dynamic_cast< AnnotationProperty * >( property );

  if ( annotationProperty != NULL )
  {
    vtkVectorText *labelText = vtkVectorText::New();
    vtkPolyDataMapper *labelMapper = vtkPolyDataMapper::New();
    labelMapper->SetInput( labelText->GetOutput() );

    vtkFollower *labelFollower = vtkFollower::New();
    labelFollower->SetMapper( labelMapper );
    labelFollower->SetCamera( renderer->GetVtkRenderer()->GetActiveCamera() );

    labelFollower->SetScale( 2.5, 2.5, 2.5 );
    labelFollower->GetProperty()->SetColor( 1.0, 0.2, 0.1 );

    labelText->SetText( annotationProperty->GetLabel() );

    const Point3D &pos = annotationProperty->GetPosition();

    Geometry3D *geometry = m_DataTreeNode->GetData()->GetGeometry();

    Point3D transformedPos;
    geometry->IndexToWorld( pos, transformedPos );

    labelFollower->SetPosition( transformedPos[0], transformedPos[1], transformedPos[2] );

    //labelFollower->SetUserTransform(
    //  m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform() );

    m_LabelActorCollection->AddItem( labelFollower );
  }
}
*/

void BaseVtkMapper3D::ReleaseGraphicsResources(vtkWindow *renWin)
{
/*
  if(m_Prop3D)
    m_Prop3D->ReleaseGraphicsResources(renWin);
*/
}


bool BaseVtkMapper3D::HasVtkProp( const vtkProp *prop, BaseRenderer *renderer )
{
  vtkProp *myProp = this->GetVtkProp( renderer );
  
  // TODO: check if myProp is a vtkAssembly and if so, check if prop is contained in its leafs
  return ( prop == myProp );
}



} // namespace

