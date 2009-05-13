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

#ifndef MITKINTERACTIONDEBUGGER_H
#define MITKINTERACTIONDEBUGGER_H

#include <itkObject.h>
#include "mitkCommon.h"


namespace mitk{

  /**
  * @brief prints out information for debugging purpose
  *
  * prints out recieved text in a sorted way
  * @ingroup Interaction
  */
  class MITKEXT_CORE_EXPORT InteractionDebugger : public itk::Object
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


