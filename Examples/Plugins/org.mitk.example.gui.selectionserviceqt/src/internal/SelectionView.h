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


#ifndef SELECTIONVIEW_H_
#define SELECTIONVIEW_H_

// berry includes
#include <berryQtViewPart.h>
#include <berryQtSelectionProvider.h>

// ui includes
#include "ui_SelectionViewControls.h"

/**
 * \ingroup org_mitk_example_gui_selectionserviceqt
 *
 * \brief This BlueBerry view is part of the BlueBerry example
 * "Selection service QT". It creates a QListWidget that provides
 * the selection events for the selection listener. The selection
 * provider is Qt-based.
 */
class SelectionView : public berry::QtViewPart
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  //! [Qt Selection Provider]
  /** @brief this pointer holds the selection provider*/
  berry::QtSelectionProvider::Pointer m_SelectionProvider;
  //! [Qt Selection Provider]

  SelectionView();

protected:

  void CreateQtPartControl(QWidget *parent);

  void SetFocus();

private:

  Ui::SelectionViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*SELECTIONVIEW_H_*/
