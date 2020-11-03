/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef ViewNavigatorView_h
#define ViewNavigatorView_h

#include <QmitkAbstractView.h>

class QmitkViewNavigatorWidget;

/**
  \brief ViewNavigatorView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class ViewNavigatorView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

private:

  QmitkViewNavigatorWidget* m_ViewNavigatorWidget;
};

#endif // ViewNavigatorView_h
