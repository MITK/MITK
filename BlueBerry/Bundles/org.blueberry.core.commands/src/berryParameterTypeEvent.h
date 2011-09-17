/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYPARAMETERTYPEEVENT_H_
#define BERRYPARAMETERTYPEEVENT_H_

#include "common/berryAbstractHandleObjectEvent.h"

namespace berry {

class ParameterType;

/**
 * An instance of this class describes changes to an instance of
 * {@link ParameterType}.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see IParameterTypeListener#ParameterTypeChanged(ParameterTypeEvent::Pointer)
 */
class BERRY_COMMANDS ParameterTypeEvent : public AbstractHandleObjectEvent {

public:

  berryObjectMacro(ParameterTypeEvent)

  /**
   * Returns the instance of the parameter type that changed.
   *
   * @return the instance of the parameter type that changed. Guaranteed not
   *         to be <code>null</code>.
   */
  SmartPointer<ParameterType> GetParameterType() const;


  /**
   * Constructs a new instance.
   *
   * @param parameterType
   *            The parameter type that changed; must not be <code>null</code>.
   * @param definedChanged
   *            <code>true</code>, iff the defined property changed.
   */
  ParameterTypeEvent(const SmartPointer<ParameterType> parameterType,
       bool definedChanged);


private:

  /**
   * The parameter type that has changed. This value is never
   * <code>null</code>.
   */
  const SmartPointer<ParameterType> parameterType;

};

}


#endif /* BERRYPARAMETERTYPEEVENT_H_ */
