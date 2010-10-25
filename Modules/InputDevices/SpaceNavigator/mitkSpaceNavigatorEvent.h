/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef SPACENAVIGATOREVENT_H_
#define SPACENAVIGATOREVENT_H_

#include <mitkCommon.h>
#include <mitkEvent.h>
#include <mitkVector.h>

#include <mitkSpaceNavigatorExports.h>

namespace mitk {

  /**
  * Documentation
  * @brief Event on 3D Mouse input
  *
  * Seven coordinates exposed by the 3D Mouse:
  * 3-dimensional translation vector
  * 3-dimensional rotation achsis (length allways 1.0)
  * scalar rotation angle
  *
  * @ingroup Interaction
  */
  class mitkSpaceNavigator_EXPORT SpaceNavigatorEvent : public Event
  {

  public:
    /**
    * @brief Constructor with all necessary arguments (for further information about the parameter: <br>
    * <code>mitk::SpaceNavigatorAddOn</code>.
    *
    * @param buttonState information from the Event
    * 
    */
    SpaceNavigatorEvent(int buttonState, const Vector3D& translation, const Vector3D& rotation, const ScalarType& angle);

    //Getter and Setter
    const Vector3D& GetTranslation() const
    {
      return m_Translation;
    }

    void SetTranslation(const Vector3D& translation) 
    { 
      m_Translation = translation; 
    }

    const Vector3D& GetRotation() const
    {
      return m_Rotation;
    }

    void SetRotation(const Vector3D& rotation) 
    {
      m_Rotation = rotation; 
    }

    const ScalarType& GetAngle() const
    {
      return m_Angle;
    }

    void SetAngle(const ScalarType& angle) 
    { 
      m_Angle = angle; 
    }


  protected:

    Vector3D m_Translation;
    Vector3D m_Rotation;
    ScalarType m_Angle;

  }; // end class SpaceNavigatorEvent
} // end namespace mitk

#endif /* SPACENAVIGATOREVENT_H_ */
