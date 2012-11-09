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


#include "mitkVtkMapper3D.h"
#include "mitkDataNode.h"
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

VtkMapper3D::VtkMapper3D()
{
}


VtkMapper3D::~VtkMapper3D()
{
}

void VtkMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *mapper)
{
  if(mapper)
    mapper->SetImmediateModeRendering(mitk::VtkPropRenderer::useImmediateModeRendering());
}


void VtkMapper3D::UpdateVtkTransform(mitk::BaseRenderer *renderer)
{
  vtkLinearTransform * vtktransform = GetDataNode()->GetVtkTransform(this->GetTimestep());

  vtkProp3D *prop = dynamic_cast<vtkProp3D*>( GetVtkProp(renderer) );
  if(prop)
    prop->SetUserTransform(vtktransform);
}


void VtkMapper3D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible( renderer )==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

void VtkMapper3D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;

 /* if(dynamic_cast<vtkLODProp3D*>(m_Prop3D) != NULL)
  {
    if(  dynamic_cast<BoolProperty*>(GetDataNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())==NULL ||
         dynamic_cast<BoolProperty*>(GetDataNode()->
                                           GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false)
       return;
  }*/

  if ( this->GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());

}


void VtkMapper3D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
    return;

  /* if(dynamic_cast<vtkLODProp3D*>(m_Prop3D) != NULL)
  {
  if(  dynamic_cast<BoolProperty*>(GetDataNode()->
  GetProperty("volumerendering",renderer).GetPointer())==NULL ||
  dynamic_cast<BoolProperty*>(GetDataNode()->
  GetProperty("volumerendering",renderer).GetPointer())->GetValue() == false)
  return;
  }*/

  if ( GetVtkProp(renderer)->GetVisibility() )
    GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());

}


void VtkMapper3D::MitkRenderOverlay(BaseRenderer* renderer)
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


void VtkMapper3D::ApplyProperties(vtkActor* actor, BaseRenderer* renderer)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  this->GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  this->GetOpacity(rgba[3], renderer);

  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);

  // Add annotations to assembly, if any (camera (renderer) must be present)
  if ( renderer != NULL )
  {
    // Check whether one or more AnnotationProperty objects have been defined for
    // this node. Check both renderer specific and global property lists, since
    // properties in both should be considered.
    //const PropertyList::PropertyMap *rendererProperties = this->GetDataNode()->GetPropertyList( renderer )->GetMap();
    //const PropertyList::PropertyMap *globalProperties = this->GetDataNode()->GetPropertyList( NULL )->GetMap();

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
void VtkMapper3D::CheckForAnnotationProperty( mitk::BaseProperty *property, BaseRenderer *renderer )
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

    Geometry3D *geometry = m_DataNode->GetData()->GetGeometry();

    Point3D transformedPos;
    geometry->IndexToWorld( pos, transformedPos );

    labelFollower->SetPosition( transformedPos[0], transformedPos[1], transformedPos[2] );

    //labelFollower->SetUserTransform(
    //  m_DataNode->GetData()->GetGeometry()->GetVtkTransform() );

    m_LabelActorCollection->AddItem( labelFollower );
  }
}
*/

void VtkMapper3D::ReleaseGraphicsResources(vtkWindow * /*renWin*/)
{
/*
  if(m_Prop3D)
    m_Prop3D->ReleaseGraphicsResources(renWin);
*/
}


bool VtkMapper3D::HasVtkProp( const vtkProp *prop, BaseRenderer *renderer )
{
  vtkProp *myProp = this->GetVtkProp( renderer );

  // TODO: check if myProp is a vtkAssembly and if so, check if prop is contained in its leafs
  return ( prop == myProp );
}



} // namespace

