#include "mitkInteractor.h"
#include <mitkDataTreeNode.h>
#include <mitkDisplayPositionEvent.h>
#include <mitkPositionEvent.h>
#include <mitkGeometry3D.h>
#include <mitkAction.h>
#include <mitkStatusBar.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <mitkOpenGLRenderer.h>
#include "mitkInteractionConst.h"
#include <vtkTransform.h>
#include <itkVector.h>




mitk::Interactor::Interactor(const char * type, DataTreeNode* dataTreeNode)
: mitk::StateMachine(type), m_DataTreeNode(dataTreeNode), m_Mode(SMDESELECTED)
{
  if (m_DataTreeNode != NULL)
    m_DataTreeNode->SetInteractor(this);
}

mitk::Interactor::SMMode mitk::Interactor::GetMode() const
{
  return m_Mode;
}

bool mitk::Interactor::IsNotSelected() const 
{
  return (m_Mode==SMDESELECTED);
}

bool mitk::Interactor::IsSelected() const 
{
  return (m_Mode!=SMDESELECTED);
}

bool mitk::Interactor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId) 
{

  switch (action->GetActionId())
  {
    case AcMODEDESELECT:
      {
        m_Mode = SMDESELECTED;
        // ToDo
        // Operation op( OPSelect, ... )
        // m_DataTreeNode->GetData()->ExecuteOperation( &op );
        return true;
      }
    case AcMODESELECT:
      {      
        m_Mode = SMSELECTED;
        // ToDo
        // Operation op( OPSelect, ... )
        // m_DataTreeNode->GetData()->ExecuteOperation( &op );
        return true;
      }
    case AcMODESUBSELECT:
      {
        mitk::StatusBar::DisplayText("Error! in XML-Interaction: an simple Interactor can not set in sub selected", 1102);
        return false;
      }
  }
  
  return false;
}

//##Documentation
//## change if derived class can handle it better!
float mitk::Interactor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnvalue = 0.0;
  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      returnvalue = 0.5;
    }
    else
    {
      returnvalue = 0;
    }

  }

  //Mouse event handling:
  else//if we have 2d or 3d position
  {
    if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)//on MouseMove do nothing! reimplement if needed differently
    {
      return 0;
    }

    //compute the center of the data taken care of
    mitk::BoundingBox *bBox = const_cast<mitk::BoundingBox*>(m_DataTreeNode->GetData()->GetGeometry()->GetBoundingBox());
    if (bBox == NULL)
      return 0;

    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL) //2D information from a 3D window
    {
      //get camera and calculate the distance between the center of this boundingbox and the camera
      vtkCamera* camera = dynamic_cast<mitk::OpenGLRenderer*>(posEvent->GetSender())->GetVtkRenderer()->GetActiveCamera();
      if (camera == NULL)
      {
        return 0;
      }
      float normal[3];
      camera->GetViewPlaneNormal(normal);
      mitk::BoundingBox::PointType center = bBox->GetCenter();
      returnvalue = center.SquaredEuclideanDistanceTo(normal);
      //map between 0.5 and 1

    }
    else//3D information from a 2D window.
    {
      mitk::Point3D const point = posEvent->GetWorldPosition();
      mitk::BoundingBox::PointType itkPoint;
      float p[3];
      p[0] = point.x;p[1] = point.y;p[2] = point.z;
      //transforming the Worldposition to local coordinatesystem
      m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);

      itkPoint[0] = p[0];itkPoint[1] = p[1];itkPoint[2] = p[2];

      //check if the given position lies inside the data-object
      if (bBox->IsInside(itkPoint))
      {
        //how near inside the center?
        mitk::BoundingBox::PointType center = bBox->GetCenter();
        //distance between center and point 
        returnvalue = 1 - itkPoint.SquaredEuclideanDistanceTo(center);
        //now in rage of 0.5 to 1 compared to size of boundingbox;
        returnvalue = returnvalue/( (bBox->GetMaximum() - bBox->GetMinimum()).GetNorm() / 2);

        //map between 0.5 and 1
        returnvalue = 0.5 + (returnvalue / 2);
      }
      else
      {
        //calculate the distance between the point and the BoundingBox
        //and set it in range between 0 and 0.5
        returnvalue = 0;
      }
    }

  }

  return returnvalue;

}

