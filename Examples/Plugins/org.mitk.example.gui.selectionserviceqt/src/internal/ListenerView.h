/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef LISTENERVIEW_H_
#define LISTENERVIEW_H_

// berry includes
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>
#include <berryQtViewPart.h>

// ui includes
#include "ui_ListenerViewControls.h"

/**
 * \ingroup org_mitk_example_gui_selectionserviceqt
 *
 * \brief This BlueBerry view is part of the BlueBerry example
 * "Selection service QT". It creates two radio buttons that listen
 * for selection events of the Qlistwidget (SelectionProvider) and
 * change the radio button state accordingly.
 *
 * @see SelectionView
 */
class ListenerView : public berry::QtViewPart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  ListenerView();

  ~ListenerView() override;

protected:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

private Q_SLOTS:

  /**
   * @brief Simple slot function that changes the selection of the radio buttons according to the passed string.
   * @param selectStr QString that contains the name of the slected list element
   */
  void ToggleRadioMethod(QString selectStr);

private:
  //! [Qt Selection Listener method and pointer]
  /**
   * @brief Method of berry::ISelectionListener that implements the selection listener functionality.
   * @param sourcepart The workbench part responsible for the selection change.
   * @param selection This parameter holds the current selection.
   *
   * @see ISelectionListener
   */
  void SelectionChanged(const berry::IWorkbenchPart::Pointer &sourcepart,
                        const berry::ISelection::ConstPointer &selection);

  /** @brief this pointer holds the selection listener */
  QScopedPointer<berry::ISelectionListener> m_SelectionListener;
  //! [Qt Selection Listener method and pointer]

  friend struct berry::SelectionChangedAdapter<ListenerView>;

  Ui::ListenerViewControls m_Controls;

  QWidget *m_Parent;
};

#endif /*LISTENERVIEW_H_*/
