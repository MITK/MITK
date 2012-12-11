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

#ifndef MITKEVENTHANDLER_H_
#define MITKEVENTHANDLER_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkEvent.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkEventConfig.h"
#include <string>


namespace mitk
{
  /** Base Class for Interaction handling classes.
   *  Handles loading of configuration object and mapping of events to variant names.
   */
  class InteractionEvent;
  class MITK_CORE_EXPORT EventHandler : public itk::Object {

  public:
    mitkClassMacro(EventHandler, itk::Object);
    itkNewMacro(Self);

  protected:
    EventHandler();
    virtual ~EventHandler();
    virtual bool LoadEventConfig();
    virtual bool LoadEventConfig(std::string filename);
    std::string GetMappedEvent(InteractionEvent* interactionEvent);

  private:
    EventConfig* m_EventConfig;

  };

} /* namespace mitk */
#endif /* MITKEVENTHANDLER_H_ */
