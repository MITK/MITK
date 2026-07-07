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
 * \brief Widget for configuring the visual evaluation settings of a registration result.
 *
 * This widget allows the user to configure how a registration evaluation object
 * (mitk::RegEvaluationObject) is visualized. It supports several evaluation styles
 * defined by mitk::RegEvalStyleProperty, including:
 * - Blend: adjustable opacity blending between target and moving images
 * - Checkerboard: configurable grid count
 * - Wipe: cross, horizontal, or vertical wipe styles
 * - Contour: target or moving contour overlay
 *
 * The widget reads and writes properties directly on the configured mitk::DataNode,
 * and emits SettingsChanged whenever a property is modified by the user.
 *
 * \sa mitk::RegEvaluationObject, mitk::RegEvalStyleProperty, mitk::RegEvalWipeStyleProperty
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegEvalSettingsWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the registration evaluation settings widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkRegEvalSettingsWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkRegEvalSettingsWidget() override;

  /**
   * \brief Configures the UI controls according to the properties of the currently selected node.
   *
   * Reads the evaluation style, blend factor, checker count, and contour style from the
   * node's properties and updates all controls accordingly. If no node is set, the
   * style-specific control groups are hidden.
   */
  void ConfigureControls();

public Q_SLOTS:
  /**
   * \brief Sets the data node whose evaluation properties should be configured.
   *
   * If the node differs from the currently set node, the widget is reconfigured
   * via ConfigureControls().
   *
   * \param[in] node Pointer to the evaluation data node. May be \c nullptr to clear.
   */
  void SetNode(mitk::DataNode *node);

signals:
  /**
   * \brief Emitted whenever a user-initiated change modifies an evaluation property on the node.
   * \param[in] node Pointer to the data node whose properties were changed.
   */
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
