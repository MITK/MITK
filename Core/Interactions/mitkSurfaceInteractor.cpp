
#include "mitkSurfaceInteractor.h"
#include "mitkPointLocator.h"
#include "mitkSurface.h"

#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkBaseRenderer.h"

#include <vtkPolyData.h>

mitk::SurfaceInteractor::SurfaceInteractor(const char * type, DataTreeNode* dataTreeNode)
: AffineInteractor(type, dataTreeNode)
{
  m_PtLoc = PLocType::New();
}


float mitk::SurfaceInteractor::CalculateJurisdiction( StateEvent const* stateEvent ) const
{
  float jd = 0.0f;

  if( ! ( stateEvent->GetEvent()->GetButtonState() == mitk::BS_ControlButton && stateEvent->GetEvent()->GetType() == mitk::Type_MouseButtonPress ))
    return jd;

  mitk::Surface* surf = dynamic_cast<mitk::Surface*>(this->GetData());
  if (surf)
  {
    mitk::DisplayPositionEvent const *event = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
    mitk::PointSet::PointType pt = event->GetWorldPosition();

    // Use ANN to get the point of the polydata closest to the world event
    if( surf->GetVtkPolyData(m_TimeStep) == NULL ) itkExceptionMacro(<< "No polydata at this time step!");
    m_PtLoc->SetPoints(dynamic_cast<vtkPointSet*>(surf->GetVtkPolyData(m_TimeStep)));
    mitk::PointLocator::DistanceType dst = m_PtLoc->GetMinimalDistance(pt);

    mitk::BaseRenderer* ren = stateEvent->GetEvent()->GetSender();

    // Get the diameter of the render window bounding box
    mitk::DataTree* tree = dynamic_cast<mitk::DataTree*>(ren->GetData()->GetTree());
    mitk::BoundingBox::Pointer bb = tree->ComputeBoundingBox(ren->GetData());
    mitk::BoundingBox::AccumulateType dia = bb->GetDiagonalLength2();

    if (dia > 0.00001) //if diameter not zero
    {
      float verh = dst/dia;
      if (verh>1) verh = 1; //if dst is bigger than dia, then set to 1
      //now inverse (0 = bad and 1 = good) and set between 0.5 and 1
      jd = ((1-verh)/2)+0.5f;
    }
  }
  return jd;
}
