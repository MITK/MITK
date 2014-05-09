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


#ifndef KeyEvent_H_HEADER_INCLUDED_C184F366
#define KeyEvent_H_HEADER_INCLUDED_C184F366

#include <MitkCoreExports.h>
#include "mitkEvent.h"
#include "mitkNumericTypes.h"


namespace mitk {

//##Documentation
//## @brief Event that stores coordinates and the key which is pressed
//##
//## Stores display position of the mouse. If requested, the correspondent
//## 3D world position in mm is calculated via picking (delegated to the
//## BaseRenderer).
//## @ingroup Interaction

  /**
   * \deprecatedSince{2013_03} KeyEvent is deprecated. Use mitk::InteractionKeyEvent instead.
   * Refer to \see DataInteractionPage  for general information about the concept of the new implementation.
   */
class MITK_CORE_EXPORT KeyEvent : public Event
{
  public:
    //##Documentation
    //## @brief Constructor with all necessary arguments.
    //##
    //## @param sender is the renderer that caused that event
    //## @param type, button, buttonState, key: information from the Event
    //## @param displPosition is the 2D Position of the mouse
    KeyEvent(BaseRenderer* sender, int type, int button, int buttonState, int key, std::string text, const Point2D& displPosition);

    const Point2D& GetDisplayPosition() const
    {
        return m_DisplayPosition;
    }

    void SetDisplayPosition(const Point2D& displPosition) {
      m_DisplayPosition = displPosition;
      m_WorldPositionIsSet = false;
    }

    const Point3D& GetWorldPosition() const;

    int GetKey() const
    {
      return m_Key;
    }

    const char* GetText() const {
      return m_Text.c_str();
    }
    int GetType() const {
      return m_Type;
    }

  protected:
    Point2D m_DisplayPosition;
    int m_Key;
    std::string m_Text;
    mutable Point3D m_WorldPosition;
    mutable bool m_WorldPositionIsSet;

};

} // namespace mitk

#endif /* DISPLAYPOSITIONozsiEVENT_H_HEADER_INCLUDED_C184F366 */
