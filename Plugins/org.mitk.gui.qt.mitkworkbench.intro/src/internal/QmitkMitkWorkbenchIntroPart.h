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

#include <QStringList>

class QmitkHtmlWidget;
class QWidget;

/**
 * \ingroup org_mitk_gui_qt_welcomescreen_internal
 * \brief Welcome screen shown in the editor area of the MITK Workbench.
 *
 * Introduces the Workbench to first-time users with a short description and a
 * set of usage tips. The header and the tip frame are native Qt widgets; the
 * cycling tip content is HTML rendered by a QmitkHtmlWidget. The "show tips"
 * preference decides whether the tips box is shown, and a random tip is picked
 * whenever the page is (re)loaded.
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
  void ReloadPage();

private:
  void ApplyTipsPreference();
  void ShowTip(int index);

  QWidget* m_TipsBox;
  QmitkHtmlWidget* m_TipView;
  QStringList m_TipFiles;
  int m_CurrentTip;
};

#endif
