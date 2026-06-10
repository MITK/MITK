/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUndoRedoPreferenceHelper_h
#define mitkUndoRedoPreferenceHelper_h

#include <MitkCoreExports.h>

namespace mitk
{
  class IPreferences;

  /**
   * \brief Shared access to the undo/redo preferences (node "/General/UndoRedo").
   *
   * Used by the UndoController and by the Undo/Redo UI so the node path, the keys
   * and the default all live in one place and cannot drift apart.
   *
   * Two keys are used:
   *  - "UndoLimit"     : the active limit; 0 means "unlimited". This is the value the
   *                      UndoController reads on start-up.
   *  - "LastUndoLimit" : the most recent positive limit the user chose. Used to restore
   *                      a sensible value when a limit is re-enabled after "unlimited".
   */
  namespace UndoRedoPreferenceHelper
  {
    /** \brief Returns the "/General/UndoRedo" preferences node, or nullptr if the
     *         preferences service is unavailable. */
    MITKCORE_EXPORT IPreferences* GetPreferences();

    /** \brief The currently configured undo limit (0 = unlimited).
     *         Falls back to mitk::DEFAULT_UNDO_REDO_LIMIT if nothing is stored. */
    MITKCORE_EXPORT int GetActiveLimit();

    /** \brief The most recent positive (non-zero) limit the user chose.
     *         Falls back to mitk::DEFAULT_UNDO_REDO_LIMIT if nothing is stored. */
    MITKCORE_EXPORT int GetLastPositiveLimit();

    /** \brief Persists the active limit. Always writes "UndoLimit"; when \p limit is
     *         greater than 0 it also records it as "LastUndoLimit". No-op if the
     *         preferences service is unavailable. */
    MITKCORE_EXPORT void StoreLimit(int limit);
  }
}

#endif
