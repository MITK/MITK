/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkActionOriginScope_h
#define mitkActionOriginScope_h

#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
  /**
   * \brief Names who acts on the current thread for as long as it lives.
   *
   * Undo steps and progress tasks remember the name that was current when
   * they were created, so that a client that acts on behalf of the user (for
   * example a REST client) can tell its own steps and tasks from the user's.
   * The empty name is the user.
   *
   * The name is kept per thread. Scopes nest: the destructor restores the
   * name that was current before the scope was opened. Code that continues
   * work later or on another thread has to carry the name along (read it with
   * GetCurrent() where the work is handed over) and open a scope with it
   * there. Otherwise what that code creates counts as the user's.
   *
   * \ingroup Undo
   */
  class MITKCORE_EXPORT ActionOriginScope
  {
  public:
    /**
     * \brief Make \c origin the current name of this thread.
     * \param[in] origin Who acts; the empty string is the user.
     */
    explicit ActionOriginScope(const std::string& origin);

    /** \brief Restore the name that was current before this scope. */
    ~ActionOriginScope();

    ActionOriginScope(const ActionOriginScope&) = delete;
    ActionOriginScope& operator=(const ActionOriginScope&) = delete;

    /**
     * \brief The name that is current on the calling thread.
     * \return The name of the innermost open scope, empty without a scope.
     */
    static std::string GetCurrent();

  private:
    std::string m_Previous;
  };
}

#endif
