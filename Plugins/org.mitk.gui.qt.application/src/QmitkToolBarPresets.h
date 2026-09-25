/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToolBarPresets_h
#define QmitkToolBarPresets_h

#include <org_mitk_gui_qt_application_Export.h>

#include <QString>
#include <QStringList>

#include <vector>

/**
 * \brief A named set of view categories whose tool bars are shown together.
 */
struct MITK_QT_APP QmitkToolBarPreset
{
  QString Name;

  /** \brief Description in Qt rich text format. */
  QString Info;

  QStringList Categories;
};

/**
 * \brief Tool bar presets and the visibility of the per-category view tool bars.
 *
 * Every view category gets its own tool bar in the workbench window. Its
 * visibility is stored as a boolean named after the category in the
 * QmitkApplicationConstants::TOOL_BARS_PREFERENCES node, where a missing
 * value means visible. The View Navigator hides the same categories. A preset
 * is a named selection of categories to show.
 */
class MITK_QT_APP QmitkToolBarPresets
{
public:
  /**
   * \brief Loads the presets shipped as resources, sorted by name.
   *
   * A preset may name an ancestor whose categories it inherits.
   *
   * \throw mitk::Exception if a preset cannot be read or parsed, or if its
   *        ancestor is missing or inherits from it.
   */
  static std::vector<QmitkToolBarPreset> Load();

  /**
   * \brief Returns all view categories, each of which has a tool bar.
   */
  static QStringList GetCategories();

  static QStringList GetVisibleCategories();

  /**
   * \brief Shows the tool bars of the given categories and hides all others.
   *
   * The visibility is stored in the preferences and applied to the tool bars
   * of all open workbench windows.
   */
  static void SetVisibleCategories(const QStringList& categories);

  /**
   * \brief Applies the stored tool bar preferences to all open workbench windows.
   */
  static void ApplyToWorkbench();
};

#endif
