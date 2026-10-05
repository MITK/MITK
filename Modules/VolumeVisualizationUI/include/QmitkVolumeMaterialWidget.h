/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeMaterialWidget_h
#define QmitkVolumeMaterialWidget_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkDataNode.h>
#include <mitkVolumeRenderingMaterial.h>
#include <mitkWeakPointer.h>

#include <QWidget>

#include <memory>

namespace Ui
{
  class QmitkVolumeMaterialWidget;
}

/**
 * \brief Controls for the shading of one volume-rendered node: the four Phong
 *        values, and a reset back to what its lighting model dictates.
 *
 * Which lighting model the node is on is not among them. A model comes with the
 * light rig it is tuned for, rigs belong to the render window, and one window
 * lights every volume drawn in it - so that choice is made on the window itself
 * and applies to all of them at once. These values are the per-node tuning left
 * over once it is made.
 *
 * Shading is not offered either. It is asserted on whenever the widget writes
 * the material, because everything the widget does offer is inert while it is
 * off.
 *
 * Reads and writes the node's properties through mitk::VolumeRenderingMaterial
 * and mitk::VolumeRenderingLightingModel, so the widget carries no knowledge of
 * property keys or of what a given model is worth.
 *
 * The host also decides whether these controls apply at all. Every property here
 * reaches the ray caster through the compositing loop that only the composite
 * blend mode runs, and the blend mode is not this widget's to know, so gating is
 * plain QWidget::setEnabled from outside, with SetTitleSuffix to say why.
 *
 * \sa mitk::VolumeRenderingLightingModel, mitk::VolumeRenderingMaterial
 */
class MITKVOLUMEVISUALIZATIONUI_EXPORT QmitkVolumeMaterialWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkVolumeMaterialWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkVolumeMaterialWidget() override;

  /**
   * \brief Show and edit the lighting properties of this node.
   * \param[in] node The node to bind to; nullptr leaves the controls at the
   *            defaults and makes every edit a no-op.
   */
  void SetDataNode(mitk::DataNode *node);

  /**
   * \brief Append a note to the title of the controls.
   * \param[in] suffix Shown after the title, separated from it; empty shows the
   *            plain title.
   */
  void SetTitleSuffix(const QString &suffix);

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
  void OnReset();

private:
  /** Write one material value to the bound node, leaving the others as the node has them. */
  void SetMaterialValue(float mitk::VolumeRenderingMaterial::*field, float value);

  std::unique_ptr<Ui::QmitkVolumeMaterialWidget> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_DataNode;
};

#endif
