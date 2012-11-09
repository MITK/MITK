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


#include "mitkDisplayPositionEvent.h"
#include "mitkBaseRenderer.h"

mitk::DisplayPositionEvent::DisplayPositionEvent(
  mitk::BaseRenderer *sender, int type, int button, int buttonState, int key,
  const mitk::Point2D& displPosition )
: Event( sender, type, button, buttonState, key ),
  m_DisplayPosition( displPosition ),
  m_WorldPositionIsSet( false ),
  m_PickedObjectIsSet( false )
{
}

const mitk::Point3D& mitk::DisplayPositionEvent::GetWorldPosition() const
{
  // Method performs position picking and sets world position
  if ( m_WorldPositionIsSet )
    return m_WorldPosition;

  assert( m_Sender != NULL );

  m_Sender->PickWorldPoint( m_DisplayPosition, m_WorldPosition );

  m_WorldPositionIsSet = true;

  return m_WorldPosition;
}

mitk::DataNode *mitk::DisplayPositionEvent::GetPickedObjectNode() const
{
  // Method performs object picking and sets both object and world position
  if ( m_PickedObjectIsSet )
  {
    return m_PickedObjectNode;
  }

  assert( m_Sender != NULL );

  m_PickedObjectNode = m_Sender->PickObject( m_DisplayPosition, m_WorldPosition );

  m_PickedObjectIsSet = true;
  m_WorldPositionIsSet = true;

  return m_PickedObjectNode;
}

mitk::BaseData *mitk::DisplayPositionEvent::GetPickedObject() const
{
  mitk::DataNode *node = this->GetPickedObjectNode();

  if ( node != NULL )
  {
    return node->GetData();
  }
  else
  {
    return NULL;
  }
}
