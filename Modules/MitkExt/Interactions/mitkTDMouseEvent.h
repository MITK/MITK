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


#ifndef TDMOUSEEVENT_H_HEADER_INCLUDED
#define TDMOUSEEVENT_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkEvent.h"
#include "mitkVector.h"


namespace mitk {

//##Documentation
//## @brief Event on 3D Mouse input
//##
//## Seven coordinates exposed by the 3D Mouse:
//## 3-dimensional translation vector
//## 3-dimensional rotation achsis (length allways 1.0)
//## scalar rotation angle
//## @ingroup Interaction
class TDMouseEvent : public Event
{
  public:
    //##Documentation
    //## @brief Constructor with all necessary arguments.
    //##
    //## buttonState: information from the Event
    TDMouseEvent(int buttonState, const Vector3D& translation, const Vector3D& rotation, const ScalarType& angle);

    const Vector3D& GetTranslation() const
    {
        return m_Translation;
    }

    void SetTranslation(const Vector3D& translation) { m_Translation = translation; }

    const Vector3D& GetRotation() const
    {
        return m_Rotation;
    }

    void SetRotation(const Vector3D& rotation) { m_Rotation = rotation; }

    const ScalarType& GetAngle() const
    {
        return m_Angle;
    }

    void SetAngle(const ScalarType& angle) { m_Angle = angle; }


  protected:
    Vector3D m_Translation;
    Vector3D m_Rotation;
    ScalarType m_Angle;

};

} // namespace mitk

#endif /* TDMOUSEEVENT_H_HEADER_INCLUDED */
