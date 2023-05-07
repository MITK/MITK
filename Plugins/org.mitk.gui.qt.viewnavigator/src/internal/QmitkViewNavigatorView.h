/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewNavigatorView_h
#define QmitkViewNavigatorView_h

#include <QmitkAbstractView.h>

class QmitkViewNavigatorWidget;

/**
* @brief
*
*/
class QmitkViewNavigatorView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

private:

  QmitkViewNavigatorWidget* m_ViewNavigatorWidget;

};

#endif
