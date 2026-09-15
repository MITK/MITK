/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeLightingWidget_h
#define QmitkVolumeLightingWidget_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkDataNode.h>
#include <mitkWeakPointer.h>

#include <QWidget>

#include <memory>

namespace Ui
{
  class QmitkVolumeLightingWidget;
}

/**
 * \brief Controls for the shading and lighting of one volume-rendered node:
 *        a lighting model and the four Phong values.
 *
 * Shading is not among them. It is asserted on whenever the widget writes the
 * material, because everything the widget does offer is inert while it is off.
 *
 * Reads and writes the node's properties through mitk::VolumeRenderingMaterial
 * and mitk::VolumeRenderingLightingModel, so the widget carries no knowledge of
 * property keys or of what a given model is worth.
 *
 * What it deliberately does not do is install the light rig a model needs. Rigs
 * belong to the renderer, and reaching one needs a render window that only a
 * plugin can supply, so the widget reports that the node changed and leaves the
 * rig to the host - see LightingChanged.
 *
 * The host also decides whether these controls apply at all. Every property here
 * reaches the ray caster through the compositing loop that only the composite
 * blend mode runs, and the blend mode is not this widget's to know, so gating is
 * plain QWidget::setEnabled from outside.
 *
 * \sa mitk::VolumeRenderingLightingModel, mitk::VolumeRenderingMaterial
 */
class MITKVOLUMEVISUALIZATIONUI_EXPORT QmitkVolumeLightingWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkVolumeLightingWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkVolumeLightingWidget() override;

  /**
   * \brief Show and edit the lighting properties of this node.
   * \param[in] node The node to bind to; nullptr leaves the controls at the
   *            defaults and makes every edit a no-op.
   */
  void SetDataNode(mitk::DataNode *node);

public slots:
  /**
   * \brief Re-read the bound node.
   *
   * Needed when something other than this widget changed the node's lighting
   * properties, which the widget has no way to notice.
   */
  void UpdateControls();

signals:
  /**
   * \brief Emitted after the widget wrote lighting properties to the node.
   *
   * The write has already happened, so a host can respond by re-deriving
   * whatever it keeps outside the node - the light rig, a render request -
   * straight from the node.
   */
  void LightingChanged();

private slots:
  void OnMaterialChanged();
  void OnModelChanged(int index);
  void OnReset();

private:
  std::unique_ptr<Ui::QmitkVolumeLightingWidget> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_DataNode;
};

#endif
