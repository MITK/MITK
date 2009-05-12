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

#include <mitkConferenceEventMapper.h>
#include <mitkCameraController.h>
#include <mitkBaseRenderer.h>
#include <mitkPositionEvent.h>
#include <mitkRenderingManager.h>


mitk::ConferenceEventMapper::ConferenceEventMapper(){}

mitk::ConferenceEventMapper::~ConferenceEventMapper(){}

//CONFERENCE USE
bool
mitk::ConferenceEventMapper::MapEvent(signed int mitkEventID, const char* sender, int Etype, int Estate, int Ebuttonstate, int Ekey, float w1,float w2,float w3,float p1,float p2)
{

  //CONFERENCE EVENT
  mitk::BaseRenderer *br = const_cast<mitk::BaseRenderer*>(mitk::BaseRenderer::GetByName( std::string(sender) ));
  
  mitk::Point3D p3d;
  p3d[0] = (mitk::ScalarType) w1;
  p3d[1] = (mitk::ScalarType) w2;
  p3d[2] = (mitk::ScalarType) w3;

  // fit relative values to absolut....
  mitk::Point2D p2d;
  p2d[0] = (mitk::ScalarType) p1 * br->GetSizeX();
  p2d[1] = (mitk::ScalarType) p2 * br->GetSizeX();


  mitk::PositionEvent *pe = new mitk::PositionEvent(br,Etype,Estate,Ebuttonstate,Ekey,p2d,p3d);
  
  // MOUSE Overlay
  mitk::Point2D  p2d_mm, pos_unit;     
  //Map world to 2d mm
  if (br->GetDisplayGeometry()->Map( p3d, p2d_mm ) )
  {
    //calculate position of the real inner widget
    br->GetDisplayGeometry()->WorldToDisplay( p2d_mm, pos_unit);
    //std::cout<<p2d_mm[0]<<", "<<p2d_mm[1]<<", U: "<<pos_unit[0]<<", "<<pos_unit[1]<<" p3d ( "<<p3d[0]<<p3d[1]<<p3d[2]<<" ) "<<std::endl;
    br->DrawOverlayMouse(pos_unit); //TEST
    //mitk::RenderingManager::GetInstance()->RequestOverlayUpdateAll();
  }


  //std::cout<<"mitkEventMapper::MapEvent(): "<<br->GetSizeX()<<" * "<<p1<<" =X (("<<p2d[0]<<"))   UND "<<br->GetSizeY()<<" * "<< p2 <<" =Y (("<<p2d[1]<<"))"<<std::endl;
  
  //ONlY 3D Widget
  if( br->GetMapperID() == 2 )
  {
    mitk::CameraController* cc = br->GetCameraController();
    
    //Qt Event IDs
    if (Etype == 6)
    {
      // KeyPress
      //cc->KeyPressEvent( keyevent); */
      ;
    }
    else
    {
      //Umrechnung fuer VTKCameraControler
      p2d[1] = (mitk::ScalarType) (br->GetSizeY() - p2) * br->GetSizeY(); 
      mitk::PositionEvent *peVTK = new mitk::PositionEvent(br,Etype,Estate,Ebuttonstate,Ekey,p2d,p3d);
      // END
      
      //PRESS
      if(Etype == 2)
      {
        cc->MousePressEvent( peVTK );
      }
      else if (Etype == 3 )
      {
        //RELEASE
        cc->MouseReleaseEvent( peVTK );
      }
      else if (Etype == 5)
      {
        //MOVE
        cc->MouseMoveEvent( peVTK );
      }
    }
  }
  
  bool ok = EventMapper::MapEvent( pe, mitkEventID );

  return ok;
}

bool
mitk::ConferenceEventMapper::MapEvent(const char* sender, float w1, float w2, float w3)
{

  //CONFERENCE EVENT
  mitk::BaseRenderer *br = const_cast<mitk::BaseRenderer*>(mitk::BaseRenderer::GetByName( std::string(sender) ));
  
  mitk::Point3D p3d;
  p3d[0] = (mitk::ScalarType) w1;
  p3d[1] = (mitk::ScalarType) w2;
  p3d[2] = (mitk::ScalarType) w3;

  // MOUSE Overlay
  mitk::Point2D  p2d_mm, pos_unit;     
  //Map world to 2d mm
  if (br->GetDisplayGeometry()->Map( p3d, p2d_mm ) )
  {
    //calculate position of the real inner widget
    br->GetDisplayGeometry()->WorldToDisplay( p2d_mm, pos_unit);
    //std::cout<<p2d_mm[0]<<", "<<p2d_mm[1]<<", U: "<<pos_unit[0]<<", "<<pos_unit[1]<<" p3d ( "<<p3d[0]<<p3d[1]<<p3d[2]<<" ) "<<std::endl;
    br->DrawOverlayMouse(pos_unit); //TEST
    //mitk::RenderingManager::GetInstance()->RequestOverlayUpdateAll();
  }

  return true;
}


