/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MINIMALVIEW_H_
#define MINIMALVIEW_H_

#include <berryQtViewPart.h>

#include "ChangeTextRegistry.h"

#include <QSignalMapper>
#include <memory>

namespace Ui
{
  class MinimalViewControls;
}

class MinimalView : public berry::QtViewPart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  MinimalView();
  ~MinimalView() override;

protected:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

private Q_SLOTS:

  void ChangeText(const QString &id);

private:
  std::unique_ptr<Ui::MinimalViewControls> m_Controls;

  QWidget *m_Parent;
  QSignalMapper m_SignalMapper;

  ChangeTextRegistry m_Registry;
};

#endif /*MINIMALVIEW_H_*/
