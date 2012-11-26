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


#ifndef MINIMALVIEW_H_
#define MINIMALVIEW_H_

#include <berryQtViewPart.h>

#include "ChangeTextRegistry.h"

#include "ui_MinimalViewControls.h"

#include <QSignalMapper>


class MinimalView : public berry::QtViewPart
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  MinimalView();

protected:

  void CreateQtPartControl(QWidget *parent);

  void SetFocus();

private Q_SLOTS:

  void ChangeText(const QString& id);

private:

  Ui::MinimalViewControls m_Controls;

  QWidget* m_Parent;
  QSignalMapper m_SignalMapper;

  ChangeTextRegistry m_Registry;
};

#endif /*MINIMALVIEW_H_*/
