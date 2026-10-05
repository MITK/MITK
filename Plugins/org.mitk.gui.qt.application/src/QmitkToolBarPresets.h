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

#include <QmitkCategoryToolBar.h>

#include <berryIViewDescriptor.h>
#include <berryIWorkbenchWindow.h>

#include <QString>
#include <QStringList>

#include <vector>

class QMainWindow;

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
   * \brief Adds a tool bar for each category of the given views to the main window.
   *
   * The tool bars are ordered by IsCategoryBefore(). Views without a
   * category share a tool bar without a name. The stored visibility and
   * category label are applied.
   */
  static void CreateToolBars(berry::IWorkbenchWindow* window, QMainWindow* mainWindow, const QList<berry::IViewDescriptor::Pointer>& views);

  /**
   * \brief Returns all view categories, each of which has a tool bar, in tool bar order.
   */
  static QStringList GetCategories();

  /**
   * \brief Returns whether the tool bar of category \p lhs comes before the one of \p rhs.
   *
   * The categories of the views shipped with MITK come first, roughly by how
   * commonly they are used, while utilities and help come last. Other
   * categories are sorted by name and placed in front of utilities and help.
   */
  static bool IsCategoryBefore(const QString& lhs, const QString& rhs);

  static QmitkCategoryToolBar::CategoryLabel GetCategoryLabel();

  /**
   * \brief Stores how category tool bars show their category name.
   *
   * Call ApplyToWorkbench() to apply it to the open workbench windows.
   */
  static void SetCategoryLabel(QmitkCategoryToolBar::CategoryLabel categoryLabel);

  static QStringList GetVisibleCategories();

  /**
   * \brief Shows the tool bars of the given categories and hides all others.
   *
   * The visibility is stored in the preferences and applied to the tool bars
   * of all open workbench windows.
   */
  static void SetVisibleCategories(const QStringList& categories);

  /**
   * \brief Applies the stored visibility and category label to all open workbench windows.
   */
  static void ApplyToWorkbench();
};

#endif
