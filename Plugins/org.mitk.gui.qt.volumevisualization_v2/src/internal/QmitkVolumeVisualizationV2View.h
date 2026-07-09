/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeVisualizationV2View_h
#define QmitkVolumeVisualizationV2View_h

#include <QmitkAbstractView.h>

#include <memory>

namespace Ui
{
  class QmitkVolumeVisualizationV2ViewControls;
}

class QmitkVolumeVisualizationV2View : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkVolumeVisualizationV2View();
  ~QmitkVolumeVisualizationV2View() override;

  void SetFocus() override;

private:
  void CreateQtPartControl(QWidget *parent) override;

  std::unique_ptr<Ui::QmitkVolumeVisualizationV2ViewControls> m_Controls;
};

#endif