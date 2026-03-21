/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRegEvalSettingsWidget_h
#define QmitkRegEvalSettingsWidget_h

#include <MitkMatchPointRegistrationUIExports.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <memory>

namespace Ui
{
  class QmitkRegEvalSettingsWidget;
}

/**
 * \class QmitkRegEvalSettingsWidget
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegEvalSettingsWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkRegEvalSettingsWidget(QWidget *parent = nullptr);
  ~QmitkRegEvalSettingsWidget() override;

  /**
   * Configures the passed settings according to the current state of the
   * widget.
   * \pre settings must point to a valid instance..
   */
  void ConfigureControls();

public Q_SLOTS:
  /**
    * \brief Slot that can be used to set the node that should be configured by the widget.*/
  void SetNode(mitk::DataNode *node);

signals:
  void SettingsChanged(mitk::DataNode *node);

protected Q_SLOTS:
  void OnComboStyleChanged(int);
  void OnBlend50Pushed();
  void OnBlendTargetPushed();
  void OnBlendMovingPushed();
  void OnBlendTogglePushed();
  void OnSlideBlendChanged(int);
  void OnSpinBlendChanged(int);
  void OnSpinCheckerChanged(int);
  void OnWipeStyleChanged();
  void OnContourStyleChanged();

protected:
  std::unique_ptr<Ui::QmitkRegEvalSettingsWidget> m_Controls;

private:
  mitk::DataNode::Pointer m_selectedEvalNode;

  bool m_internalBlendUpdate;
  bool m_internalUpdate;
};

#endif
