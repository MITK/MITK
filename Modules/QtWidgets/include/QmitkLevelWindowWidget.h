/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowWidget_h
#define QmitkLevelWindowWidget_h

#include <MitkQtWidgetsExports.h>
#include <mitkLevelWindowManager.h>

#include <QWidget>

// Forward declarations
namespace Ui
{
  class QmitkLevelWindow;
}

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkLevelWindowWidget : public QWidget
{
  Q_OBJECT
public:
  QmitkLevelWindowWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkLevelWindowWidget() override;

  mitk::LevelWindowManager *GetManager();

public slots:
  void SetDataStorage(mitk::DataStorage *ds);

protected:
  // unsigned long m_ObserverTag;
  mitk::LevelWindowManager::Pointer m_Manager;

private:
  // GUI controls of this plugin
  Ui::QmitkLevelWindow* ui;

};
#endif
