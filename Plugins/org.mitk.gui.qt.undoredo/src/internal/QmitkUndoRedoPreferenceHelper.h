/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUndoRedoPreferenceHelper_h
#define QmitkUndoRedoPreferenceHelper_h

namespace mitk
{
  class IPreferences;
}

/**
 * \brief Shared access to the Undo/Redo plugin preferences.
 *
 * Both QmitkUndoRedoPreferencePage and QmitkUndoRedoView read and write the same
 * "/General/UndoRedo" preferences. These helpers keep the node path, the keys and
 * the default in one place so the two screens cannot drift apart.
 *
 * Two keys are used:
 *  - "UndoLimit"     : the active limit; 0 means "unlimited". This is the value the
 *                      UndoController reads on start-up.
 *  - "LastUndoLimit" : the most recent positive limit the user chose. Used to restore
 *                      a sensible value when a limit is re-enabled after "unlimited".
 */
namespace QmitkUndoRedoPreferences
{
  /** \brief Returns the "/General/UndoRedo" preferences node, or nullptr if the
   *         preferences service is unavailable. */
  mitk::IPreferences* GetPreferences();

  /** \brief The currently configured undo limit (0 = unlimited).
   *         Falls back to mitk::DEFAULT_UNDO_REDO_LIMIT if nothing is stored. */
  int GetActiveLimit();

  /** \brief The most recent positive (non-zero) limit the user chose.
   *         Falls back to mitk::DEFAULT_UNDO_REDO_LIMIT if nothing is stored. */
  int GetLastPositiveLimit();

  /** \brief Persists the active limit. Always writes "UndoLimit"; when \p limit is
   *         greater than 0 it also records it as "LastUndoLimit". No-op if the
   *         preferences service is unavailable. */
  void StoreLimit(int limit);
}

#endif
