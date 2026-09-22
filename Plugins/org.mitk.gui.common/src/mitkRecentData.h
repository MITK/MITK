/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRecentData_h
#define mitkRecentData_h

#include <org_mitk_gui_common_Export.h>

#include <QString>
#include <QStringList>

namespace mitk
{
  /**
   * \ingroup org_mitk_gui_common
   *
   * \brief Paths of recently opened or saved data, kept across sessions.
   *
   * Projects (MITK scene files) and all other files are kept in separate
   * lists, so that frequently opened files do not push projects out. Each
   * list starts with the most recent entry and is limited in length.
   */
  class MITK_GUI_COMMON_PLUGIN RecentData
  {
  public:
    enum class Kind
    {
      Project,
      File
    };

    /**
     * \brief Returns the absolute paths of the given kind, most recent first.
     */
    static QStringList Get(Kind kind);

    /**
     * \brief Moves the given paths to the front of their lists.
     *
     * MITK scene files (.mitk, .mitkscene.json) are projects, all other paths
     * are files. Relative paths are made absolute.
     */
    static void Add(const QStringList& paths);

    static void Remove(const QString& path);

    static void Clear(Kind kind);
  };
}

#endif
