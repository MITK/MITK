/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYNESTABLEHANDLERSERVICE_H
#define BERRYNESTABLEHANDLERSERVICE_H

#include "berrySlaveHandlerService.h"
#include "berryINestable.h"

namespace berry {

/**
 * <p>
 * A handler service which delegates almost all responsibility to the parent
 * service. It is capable of being nested inside a component that is not
 * recognizable by the "sources" event mechanism. This means that the handlers
 * must be activated and deactivated manually.
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.eclipse.ui.workbench</code> plug-in.
 * </p>
 */
class NestableHandlerService : public SlaveHandlerService, public INestable
{

private:

  /**
   * Whether the component with which this service is associated is active.
   */
  bool active;

public:

  berryObjectMacro(berry::NestableHandlerService)

  /**
   * Constructs a new instance.
   *
   * @param parentHandlerService
   *            The parent handler service for this slave; must not be
   *            <code>null</code>.
   * @param defaultExpression
   *            The default expression to use if none is provided. This is
   *            primarily used for conflict resolution. This value may be
   *            <code>null</code>.
   */
  NestableHandlerService(IHandlerService* parentHandlerService,
                         const SmartPointer<Expression>& defaultExpression);

  void Activate() override;

  void Deactivate() override;

protected:

  SmartPointer<IHandlerActivation> DoActivation(
      const SmartPointer<IHandlerActivation>& localActivation) override;

};

}

#endif // BERRYNESTABLEHANDLERSERVICE_H
