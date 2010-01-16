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

#include "berrySafeRunner.h"
#include "berrySolsticeExceptions.h"

#include <typeinfo>

namespace berry
{

void SafeRunner::Run(ISafeRunnable::Pointer code)
{
  poco_assert(code);
  try
  {
    code->Run();
  } catch (const std::exception& e)
  {
    HandleException(code, e);
  } catch (...)
  {
    HandleException(code);
  }
}

void SafeRunner::HandleException(ISafeRunnable::Pointer code,
    const std::exception& e)
{
  try {
    dynamic_cast<const OperationCanceledException&>(e);
  }
    catch (const std::bad_cast& )
    {
      // TODO proper exception logging
//    // try to obtain the correct plug-in id for the bundle providing the safe runnable
//    Activator activator = Activator.getDefault();
//    String pluginId = null;
//    if (activator != null)
//      pluginId = activator.getBundleId(code);
//    if (pluginId == null)
//      pluginId = IRuntimeConstants.PI_COMMON;
//    String message = NLS.bind(CommonMessages.meta_pluginProblems, pluginId);
//    IStatus status;
//    if (e instanceof CoreException) {
//      status = new MultiStatus(pluginId, IRuntimeConstants.PLUGIN_ERROR, message, e);
//      ((MultiStatus) status).merge(((CoreException) e).getStatus());
//    } else {
//      status = new Status(IStatus.ERROR, pluginId, IRuntimeConstants.PLUGIN_ERROR, message, e);
//    }

//    // Make sure user sees the exception: if the log is empty, log the exceptions on stderr
//    if (!RuntimeLog.isEmpty())
//    {
//      RuntimeLog.log(status);
//    }
//    else
//    {
      //e.printStackTrace();
      std::cerr << e.what() << std::endl;
//    }
  }
  code->HandleException(e);
}

}
