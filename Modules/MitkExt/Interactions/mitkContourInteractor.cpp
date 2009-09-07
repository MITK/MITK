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

#include "mitkContourInteractor.h"

#include <mitkContour.h>

//#include "ipSegmentation.h"
//#include "mitkDataTreeNode.h"
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkPointOperation.h>
#include <mitkAction.h>
#include <mitkPositionEvent.h>
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>
#include <mitkVtkPropRenderer.h>

#include <vtkRenderer.h>

mitk::ContourInteractor::ContourInteractor(const char * type, mitk::DataTreeNode* dataTreeNode)
  : mitk::Interactor(type, dataTreeNode), m_Started(false)
{
  assert(m_DataTreeNode != NULL);

  m_DataTreeNode->SetProperty("layer", mitk::IntProperty::New(100) );
  m_DataTreeNode->SetProperty("name", mitk::StringProperty::New("InteractiveFeedbackData") );
  m_DataTreeNode->SetOpacity(1);
  m_DataTreeNode->SetColor(0.4,0.9,0.0);
  m_DataTreeNode->SetProperty( "Width", mitk::FloatProperty::New(2.0) );
  m_Started = false;
}

mitk::ContourInteractor::~ContourInteractor()
{

}

//mitk::Contour::Pointer ContourInteractor::ExtractContour(mitkIpPicDescriptor* pic)
//{
//  int idx;
//  int size = _mitkIpPicElements (pic);
//  for (idx = 0; idx < size; idx++)
//    if ( ((ipUInt1_t*) pic->data)[idx]> 0) break;
//
//  int sizePoints;  // size of the _points buffer (number of coordinate pairs that fit in)
//  int numPoints;    // number of coordinate pairs stored in _points buffer
//  float *points = 0;
//
//  points = ipSegmentationGetContour8N( pic, idx, numPoints, sizePoints, points );
//
//  mitk::Contour::Pointer contour = mitk::Contour::New(); 
//  contour->Initialize();
//  mitk::Point3D pointInMM, pointInUnits;
//  mitk::Point3D itkPoint;
//  for (int pointIdx = 0; pointIdx < numPoints; pointIdx++)
//  {
//    pointInUnits[0] = points[2*pointIdx];
//    pointInUnits[1] = points[2*pointIdx+1];
//    pointInUnits[2] = m_ZCoord;
//    m_SelectedImageGeometry->IndexToWorld(CorrectPointCoordinates(pointInUnits),pointInMM);
//    contour->AddVertex(pointInMM);
//  }
//  return contour;
//}

bool mitk::ContourInteractor::ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent)
{
  mitk::Point3D eventPoint;

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
    vtkFloatingPointType *vtkwp = sender->GetVtkRenderer()->GetWorldPoint();
    vtk2itk(vtkwp, eventPoint);
  }
  else
  {
    eventPoint = posEvent->GetWorldPosition();
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
        Release(eventPoint);
        ok = true;
        m_Started = false;
      }
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

void mitk::ContourInteractor::Press(mitk::Point3D& point)
{
  mitk::Contour* contour = dynamic_cast<mitk::Contour*>(m_DataTreeNode->GetData());
  assert(contour!=NULL);

  if (!m_Positive)
    m_DataTreeNode->SetColor(1.0,0.0,0.0);

  contour->Initialize();
  contour->AddVertex( point );
}

void mitk::ContourInteractor::Move(mitk::Point3D& point)
{
  mitk::Contour* contour = dynamic_cast<mitk::Contour*>(m_DataTreeNode->GetData());
  assert(contour!=NULL);

  contour->AddVertex( point );
//  m_Parent->UpdateWidgets();
}

void mitk::ContourInteractor::Release(mitk::Point3D& /*point*/)
{
//vermutlich  m_Parent->UpdateWidgets();
}
