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


#include "mitkVerboseLimitedLinearUndo.h"
#include "mitkOperationEvent.h"

mitk::VerboseLimitedLinearUndo::VerboseLimitedLinearUndo()
{
}

mitk::VerboseLimitedLinearUndo::~VerboseLimitedLinearUndo()
{
}

bool mitk::VerboseLimitedLinearUndo::SetOperationEvent(UndoStackItem* undoStackItem)
{
  if (!undoStackItem) return false;

  // clear the redolist, if a new operation is saved
  if (!m_RedoList.empty()) 
  {
    m_RedoList.clear();
    InvokeEvent( RedoEmptyEvent() );
  }

  m_UndoList.push_back(undoStackItem);

  InvokeEvent( UndoNotEmptyEvent() );

  return true;
}

mitk::VerboseLimitedLinearUndo::StackDescription mitk::VerboseLimitedLinearUndo::GetUndoDescriptions()
{
  mitk::VerboseLimitedLinearUndo::StackDescription descriptions;
  
  if(m_UndoList.empty()) return descriptions;

  int oeid = m_UndoList.back()->GetObjectEventId();  // ObjectEventID of current group
  std::string currentDescription;                    // description of current group
  int currentDescriptionCount(0);                    // counter, how many items of the current group gave descriptions
  bool niceDescriptionFound(false);                  // have we yet seen a plain descriptive entry (not OperationEvent)?
  std::string lastDescription;                       // stores the last description to inhibit entries like "name AND name AND name..." if name is always the same

  for ( std::vector<UndoStackItem*>::reverse_iterator iter = m_UndoList.rbegin(); iter != m_UndoList.rend(); ++iter )
  {
    if ( oeid != (*iter)->GetObjectEventId() )
    {
      // current description complete, append to list
      if ( currentDescription.empty() )
        currentDescription = "Some unnamed action";  // set a default description
      
      descriptions.push_back( StackDescriptionItem(oeid,currentDescription) );
      
      currentDescription = "";                 // prepare for next group
      currentDescriptionCount = 0;
      niceDescriptionFound = false;
      oeid = (*iter)->GetObjectEventId();
    }

    if ( !(*iter)->GetDescription().empty() )  // if there is a description
    {
      if (!dynamic_cast<OperationEvent*>(*iter))
      {
        // anything but an OperationEvent overrides the collected descriptions
        currentDescription = (*iter)->GetDescription();
        niceDescriptionFound = true;
      }
      else if (!niceDescriptionFound) // mere descriptive items override OperationEvents' descriptions
      {
        if ( currentDescriptionCount )            // if we have already seen another description
        {
          if (lastDescription != (*iter)->GetDescription())
          {
            //currentDescription += '\n';           // concatenate descriptions with newline
            currentDescription += " AND ";          // this has to wait until the popup can process multiline items
            currentDescription += (*iter)->GetDescription();
          }
        }
        else
        {
          currentDescription += (*iter)->GetDescription();
        }
      }
      lastDescription = (*iter)->GetDescription();
      ++currentDescriptionCount;
    }

  } // for

  // add last description to list
  if ( currentDescription.empty() )
    currentDescription = "Some unnamed action";
  descriptions.push_back( StackDescriptionItem( oeid, currentDescription) );

  return descriptions;  // list ready
}

mitk::VerboseLimitedLinearUndo::StackDescription mitk::VerboseLimitedLinearUndo::GetRedoDescriptions()
{
  mitk::VerboseLimitedLinearUndo::StackDescription descriptions;
  
  if(m_RedoList.empty()) return descriptions;

  int oeid = m_RedoList.back()->GetObjectEventId();  // ObjectEventID of current group
  std::string currentDescription;                    // description of current group
  int currentDescriptionCount(0);                    // counter, how many items of the current group gave descriptions
  bool niceDescriptionFound(false);                  // have we yet seen a plain descriptive entry (not OperationEvent)?
  std::string lastDescription;                       // stores the last description to inhibit entries like "name AND name AND name..." if name is always the same

  for ( std::vector<UndoStackItem*>::reverse_iterator iter = m_RedoList.rbegin(); iter != m_RedoList.rend(); ++iter )
  {
    if ( oeid != (*iter)->GetObjectEventId() )
    {
      // current description complete, append to list
      if ( currentDescription.empty() )
        currentDescription = "Some unnamed action";  // set a default description
      
      descriptions.push_back( StackDescriptionItem( oeid, currentDescription) );
      
      currentDescription = "";                 // prepare for next group
      currentDescriptionCount = 0;
      niceDescriptionFound = false;
      oeid = (*iter)->GetObjectEventId();
    }

    if ( !(*iter)->GetDescription().empty() )  // if there is a description
    {
   
      if (!dynamic_cast<OperationEvent*>(*iter))
      {
        // anything but an OperationEvent overrides the collected descriptions
        currentDescription = (*iter)->GetDescription();
        niceDescriptionFound = true;
      }
      else if (!niceDescriptionFound) // mere descriptive items override OperationEvents' descriptions
      {
        if ( currentDescriptionCount )            // if we have already seen another description
        {
          if (lastDescription != (*iter)->GetDescription())
          {
            //currentDescription += '\n';           // concatenate descriptions with newline
            currentDescription += " AND ";          // this has to wait until the popup can process multiline items
            currentDescription += (*iter)->GetDescription();
          }
        }
        else
        {
          currentDescription += (*iter)->GetDescription();
        }
      }
      lastDescription = (*iter)->GetDescription();
      ++currentDescriptionCount;
    }

  } // for

  // add last description to list
  if ( currentDescription.empty() )
    currentDescription = "Some unnamed action";
  descriptions.push_back( StackDescriptionItem( oeid, currentDescription) );

  return descriptions;  // list ready
}


