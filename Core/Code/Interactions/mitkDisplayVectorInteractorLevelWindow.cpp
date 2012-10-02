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


#include "mitkDisplayVectorInteractorLevelWindow.h"
#include "mitkOperation.h"
#include "mitkDisplayCoordinateOperation.h"
#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkNodePredicateDataType.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLevelWindow.h"


void mitk::DisplayVectorInteractorLevelWindow::ExecuteOperation(Operation* itkNotUsed( operation ) )
{
}

bool mitk::DisplayVectorInteractorLevelWindow::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok=false;
  
  const DisplayPositionEvent* posEvent=dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
  if(posEvent==NULL) return false;

  int actionId = action->GetActionId();
  
  switch(actionId)
  {
  case AcINITMOVE:
    // MITK_INFO << "AcINITMOVE";
    {
      m_Sender=posEvent->GetSender();
      m_StartDisplayCoordinate=posEvent->GetDisplayPosition();
      m_LastDisplayCoordinate=posEvent->GetDisplayPosition();
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      ok = true;
      break;
    }
  case AcLEVELWINDOW:
    {

      this->InvokeEvent( StartInteractionEvent() );

      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
  
      mitk::DataStorage::Pointer storage =  m_Sender->GetDataStorage();
      mitk::DataNode::Pointer node = NULL;

      mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = storage->GetSubset(mitk::NodePredicateDataType::New( "Image") );

      for ( unsigned int i = 0; i < allImageNodes->size() ; i++ )
      {
        bool isActiveImage = false;
        bool propFound = allImageNodes->at( i )->GetBoolProperty( "imageForLevelWindow", isActiveImage );

        if ( propFound && isActiveImage )
        { 
          node = allImageNodes->at( i );
          continue;
        }
      }
      if ( node.IsNull() )
      {
        node = storage->GetNode( mitk::NodePredicateDataType::New( "Image" ) );
      }

      if ( node.IsNull() )
      {
        return false;
      }

      mitk::LevelWindow lv = mitk::LevelWindow();
      node->GetLevelWindow(lv);
      ScalarType level = lv.GetLevel();
      ScalarType window = lv.GetWindow();

      // determine which index shall be used for level and which one for window
      int levelIndex = m_HorizontalLevelling ? 0 : 1;
      int windowIndex = m_HorizontalLevelling ? 1 : 0;

      // determine if one these is supposed to be inverted
      Point2D invertModifier;
      invertModifier[levelIndex] = m_InvertLevel ? -1 : 1;
      invertModifier[windowIndex] = m_InvertWindow ? -1 : 1;

      // difference of the two positions 
      // *2 to make interaction faster
      // *inverModifier to handle inverted interactionscheme
      level += ( m_CurrentDisplayCoordinate[levelIndex] - m_LastDisplayCoordinate[levelIndex] )* static_cast<ScalarType>(2) * invertModifier[levelIndex];
      window += ( m_CurrentDisplayCoordinate[windowIndex] - m_LastDisplayCoordinate[windowIndex] )* static_cast<ScalarType>(2) * invertModifier[windowIndex];

      lv.SetLevelWindow( level, window );
      dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"))->SetLevelWindow( lv );

      m_Sender->GetRenderingManager()->RequestUpdateAll();

      ok = true;
      break;
    }
  case AcFINISHMOVE:
    // MITK_INFO << "AcFINISHMOVE";
    {
      ok = true;
      this->InvokeEvent( EndInteractionEvent() );
      
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

mitk::DisplayVectorInteractorLevelWindow::DisplayVectorInteractorLevelWindow(const char * type)
  : mitk::StateMachine(type)
  , m_Sender(NULL)
  , m_HorizontalLevelling( false )
  , m_InvertLevel( false )
  , m_InvertWindow( false )
{
  m_StartDisplayCoordinate.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);
  m_UndoEnabled = false;

}


mitk::DisplayVectorInteractorLevelWindow::~DisplayVectorInteractorLevelWindow()
{
}

void mitk::DisplayVectorInteractorLevelWindow::SetHorizontalLevelling( bool enabled )
{
  m_HorizontalLevelling = enabled;
}

void mitk::DisplayVectorInteractorLevelWindow::SetInvertLevel( bool inverted )
{
  m_InvertLevel = inverted;
}

void mitk::DisplayVectorInteractorLevelWindow::SetInvertWindow( bool inverted )
{
  m_InvertWindow = inverted;
}

