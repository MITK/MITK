#ifndef _MITK_DISPLAY_INTERACTOR_CPP_
#define _MITK_DISPLAY_INTERACTOR_CPP_

#include "mitkDisplayInterActor.h"

#include <OperationActor.h>
#include <EventMapper.h>
#include <GlobalInteraction.h>
#include <mitkCoordinateSupplier.h>
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>


#include <BaseRenderer.h>
#include <mitkRenderWindow.h>

#include <mitkInteractionConst.h>

mitk::DisplayInteractor::DisplayInteractor(mitk::BaseRenderer * ren)
{
  m_ParentRenderer = ren;
}

void mitk::DisplayInteractor::ExecuteOperation(mitk::Operation * operation)
{

  bool ok;//as return type

  mitk::DisplayCoordinateOperation* dcOperation=dynamic_cast<mitk::DisplayCoordinateOperation*>(operation);
  if ( dcOperation != NULL )
  {
    /****ZOOM & MOVE of the whole volume****/
    mitk::BaseRenderer* renderer = dcOperation->GetRenderer();
    if( renderer == NULL || (m_ParentRenderer != NULL && m_ParentRenderer != renderer))
      return;
    switch (operation->GetOperationType())
    {
    case OpMOVE :
      {
        renderer->GetDisplayGeometry()->MoveBy(dcOperation->GetLastToCurrentDisplayVector()*(-1.0));
        renderer->GetRenderWindow()->Update();
        ok = true;
      }
      break;
    case OpZOOM :
      {
        float distance = dcOperation->GetLastToCurrentDisplayVector().y;
        distance = (distance > 0 ? 1 : (distance < 0 ? -1 : 0));
        float factor= 1.0 + distance * 0.05;
        //renderer->GetDisplayGeometry()->Zoom(factor, dcOperation->GetStartDisplayCoordinate());
        renderer->GetDisplayGeometry()->Zoom(factor, 
          Vector2f(renderer->GetDisplayGeometry()->GetDisplayWidth()/2,
                    renderer->GetDisplayGeometry()->GetDisplayHeight()/2));
        renderer->GetRenderWindow()->Update();
        ok = true;
      }
      break;
    default:
      ;
    }
  }
}


#endif // _MITK_DISPLAY_INTERACTOR_CPP_

