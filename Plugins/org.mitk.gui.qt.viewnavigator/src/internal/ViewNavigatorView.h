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

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

private:

  QmitkViewNavigatorWidget* m_ViewNavigatorWidget;
};

#endif // ViewNavigatorView_h
