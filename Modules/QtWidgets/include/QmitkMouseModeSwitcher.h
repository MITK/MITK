/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkMouseModeSwitcher_h
#define QmitkMouseModeSwitcher_h

#include "MitkQtWidgetsExports.h"

#include "mitkMouseModeSwitcher.h"

#include <QActionGroup>
#include <QToolBar>

/**
 * \ingroup QmitkModule
 * \brief Qt toolbar representing mitk::MouseModeSwitcher.
 *
 * Provides buttons for the interaction modes defined in mitk::MouseModeSwitcher
 * and communicates with this non-graphical class.
 *
 * Can be used in a GUI to provide a mouse mode selector to the user.
 */
class MITKQTWIDGETS_EXPORT QmitkMouseModeSwitcher : public QToolBar
{
  Q_OBJECT

public:

  QmitkMouseModeSwitcher(QWidget* parent = nullptr);
  ~QmitkMouseModeSwitcher() override;

  typedef mitk::MouseModeSwitcher::MouseMode MouseMode;

public slots:

  /**
    \brief Connect to non-GUI class.

    When a button is pressed, given mitk::MouseModeSwitcher is informed to adapt interactors.

    \todo QmitkMouseModeSwitcher could be enhanced to actively observe mitk::MouseModeSwitcher and change available
    actions or visibility appropriately.
  */
  void setMouseModeSwitcher(mitk::MouseModeSwitcher*);

signals:

  /**
    \brief Mode activated.

    This signal is needed for other GUI element to react appropriately.
    Sadly this is needed to provide "normal" functionality of QmitkStdMultiWidget,
    because this must enable/disable automatic reaction of SliceNavigationControllers
    to mouse clicks - depending on which mode is active.
  */
  void MouseModeSelected(MouseMode id);

protected slots:

  void OnMouseModeChangedViaButton();
  void addButton(MouseMode id, const QString& toolName, const QIcon& icon, bool on = false);

protected:

  void OnMouseModeChangedViaCommand(const itk::EventObject&);

  QActionGroup* m_ActionGroup;
  mitk::MouseModeSwitcher* m_MouseModeSwitcher;

  unsigned long m_ObserverTag;

  bool m_ActionButtonBlocked;
};

#endif
