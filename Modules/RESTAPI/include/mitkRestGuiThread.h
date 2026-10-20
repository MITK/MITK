/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestGuiThread_h
#define mitkRestGuiThread_h

#include <mitkIRestServerService.h>

#include <exception>
#include <optional>
#include <type_traits>

namespace mitk
{
  /**
   * \brief Run a function in the GUI thread and wait for its result.
   *
   * For the handlers of REST routes, which run on threads of the server.
   * Exceptions of the function are thrown again in the calling thread.
   *
   * Do not call the server's FindNode(), GetDataStorage() and the like from
   * within the function: requests below /api/v1 hold locks while they wait
   * for the GUI thread. Resolve those before, in the handler's thread.
   */
  template <typename Function>
  auto CallInGuiThread(const IRestServerService& server, Function function) -> std::invoke_result_t<Function>
  {
    using Result = std::invoke_result_t<Function>;
    std::exception_ptr exception;

    if constexpr (std::is_void_v<Result>)
    {
      server.Dispatch([&]() {
        try
        {
          function();
        }
        catch (...)
        {
          exception = std::current_exception();
        }
      });

      if (exception)
        std::rethrow_exception(exception);
    }
    else
    {
      std::optional<Result> result;

      server.Dispatch([&]() {
        try
        {
          result.emplace(function());
        }
        catch (...)
        {
          exception = std::current_exception();
        }
      });

      if (exception)
        std::rethrow_exception(exception);

      return std::move(*result);
    }
  }
}

#endif
