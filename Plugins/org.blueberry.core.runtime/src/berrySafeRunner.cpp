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

#include "berrySafeRunner.h"
#include "berryOperationCanceledException.h"
#include "berryCoreException.h"
#include "berryMultiStatus.h"

#include "internal/berryIRuntimeConstants.h"
#include "internal/berryCTKPluginActivator.h"

#include <typeinfo>

namespace berry
{

void SafeRunner::Run(ISafeRunnable::Pointer code)
{
  poco_assert(code);
  try
  {
    code->Run();
  }
  catch (const ctkException& e)
  {
    HandleException(code, e);
  }
  catch (const std::exception& e)
  {
    HandleException(code, e);
  }
  catch (...)
  {
    HandleException(code);
  }
}

void SafeRunner::HandleException(ISafeRunnable::Pointer code)
{
  HandleException(code, ctkException("Unknown exception thrown"));
}

void SafeRunner::HandleException(ISafeRunnable::Pointer code,
                                 const std::exception& e)
{
  HandleException(code, ctkException(e.what()));
}

void SafeRunner::HandleException(ISafeRunnable::Pointer code,
                                 const ctkException& e)
{
  try {
    Q_UNUSED(dynamic_cast<const OperationCanceledException&>(e))
  }
  catch (const std::bad_cast& )
  {
    // try to obtain the correct plug-in id for the bundle providing the safe runnable
    QString pluginId = CTKPluginActivator::getPluginId(static_cast<void*>(code.GetPointer()));
    if (pluginId.isEmpty())
      pluginId = IRuntimeConstants::PI_RUNTIME();
    QString message = QString("Problems occurred when invoking code from plug-in: \"%1\".").arg(pluginId);
    IStatus::Pointer status;
    try {
      const CoreException& coreExc = dynamic_cast<const CoreException&>(e);
      MultiStatus::Pointer multiStatus(new MultiStatus(pluginId, IRuntimeConstants::PLUGIN_ERROR,
                                                       message, e, BERRY_STATUS_LOC));
      multiStatus->Merge(coreExc.GetStatus());
      status = multiStatus;
    }
    catch (const std::bad_cast&)
    {
      IStatus::Pointer tmpStatus(new Status(IStatus::ERROR_TYPE, pluginId,
                                            IRuntimeConstants::PLUGIN_ERROR, message, e, BERRY_STATUS_LOC));
      status = tmpStatus;
    }
    // Make sure user sees the exception: if the log is empty, log the exceptions on stderr
//    if (!RuntimeLog.isEmpty())
//      RuntimeLog.log(status);
//    else
    qWarning() << e.printStackTrace();
  }
  code->HandleException(e);
}

}
