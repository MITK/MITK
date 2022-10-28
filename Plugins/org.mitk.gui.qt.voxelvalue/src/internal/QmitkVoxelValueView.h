/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVoxelValueView_h
#define QmitkVoxelValueView_h

#include <QmitkAbstractView.h>
#include <QmitkSliceNavigationListener.h>
#include <mitkIRenderWindowPartListener.h>

#include <QLineEdit.h>
#include <QLabel.h>
#include <QHBoxLayout>

class QmitkVoxelValueView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{

  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkVoxelValueView(QObject *parent = nullptr);

  ~QmitkVoxelValueView() override;

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override { };

protected Q_SLOTS:
  void OnSliceChanged();

protected:
  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

private:
  QHBoxLayout* m_Layout;
  QLabel* m_Label;
  QLineEdit* m_TextWidget;
  QmitkSliceNavigationListener m_SliceNavigationListener;
};

#endif // QmitkVoxelValueView_h
