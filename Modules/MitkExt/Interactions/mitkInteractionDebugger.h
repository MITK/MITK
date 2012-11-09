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

#ifndef MITKINTERACTIONDEBUGGER_H
#define MITKINTERACTIONDEBUGGER_H

#include <itkObject.h>
#include "MitkExtExports.h"
#include "mitkCommon.h"


namespace mitk{

  /**
  * @brief prints out information for debugging purpose
  *
  * prints out recieved text in a sorted way
  * @ingroup Interaction
  */
  class MitkExt_EXPORT InteractionDebugger : public itk::Object
  {
  public:
    /**
    * set a text for output
    * @param sender: who has sent this text
    */
    static void Set(const char* sender, const char* text);
    static void Activate();
    static void Deactivate();

  protected:
    InteractionDebugger();
    ~InteractionDebugger();

  private:
    static const char* m_CurrentSender;
    static bool m_Active;
  };

}//namespace mitk

#endif /* MITKINTERACTIONDEBUGGER_H */


