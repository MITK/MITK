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

#include "mitkExtrudedContourInteractor.h"

#include <mitkContour.h>
#include <mitkExtrudedContour.h>

#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkPointOperation.h>
#include <mitkAction.h>
#include <mitkPositionEvent.h>
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkVtkPropRenderer.h>
#include <mitkDataTreeNodeFactory.h>
#include <mitkVtkRepresentationProperty.h>

#include <vtkRenderer.h>
#include <vtkCamera.h>

mitk::ExtrudedContourInteractor::ExtrudedContourInteractor(const char * type, mitk::DataTreeNode* dataTreeNode)
  : mitk::Interactor(type, dataTreeNode), m_Started(false)
{
  assert(m_DataTreeNode != NULL);

  m_DataTreeNode->SetProperty( "representation", new mitk::VtkRepresentationProperty("surface") );

  m_Contour = mitk::Contour::New();

  m_ContourNode = mitk::DataTreeNode::New();
  m_ContourNode->SetData(m_Contour);

  m_ContourNode->SetProperty("layer", new mitk::IntProperty(100) );
  m_ContourNode->SetProperty("name", new mitk::StringProperty("InteractiveFeedbackData") );
  m_ContourNode->SetOpacity(1);
  m_ContourNode->SetColor(0.4,0.9,0.0);
  m_ContourNode->SetProperty( "Width", new mitk::FloatProperty(2.0) );
  m_Started = false;
}

mitk::ExtrudedContourInteractor::~ExtrudedContourInteractor()
{

}

//mitk::Contour::Pointer ExtrudedContourInteractor::ExtractContour(ipPicDescriptor* pic)
//{
//  int idx;
//  int size = _ipPicElements (pic);
//  for (idx = 0; idx < size; idx++)
//    if ( ((ipUInt1_t*) pic->data)[idx]> 0) break;
//
//  int sizePoints;	// size of the _points buffer (number of coordinate pairs that fit in)
//  int numPoints;		// number of coordinate pairs stored in _points buffer
//  float *points = 0;
//
//  points = ipSegmentationGetContour8N( pic, idx, numPoints, sizePoints, points );
//
//  mitk::Contour::Pointer m_Contour = mitk::Contour::New(); 
//  m_Contour->Initialize();
//  mitk::Point3D pointInMM, pointInUnits;
//  mitk::Point3D itkPoint;
//  for (int pointIdx = 0; pointIdx < numPoints; pointIdx++)
//  {
//    pointInUnits[0] = points[2*pointIdx];
//    pointInUnits[1] = points[2*pointIdx+1];
//    pointInUnits[2] = m_ZCoord;
//    m_SelectedImageGeometry->IndexToWorld(CorrectPointCoordinates(pointInUnits),pointInMM);
//    m_Contour->AddVertex(pointInMM);
//  }
//  return m_Contour;
//}

bool mitk::ExtrudedContourInteractor::ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent)
{
  mitk::Point3D eventPoint;
  mitk::Vector3D eventPlaneNormal;

  const mitk::PositionEvent* posEvent = dynamic_cast<const mitk::PositionEvent*>(stateEvent->GetEvent());

  if(posEvent==NULL)
  {
    const mitk::DisplayPositionEvent* displayPosEvent = dynamic_cast<const mitk::DisplayPositionEvent*>(stateEvent->GetEvent());
    mitk::VtkPropRenderer* sender = (mitk::VtkPropRenderer*) stateEvent->GetEvent()->GetSender();

    if((displayPosEvent == NULL) || (sender == NULL))
      return false;

    eventPoint[0] = displayPosEvent->GetDisplayPosition()[0];
    eventPoint[1] = displayPosEvent->GetDisplayPosition()[1];
    eventPoint[2] = 0;

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    typedef itk::Point<double,3> DoublePoint3D;
    DoublePoint3D p;
    p.CastFrom(eventPoint);
    sender->GetVtkRenderer()->SetDisplayPoint(p.GetDataPointer());
#else
    sender->GetVtkRenderer()->SetDisplayPoint(eventPoint.GetDataPointer());
#endif
    sender->GetVtkRenderer()->DisplayToWorld();


#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double *vtkwp = sender->GetVtkRenderer()->GetWorldPoint();
#else
    float *vtkwp = sender->GetVtkRenderer()->GetWorldPoint();
#endif
    vtk2itk(vtkwp, eventPoint);

    double *vtkvpn = sender->GetVtkRenderer()->GetActiveCamera()->GetViewPlaneNormal();
    vtk2itk(vtkvpn, eventPlaneNormal); eventPlaneNormal = -eventPlaneNormal;
  }
  else
  {
    eventPoint = posEvent->GetWorldPosition();
    mitk::BaseRenderer* sender = (mitk::BaseRenderer*) stateEvent->GetEvent()->GetSender();
    eventPlaneNormal = sender->GetCurrentWorldGeometry2D()->GetAxisVector(2);
  }


  bool ok = false;
  switch (action->GetActionId())
  {
  case mitk::AcNEWPOINT:
    {
      Press(eventPoint);
      ok = true;
      m_Started = true;
      break;
    }
  case mitk::AcINITMOVEMENT:
    {
      if (m_Started)
      {
        Move(eventPoint);
        ok = true;
        break;
      }
    }
  case mitk::AcMOVEPOINT:
    {
      if (m_Started)
      {
        Move(eventPoint);
        ok = true;
        break;
      }
    }
  case mitk::AcFINISHMOVEMENT:
    {
      if (m_Started)
      {
        mitk::ExtrudedContour* extrudedcontour = dynamic_cast<mitk::ExtrudedContour*>(m_DataTreeNode->GetData());
        extrudedcontour->SetContour(m_Contour);
        extrudedcontour->SetVector(eventPlaneNormal);
        Release(eventPoint);
        ok = true;
        m_Started = false;
        InvokeEvent(itk::EndEvent());
      }
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

void mitk::ExtrudedContourInteractor::Press(mitk::Point3D& point)
{
  if (!m_Positive)
    m_ContourNode->SetColor(1.0,0.0,0.0);

  m_Contour->Initialize();
  m_Contour->AddVertex( point );
}

void mitk::ExtrudedContourInteractor::Move(mitk::Point3D& point)
{
  assert(m_Contour.IsNotNull());

  m_Contour->AddVertex( point );
//  m_Parent->UpdateWidgets();
}

void mitk::ExtrudedContourInteractor::Release(mitk::Point3D& /*point*/)
{
//vermutlich  m_Parent->UpdateWidgets();
}
