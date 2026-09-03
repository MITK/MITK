/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkVolumeTransferFunctionEditor_h
#define QmitkVolumeTransferFunctionEditor_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkDataNode.h>
#include <mitkSimpleHistogram.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionPresets.h>
#include <mitkWeakPointer.h>

#include <vtkSmartPointer.h>

#include <QWidget>

#include <array>
#include <memory>
#include <string>

class QmitkVolumeThumbnailRenderer;
class vtkColorTransferFunction;

namespace Ui
{
  class QmitkVolumeTransferFunctionEditor;
}

/**
 * \brief The transfer function of one volume-rendered node, in two modes: a
 *        catalogued preset adjusted by colour and opacity windows, or a curve
 *        authored point by point.
 *
 * The two modes replace each other in place, so a host sees one widget rather
 * than a mode it has to manage. Authoring ends either by keeping the curve it
 * produced or by cancelling back to the one that stood when authoring began.
 *
 * What the widget records on the node is a recipe rather than only a result -
 * the preset it started from plus the four window offsets - so that returning
 * to a node restores the controls as they were left. See the
 * volumerendering.transferfunction.* entries in the property documentation.
 *
 * \sa mitk::TransferFunctionPresets, QmitkCombinedTransferFunctionCanvas
 */
class MITKVOLUMEVISUALIZATIONUI_EXPORT QmitkVolumeTransferFunctionEditor : public QWidget
{
  Q_OBJECT

public:
  QmitkVolumeTransferFunctionEditor(QWidget *parent = nullptr, Qt::WindowFlags f = {});
  ~QmitkVolumeTransferFunctionEditor() override;

  /**
   * \brief Bind to a node and take over whatever transfer function it carries.
   *
   * Call this when the selection changes, and only then. It is not a refresh:
   * it decides whether the node's existing function is one to adopt, and the
   * answer depends on state a caller can change between selections. Re-binding
   * the same node just after switching volume rendering on would adopt the
   * default function mitk::VolumeMapperVtkSmart3D registers on every image -
   * a curve nobody chose - instead of leaving room for
   * EnsureTransferFunction to apply a preset.
   *
   * \param[in] node The node to edit; nullptr clears the editor.
   */
  void SetDataNode(mitk::DataNode *node);

  /**
   * \brief Apply the first catalogued preset unless a function is already held.
   *
   * For the moment volume rendering is switched on: the node has a transfer
   * function by then, but it is the mapper's registered default, which no
   * preset names and which the catalogue is meant to supersede.
   */
  void EnsureTransferFunction();

protected:
  /** \brief Re-measure the preset grid when the room it has to fill changes. */
  bool eventFilter(QObject *watched, QEvent *event) override;

signals:
  /**
   * \brief Emitted after the widget changed what the node renders as.
   *
   * The node is already updated, so a host has only to re-render - and to
   * refresh its own controls, because loading or authoring a function switches
   * volume rendering on so that the result is visible at once.
   */
  void TransferFunctionChanged();

  /**
   * \brief Emitted when authoring a curve by hand starts or ends.
   *
   * Authoring wants the room, so a host laying other sections out around this
   * widget may want to fold them away for the duration.
   */
  void CustomModeChanged(bool active);

private slots:
  void OnPresetSelected(const QString &presetName);
  void OnColorWindowChanged();
  void OnCanvasOpacityChanged();
  void OnResetAdjustments();
  void OnCreateCustom();
  void OnImportCustom();
  void OnCancelCustom();
  void OnDoneCustom();
  void OnSaveCustom();

private:
  /** \brief Write the held function onto the node and re-seed the editor. */
  void ApplyCurrentTransferFunction();

  /** \brief Take over the function the bound node already carries. */
  void AdoptTransferFunctionFromNode();

  /** \brief Point the canvas and the adjust sliders at the held function, or
   *         clear them when none is held.
   */
  void ShowAppliedTransferFunction();

  /** \brief Rebuild the function from the preset and offsets the node records.
   *  \return True if a complete recipe was found and re-executed.
   */
  bool ReplayAdjustOffsets(const std::string &presetName);

  /** \brief Record the adjust sliders' current offsets on the node. */
  void RecordAdjustOffsets();

  /** \brief Drop the recorded preset and offsets, for a function the widget
   *         cannot reproduce.
   */
  void ForgetTransferFunctionRecipe(mitk::DataNode *node);

  /** \brief Deselect any preset, and say in the combo's placeholder whether that
   *         is because a curve no preset describes is held, or because none is.
   */
  void ClearPresetSelection();

  void SnapshotAppliedTransferFunction();

  /** \brief The colour window width that reproduces the baseline unchanged. */
  double NeutralColorWidth() const;

  void ResetAdjustSliders();

  /** \brief Swap the preset controls for the per-point editor, or back. */
  void SetCustomModeActive(bool active);

  /**
   * \brief Size the preset cells to the width the panel currently gives them.
   *
   * The panel is a fraction of the workbench window rather than a fixed width,
   * so the cells are measured from it instead of fixed, and measured again
   * whenever it changes. What stays fixed is how many previews stand side by
   * side, which is the point of the grid.
   */
  void UpdatePresetGrid();

  /**
   * \brief Begin drawing a preview for every preset, unless they are current.
   *
   * Bound to the preset combo opening rather than to the selection changing,
   * so that clicking through a list of images does not draw previews nobody
   * asked to see.
   */
  void StartThumbnailGeneration();

  /**
   * \brief Draw one preset's preview and queue the next.
   *
   * \param[in] run The generation this call belongs to; it abandons itself if
   *                the previews have been invalidated since it was queued.
   */
  void GenerateNextThumbnail(int run);

  /** \brief Drop every preview, and abandon a generation in progress. */
  void InvalidateThumbnails();

  std::unique_ptr<Ui::QmitkVolumeTransferFunctionEditor> m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_DataNode;

  std::unique_ptr<QmitkVolumeThumbnailRenderer> m_ThumbnailRenderer;

  /** \brief The image the previews on show were drawn from. */
  mitk::WeakPointer<mitk::Image> m_ThumbnailImage;

  /** \brief How far the generation in progress has got.
   *
   * The loop counter, kept here rather than on the stack because the loop
   * returns to the event loop between iterations.
   */
  int m_NextThumbnailIndex = 0;

  /** \brief A version number for the previews on show.
   *
   * Drawing them is spread across many turns of the event loop, and anything
   * can happen in between. Every invalidation bumps this; each queued step
   * remembers the number it was queued under and does nothing if the two no
   * longer agree, which is how a step left over from a previous image, or
   * from a run that was cleared part-way, stops on its own.
   */
  int m_ThumbnailRun = 0;

  mitk::TransferFunctionPresets m_Presets;
  mitk::SimpleHistogramCache m_HistogramCache;

  mitk::TransferFunction::Pointer m_AppliedTransferFunction;
  vtkSmartPointer<vtkColorTransferFunction> m_BaseColorFn;
  mitk::TransferFunction::Pointer m_PreEditTransferFunction;
  std::array<double, 2> m_DataRange { 0.0, 0.0 };
};

#endif
