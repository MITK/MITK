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
 * \brief The transfer function of one volume-rendered node: a catalogued preset
 *        adjusted by colour and opacity windows, or a curve edited point by
 *        point on the canvas that shows it.
 *
 * Editing happens on that canvas rather than on a page of its own, so a host
 * sees one widget throughout and has no mode to lay out around. What is drawn
 * is kept; the curve that stood before it is not held on to.
 *
 * What the widget records on the node is a recipe rather than only a result -
 * the preset it started from plus the four window offsets - so that returning
 * to a node restores the controls as they were left. The recipe is also all that
 * outlives the selection: a curve drawn over a preset point by point is recorded
 * nowhere, so coming back to the node rebuilds the preset and the offsets and
 * the drawing is gone. The panel marks such a curve as edited while it is on
 * show, and keeping one means saving it as a preset of its own. A curve loaded
 * from a file answers to no catalogue entry at all, and what is recorded for it
 * is only that it was chosen here. See the volumerendering.transferfunction.*
 * entries in the property documentation.
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
   * refresh its own controls, because loading a function from a file switches
   * volume rendering on so that the result is visible at once.
   */
  void TransferFunctionChanged();

private slots:
  void OnPresetSelected(const QString &presetName);
  void OnColorWindowChanged();
  void OnCanvasOpacityChanged();
  void OnResetAdjustments();
  void OnImportCustom();
  void OnPresetContextMenu(const QPoint &pos);
  void OnAddColorStop();
  void OnPickColorStopColor();

private:
  /**
   * \brief Take the presets saved from here in earlier sessions into the
   *        catalogue, and forget the files that are no longer there.
   *
   * Runs before the grid is filled, so that they reach it as any other entry
   * does - at its end, which is where the catalogue keeps what is added to it.
   */
  void LoadRememberedPresets();

  /**
   * \brief Write the curve on show to a file of the user's choosing, and offer
   *        it from then on as a preset of its own.
   *
   * The file is what the preset is: it is read straight back in, remembered, and
   * looked for again at every later start. Nothing is written for a curve that
   * is still the preset it came from - see DiffersFromPreset.
   */
  void SaveCustomPreset();

  /** \brief Write the held function onto the node and re-seed the editor. */
  void ApplyCurrentTransferFunction();

  /** \brief Record on the node the blend mode a preset or a loaded file brings.
   *
   * The mode travels with the curve rather than being chosen on its own, so
   * whatever supplies the curve supplies this too.
   */
  void ApplyBlendMode(mitk::VolumeBlendMode blendMode);

  /** \brief Take over the function the bound node already carries. */
  void AdoptTransferFunctionFromNode();

  /** \brief Point the canvas and the adjust sliders at the held function, or
   *         clear them when none is held.
   */
  void ShowAppliedTransferFunction();

  /**
   * \brief Enable the controls that apply to the node, the function and the
   *        mode in force.
   *
   * Apart from ShowAppliedTransferFunction, which re-seeds the canvas and both
   * baselines: entering an edit changes what may be done to the curve without
   * changing the curve, and re-seeding for that would measure the sliders from
   * an edit not yet made.
   */
  void UpdateControlAvailability();

  /**
   * \brief Rebuild the function from the preset the node names, and from the
   *        four offsets where it records them too.
   *
   * What the node stores as its curve is not consulted: a curve drawn over a
   * preset point by point is no part of the recipe, and rebuilding is what
   * discards it.
   *
   * \return True unless there is no node to rebuild for.
   */
  bool ReplayRecipe(const QString &presetName);

  /** \brief Record the adjust sliders' current offsets on the node. */
  void RecordAdjustOffsets();

  /**
   * \brief Drop the four offsets, leaving the rest of what the node records
   *        about how its curve was arrived at.
   *
   * What lets a curve drawn over by hand keep its preset name. The offsets
   * describe the preset that was drawn over rather than the drawing, and
   * nothing records the drawing but the curve itself, so replaying them on the
   * next selection would put the catalogue's curve back over it.
   */
  void ForgetAdjustOffsets(mitk::DataNode *node);

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

  /** \brief Leave the grid with no entry marked.
   *
   * Either a curve that no preset describes is held, or none is held at all.
   * Nothing on the panel tells the two apart.
   */
  void ClearPresetSelection();

  void SnapshotAppliedTransferFunction();

  /** \brief The colour window width that reproduces the baseline unchanged. */
  double NeutralColorWidth() const;

  void ResetAdjustSliders();

  /**
   * \brief Hand the canvas over to point-by-point editing, or take it back.
   *
   * Leaving keeps whatever was drawn - there is no way back to the curve that
   * stood before it - so leaving is also where a curve that was really edited
   * is recorded as answering to no preset.
   *
   * A request for the mode already in force does nothing. SetDataNode ends
   * editing on every selection change, whether any was in progress or not, and
   * relies on that call being inert.
   */
  void SetEditModeActive(bool active);

  /**
   * \brief Give the canvas its handles, or take them away, and show which of the
   *        two the panel is in.
   *
   * The one place that turns "editing" into what the canvas and the panel do
   * about it, so that entering and leaving arrive at their states by the same
   * route.
   */
  void ShowEditMode();

  /**
   * \brief Scale the canvas axis, to the image's intensity band or to the whole
   *        curve.
   *
   * The band is what the 2D views show the image over, and a preset authored on
   * its modality's own scale reaches well past it, so the curve's outermost
   * points can sit off the axis. Widening brings them on, at the price of the
   * band - where the image actually is - shrinking into part of the plot.
   *
   * The band remains the range the sliders and the colour window are measured
   * against either way: this is what the canvas shows, not what the panel means
   * by the data.
   */
  void ApplyAxisRange();

  /**
   * \brief Point the colour stop controls at what the canvas currently holds and
   *        has selected.
   *
   * Driven by the canvas rather than kept alongside it: a stop can be added,
   * moved, recoloured or selected on the canvas just as well as here, and one
   * copy of that state is one thing to keep right.
   */
  void ShowColorStops();

  /** \brief Mark the preset in force as edited, or take the mark away. */
  void ShowPresetEdited();

  /**
   * \brief Whether the curve on show is no longer what the preset in force
   *        would produce.
   *
   * Answered from the sliders and from what the node records, rather than kept
   * as a flag, so that a node coming back into the panel is described by what
   * it carries rather than by what was last done here.
   */
  bool DiffersFromPreset() const;

  /**
   * \brief The colours on show, carried by points that can be taken hold of.
   *
   * A colour window bakes itself into hundreds of evenly spaced samples, which
   * name no colour in particular: there is one per pixel column and no way to
   * tell which of them was meant. The same window laid over the baseline's own
   * nodes instead carries the same colours at the points the curve actually
   * names them, which is the form to edit in and the form to save in.
   *
   * The window is measured from the baseline's own range, so a baseline must be
   * held and must be one nothing has windowed yet.
   */
  vtkSmartPointer<vtkColorTransferFunction> WindowedColorHandles() const;

  /**
   * \brief Give the colour function back the handful of points it can be taken
   *        hold of by, without changing the colours it shows.
   *
   * Once per edit: the window bakes itself into hundreds of evenly spaced
   * points, and doing this a second time would copy over the colours just
   * edited. Does nothing where there is nothing to restore.
   */
  void RestoreColorHandles();

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

  /** \brief Whether the canvas is currently editable.
   *
   * Kept rather than read back from the button that sets it, which is also set
   * from here, and which reports nothing at all while the panel it sits on is
   * hidden behind another tab.
   */
  bool m_EditModeActive = false;

  /** \brief Whether any point was added, moved, removed or recoloured since
   *         editing began.
   *
   * What separates leaving an edit from never having made one: an untouched
   * curve is still the preset it came from, and dropping the recipe for it
   * would cost the sliders their baseline for nothing.
   */
  bool m_CurveEdited = false;

  /** \brief Whether the curve on show was drawn over the preset it names.
   *
   * What m_CurveEdited says while an edit is under way, kept on after it is
   * over: leaving makes the drawing the baseline, and that clears the other
   * flag. Held here rather than on the node because a drawing is recorded
   * nowhere - the node goes on describing the preset and the offsets, and
   * rebuilding those is what the next selection does - so this lasts exactly as
   * long as the drawing does, which is as long as this node is on show. It is
   * also what keeps the entry marked, and so the curve saveable, in between.
   */
  bool m_CurveDrawnOver = false;

  /** \brief Whether the colour function has been reduced to countable handles
   *         for the edit in progress.
   *
   * A colour window bakes itself into hundreds of evenly spaced points, which
   * no one can edit by hand, so editing colours starts by putting the same
   * colours back on the baseline's own nodes. Once per edit: a second reduction
   * would copy over the colours just edited.
   */
  bool m_ColorHandlesRestored = false;

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
