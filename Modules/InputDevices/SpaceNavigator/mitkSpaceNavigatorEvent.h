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


#ifndef SPACENAVIGATOREVENT_H_
#define SPACENAVIGATOREVENT_H_

#include <mitkCommon.h>
#include <mitkEvent.h>
#include <mitkNumericTypes.h>

#include <MitkSpaceNavigatorExports.h>

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
