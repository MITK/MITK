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

#ifndef MITKDISPATCHER_H_
#define MITKDISPATCHER_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkEvent.h"
#include "mitkCommon.h"
#include "mitkInteractor.h" // TODO: Change to EventHandler, when done.
#include <MitkExports.h>


namespace mitk
{

  class MITK_CORE_EXPORT Dispatcher : public itk::Object {

  public:
    mitkClassMacro(Dispatcher, itk::Object);
    itkNewMacro(Self);

    bool ProcessEvent(mitk::Event event);

  protected:
    Dispatcher();
    virtual ~Dispatcher();

  private:
    std::list<Interactor::Pointer> m_EventHandler; // TODO: Change to EventHandler Type, when done.
  };

} /* namespace mitk */
#endif /* MITKDISPATCHER_H_ */
