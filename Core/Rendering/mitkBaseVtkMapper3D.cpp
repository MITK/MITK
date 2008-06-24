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

BaseVtkMapper3D::BaseVtkMapper3D() : m_Prop3D(NULL)
{
  //vtkMapper::GlobalImmediateModeRenderingOn();

  m_LabelActorCollection = vtkProp3DCollection::New();
}


BaseVtkMapper3D::~BaseVtkMapper3D()
{
  if(m_Prop3D)
    m_Prop3D->Delete();

  m_LabelActorCollection->Delete();
}


vtkProp *BaseVtkMapper3D::GetProp()
{
  return m_Prop3D;
}


vtkProp *BaseVtkMapper3D::GetLabelProp()
{
  //return m_LabelActor;
  return NULL;
}


void BaseVtkMapper3D::UpdateVtkTransform()
{
  vtkLinearTransform * vtktransform = GetDataTreeNode()->GetVtkTransform(this->GetTimestep());
  m_Prop3D->SetUserTransform(vtktransform);
}


void BaseVtkMapper3D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( IsVisible(renderer)==false ) 
    return;
  
  if ( GetProp()->GetVisibility() )
    GetProp()->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
}

void BaseVtkMapper3D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
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
  
  if ( GetProp()->GetVisibility() )
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=3)  )
    GetProp()->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
#else
    GetProp()->RenderTranslucentGeometry(renderer->GetVtkRenderer());
#endif

}


void BaseVtkMapper3D::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( IsVisible(renderer)==false ) 
    return;
  
  if ( GetProp()->GetVisibility() )
    GetProp()->RenderOverlay(renderer->GetVtkRenderer());

  // Render annotations as overlay
  m_LabelActorCollection->InitTraversal();
  vtkProp3D *labelActor;
  for ( m_LabelActorCollection->InitTraversal(); 
        (labelActor = m_LabelActorCollection->GetNextProp3D()); )
  {
    if ( labelActor->GetVisibility() )
      labelActor->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}


void BaseVtkMapper3D::ApplyProperties(vtkActor* actor, BaseRenderer* renderer)
{
  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  this->GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  this->GetOpacity(rgba[3], renderer);

  // Combine renderer-specific and universal properties into one list
  PropertyList::Pointer propertyList = PropertyList::New();
  propertyList->ConcatenatePropertyList(
    this->GetDataTreeNode()->GetPropertyList( renderer ) );
  propertyList->ConcatenatePropertyList(
    this->GetDataTreeNode()->GetPropertyList( NULL ) );

  const PropertyList::PropertyMap *propertyMap =
    propertyList->GetMap();
  
  // Add annotations to assembly, if any (camera (renderer) must be present)
  if ( renderer != NULL )
  {
    m_LabelActorCollection->RemoveAllItems();
    
    PropertyList::PropertyMap::const_iterator it;
    for ( it = propertyMap->begin(); it != propertyMap->end(); ++it )
    {
      AnnotationProperty *annotationProperty =
        dynamic_cast< AnnotationProperty * >( (*it).second.first.GetPointer() );

      if ( annotationProperty != NULL )
      {
        vtkVectorText *labelText = vtkVectorText::New();
        vtkPolyDataMapper *labelMapper = vtkPolyDataMapper::New();
        labelMapper->SetInput( labelText->GetOutput() );

        vtkFollower *labelFollower = vtkFollower::New();
        labelFollower->SetMapper( labelMapper );
        labelFollower->SetCamera( renderer->GetVtkRenderer()->GetActiveCamera() );

        labelFollower->SetScale( 2.0, 2.0, 2.0 );

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
  }


#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double drgba[4]={rgba[0],rgba[1],rgba[2],rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
#else
  actor->GetProperty()->SetColor(rgba);
  actor->GetProperty()->SetOpacity(rgba[3]);
#endif
}

} // namespace

