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
 * to a node restores the controls as they were left. Where no recipe can
 * describe the curve, because it was authored here or loaded from a file, what
 * is recorded instead is that the curve was chosen here at all. See the
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
   * \brief Apply the preset that suits the image, unless a function is already
   *        held.
   *
   * For the moment volume rendering is switched on: the node has a transfer
   * function by then, but it is the mapper's registered default, which no
   * preset names and which the catalogue is meant to supersede. Which preset
   * that is follows from the image's DICOM metadata where it has any - see
   * mitk::TransferFunctionPresets::GetDefaultPresetName.
   */
  void EnsureTransferFunction();

protected:
  /** \brief Re-measure the preset entries when the room they have to fill
   *         changes.
   */
  bool eventFilter(QObject *watched, QEvent *event) override;

  /** \brief Draw the preset previews once the editor is live for its node. */
  void changeEvent(QEvent *event) override;

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
   * Authoring takes the panel, not merely this widget: it wants the room, and a
   * change confined to one section in the middle of a panel reads as a section
   * that changed shape rather than as arriving somewhere. A host laying other
   * sections out around this widget is expected to fold them away for the
   * duration, and to put the panel back at its top either way.
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
  void OnBlendModeChanged(int index);

private:
  /** \brief Write the held function onto the node and re-seed the editor. */
  void ApplyCurrentTransferFunction();

  /** \brief Record the blend mode on the node, and show it on the control.
   *
   * Both, because either one alone leaves the two disagreeing: a preset brings
   * a mode the control has to catch up with, and the control brings one the
   * node has to.
   */
  void ApplyBlendMode(mitk::VolumeBlendMode blendMode);

  /** \brief Point the blend mode control at what the bound node records. */
  void ShowNodeBlendMode();

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

  /** \brief Drop everything this widget records about how the node's curve was
   *         arrived at, for a function it cannot reproduce.
   */
  void ForgetTransferFunctionRecipe(mitk::DataNode *node);

  /** \brief Record that the node's curve was authored here or loaded from a
   *         file.
   *
   * Dropping the recipe and recording the fact are one act rather than two:
   * what makes such a curve custom is precisely that no recipe describes it.
   */
  void RecordCustomTransferFunction(mitk::DataNode *node);

  /** \brief Deselect any preset, leaving the section header to say why.
   *
   * Whether that is because a curve no preset describes is held, or because
   * none is, is named on the header rather than in the grid.
   */
  void ClearPresetSelection();

  void SnapshotAppliedTransferFunction();

  /** \brief The colour window width that reproduces the baseline unchanged. */
  double NeutralColorWidth() const;

  void ResetAdjustSliders();

  /**
   * \brief Swap the preset controls for the per-point editor, or back.
   *
   * A request for the mode already in force does nothing. SetDataNode cancels
   * authoring on every selection change, whether any was in progress or not,
   * and relies on that call being inert.
   */
  void SetCustomModeActive(bool active);

  /**
   * \brief Lay the presets out as a grid of previews, or as a list of names
   *        each beside a small one.
   *
   * Both presentations draw the same previews from the same pixmaps; what
   * differs is how much of one an entry gets, so nothing is rendered again on
   * a switch.
   */
  void SetCompactPresetList(bool compact);

  /**
   * \brief Size the preset cells to the width the panel currently gives them.
   *
   * The panel is a fraction of the workbench window rather than a fixed width,
   * so the cells are measured from it instead of fixed, and measured again
   * whenever it changes. What stays fixed is how many previews stand side by
   * side, which is the point of the grid; a list has one entry per row and
   * hands it the whole width.
   */
  void UpdatePresetLayout();

  /**
   * \brief Ask for a preview of every preset, unless they are current.
   *
   * A request rather than an order: whether previews are worth drawing at all
   * is settled one turn of the event loop later, in GenerateNextThumbnail.
   * Binding a node and greying the editor out for it are two steps of the same
   * selection change, and the second has not run yet when the first asks.
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

  /**
   * \brief Give every entry still waiting for a preview a stand-in built for
   *        the cell size the grid currently uses.
   *
   * A preview is drawn once at a width no cell exceeds and scaled down from
   * there, so it survives a re-measure. A stand-in is built at the exact cell
   * size instead, and a QIcon holding a single pixmap is never scaled up, so a
   * grid that has grown since would draw it small and centred.
   */
  void RefreshPlaceholders();

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

  /** \brief The mode that stood when authoring began.
   *
   * Meaningful only alongside m_PreEditTransferFunction, and restored with it:
   * cancelling back to the previous curve while keeping a mode picked during
   * the abandoned edit would render that curve in a mode nobody chose for it.
   */
  mitk::VolumeBlendMode m_PreEditBlendMode { mitk::VolumeBlendMode::Composite };

  /** \brief Whether the per-point editor is the mode currently in force.
   *
   * Kept rather than read back from the panels it swaps: a view sitting behind
   * another tab has its part control hidden, and every widget in it then
   * reports isVisible() false whatever mode the editor is in.
   */
  bool m_CustomModeActive = false;

  /** \brief Whether the presets are listed as names beside small previews
   *         rather than laid out as a grid of large ones.
   *
   * The only record of the choice. The button that makes it is not checkable,
   * since its icon names the presentation pressing it brings rather than the
   * one in force, and a second copy of the state is a second thing to keep in
   * step with this one.
   */
  bool m_CompactPresetList = false;

  std::array<double, 2> m_DataRange { 0.0, 0.0 };
};

#endif
