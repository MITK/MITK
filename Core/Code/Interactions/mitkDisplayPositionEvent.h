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


#ifndef DISPLAYPOSITIONEVENT_H_HEADER_INCLUDED_C184F366
#define DISPLAYPOSITIONEVENT_H_HEADER_INCLUDED_C184F366

#include <MitkCoreExports.h>
#include "mitkEvent.h"
#include "mitkNumericTypes.h"
#include "mitkDataNode.h"


namespace mitk {

/**
 * \brief Event that stores coordinates
 *
 * Stores display position of the mouse.
 *
 * If requested, the correspondent 3D world position in mm is calculated via
 * picking (delegated to the BaseRenderer). Additionally, the mitk::BaseData or
 * mitk::DataNode corresponding to the picked object in the (3D) scene can
 * be retrieved.
 * \ingroup Interaction
 * \deprecatedSince{2013_03} mitk::DisplayPositionEvent is deprecated. Use mitk::InteractionPositionEvent instead.
 * Refer to \see DataInteractionPage  for general information about the concept of the new implementation.
 */
class MITK_CORE_EXPORT DisplayPositionEvent : public Event
{
public:
  /** \brief Constructor with all necessary arguments.
   *
   * \param sender is the renderer that caused that event
   * \param type, button, buttonState, key: information from the Event
   * \param displPosition is the 2D Position of the mouse
   */
  DisplayPositionEvent(BaseRenderer* sender, int type, int button, int buttonState, int key, const Point2D& displPosition);

  const Point2D& GetDisplayPosition() const
  {
      return m_DisplayPosition;
  }

  void SetDisplayPosition(const Point2D& displayPosition)
  {
    m_DisplayPosition = displayPosition;
    m_WorldPositionIsSet = false;
    m_PickedObjectIsSet = false;
  }

  Point3D& GetWorldPosition() const;

  /** Returns node with object at the current position (NULL if not applicable) */
  mitk::DataNode *GetPickedObjectNode() const;

  /** Returns object at the current position (NULL if not applicable) */
  mitk::BaseData *GetPickedObject() const;


protected:
  Point2D m_DisplayPosition;
  mutable Point3D m_WorldPosition;
  mutable bool m_WorldPositionIsSet;

  mutable mitk::DataNode::Pointer m_PickedObjectNode;
  mutable bool m_PickedObjectIsSet;
};

typedef DisplayPositionEvent MouseEvent;

} // namespace mitk

#endif /* DISPLAYPOSITIONozsiEVENT_H_HEADER_INCLUDED_C184F366 */
