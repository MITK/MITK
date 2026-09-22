/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMitkWorkbenchIntroPart_h
#define QmitkMitkWorkbenchIntroPart_h

#include <berryQtIntroPart.h>

class QLabel;
class QWidget;
class QmitkWelcomeRecentDataPage;
class QmitkWelcomeTipsPage;

/**
 * \ingroup org_mitk_gui_qt_welcomescreen_internal
 * \brief Welcome screen shown in the editor area of the MITK Workbench.
 *
 * Introduces the Workbench with a short description above three tabs: usage
 * tips, the most common settings, and recently opened data. The recent data is
 * shown first if there is any, otherwise the settings, so that new users see
 * them first. The screen follows the application theme at runtime.
 */
class QmitkMitkWorkbenchIntroPart : public berry::QtIntroPart
{

// this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  QmitkMitkWorkbenchIntroPart();
 ~QmitkMitkWorkbenchIntroPart() override;

  void CreateQtPartControl(QWidget *parent) override;
  void StandbyStateChanged(bool) override;
  void SetFocus() override;

private:
  void ApplyTheme();

  QWidget* m_Content;
  QLabel* m_Links;
  QmitkWelcomeTipsPage* m_TipsPage;
  QmitkWelcomeRecentDataPage* m_RecentDataPage;
};

#endif
