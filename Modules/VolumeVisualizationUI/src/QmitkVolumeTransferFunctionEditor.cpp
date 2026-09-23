/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumeTransferFunctionEditor.h"

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkImage.h>
#include <mitkLevelWindow.h>
#include <mitkLog.h>
#include <mitkProperties.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunctionTransform.h>

#include <QmitkCombinedTransferFunctionCanvas.h>
#include <QmitkStyleManager.h>
#include <QmitkVolumeThumbnailRenderer.h>

#include <ui_QmitkVolumeTransferFunctionEditorControls.h>

#include <ctkDoubleSlider.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRect>
#include <QSignalBlocker>
#include <QStringList>
#include <QTimer>
#include <QToolButton>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <optional>

namespace
{
  /** Keys recording how the node's transfer function was arrived at: the preset
   * it started from, and the four offsets the adjust sliders then applied.
   *
   * Together they are a recipe this widget can re-execute, which is what lets
   * the sliders come back showing where they were left. The curve alone cannot
   * serve that purpose - the colour window is baked into 256 evenly spaced
   * points, and an offset is only meaningful next to the baseline it was
   * measured from. Same reasoning as
   * mitk::VolumeRenderingLightingModel::MODEL_PROPERTY: record the choice, do
   * not infer it.
   *
   * The offsets are meaningful only alongside the preset name, and all five are
   * absent on a curve no preset describes. TF_CUSTOM_PROPERTY is what then tells
   * a curve chosen here apart from one that came from outside this widget.
   */
  constexpr const char *TF_PRESET_PROPERTY = "volumerendering.transferfunction.preset";
  constexpr const char *TF_OPACITY_SHIFT_PROPERTY = "volumerendering.transferfunction.opacityshift";
  constexpr const char *TF_OPACITY_HEIGHT_PROPERTY = "volumerendering.transferfunction.opacityheight";
  constexpr const char *TF_COLOR_SHIFT_PROPERTY = "volumerendering.transferfunction.colorshift";
  constexpr const char *TF_COLOR_WIDTH_PROPERTY = "volumerendering.transferfunction.colorwidth";

  /** Records that the node's curve was chosen here but answers to no preset: the
   * one it was built from has since been removed from the grid.
   *
   * The keys above cannot say so: what makes such a curve custom is precisely
   * that no recipe describes it. What is recorded is therefore only that the
   * curve was chosen here at all, which is what AdoptTransferFunctionFromNode
   * needs to take it back over rather than leave it to be replaced. Written true
   * or removed, never false, so absence keeps the single meaning the keys above
   * already give it.
   */
  constexpr const char *TF_CUSTOM_PROPERTY = "volumerendering.transferfunction.custom";

  /** \brief Where the files presets were saved to are remembered.
   *
   * This widget belongs to a module rather than to a view, so it has no site to
   * ask for a preference node of its own and has to name one. This is the id the
   * volume visualization v2 view declares in its plugin.xml, and it has to
   * follow that file should the view ever be renamed. mitk::VideoRecorder names
   * the movie maker view's node from a module in the same way.
   */
  constexpr const char *PRESET_PREFERENCE_NODE = "/org.mitk.views.volumevisualization_v2";

  /** \brief The key the remembered paths stand under, as one joined string.
   *
   * mitk::IPreferences stores strings, numbers and bools but no list, so a list
   * has to be one string, and joining paths with a semicolon is what the rest of
   * MITK does with one. Nothing escapes the separator, there or here, so a path
   * carrying one is refused where a preset is saved rather than quietly split in
   * two on the way back in.
   */
  constexpr const char *PRESET_FILES_PREFERENCE = "customPresetFiles";

  constexpr QLatin1Char PRESET_FILE_SEPARATOR(';');

  /** \brief The preference node the remembered preset files stand in. */
  mitk::IPreferences *PresetPreferences()
  {
    mitk::CoreServicePointer preferencesService(mitk::CoreServices::GetPreferencesService());

    auto *preferences = preferencesService->GetSystemPreferences();

    return preferences != nullptr ? preferences->Node(PRESET_PREFERENCE_NODE) : nullptr;
  }

  /** \brief The files presets were saved to, in the order they were saved. */
  QStringList RememberedPresetFiles()
  {
    auto *preferences = PresetPreferences();

    if (preferences == nullptr)
      return QStringList();

    const auto remembered = QString::fromStdString(preferences->Get(PRESET_FILES_PREFERENCE, ""));

    return remembered.split(PRESET_FILE_SEPARATOR, Qt::SkipEmptyParts);
  }

  /** \brief Record which files to look for presets in next time. */
  void RememberPresetFiles(const QStringList &presetFiles)
  {
    auto *preferences = PresetPreferences();

    if (preferences == nullptr)
      return;

    preferences->Put(PRESET_FILES_PREFERENCE,
                     presetFiles.join(PRESET_FILE_SEPARATOR).toStdString());

    // Written out here because nothing else will: the preferences dialog flushes
    // the nodes its own pages own, and this one belongs to no page.
    preferences->Flush();
  }

  /** \brief Check that a file can stand among the remembered ones, and tell the
   *         user why not where it cannot.
   *
   * Nothing escapes the separator the remembered paths are joined by, so a path
   * carrying one could not be read back as itself.
   *
   * \return True if the path passed, false once the user has been warned.
   */
  bool ValidatePresetFilePath(QWidget *parent, const QString &title, const QString &fileName)
  {
    if (!fileName.contains(PRESET_FILE_SEPARATOR))
      return true;

    QMessageBox::warning(parent, title,
      "The path must not contain a semicolon, which separates the remembered "
      "preset files from one another. Please choose a file whose path has none.");

    return false;
  }

  /** \brief Where an entry keeps the name of the preset it stands for.
   *
   * Not the text it shows: that gains a marker once the curve has been moved
   * away from the preset, and then matches no name in the catalogue.
   */
  constexpr int PRESET_NAME_ROLE = Qt::UserRole;

  /** \brief Where a preset in the grid came from.
   *
   * The catalogue holds both kinds side by side and answers to a name whichever
   * it is, so a name alone no longer says which was meant. What a node records
   * carries this beside the name, to be read in the Properties view or in a
   * scene opened where none of these files are.
   */
  enum class PresetOrigin
  {
    Internal, /**< From the catalogue embedded in MitkVolumeVisualization. */
    File      /**< From a file saved here, and remembered since. */
  };

  /** \brief Where an entry keeps the origin of the preset it stands for. */
  constexpr int PRESET_ORIGIN_ROLE = Qt::UserRole + 1;

  /** \brief Where an entry of file origin keeps the file it was read from.
   *
   * Removing such an entry means forgetting that file, and the name will not
   * say which it is: AddPreset numbers a name the catalogue already holds, and
   * the numbered one answers to no file's own name.
   *
   * Empty on an entry from the embedded catalogue, which stands for no file.
   */
  constexpr int PRESET_FILE_ROLE = Qt::UserRole + 2;

  /** \brief What an entry adds to its name while it is showing such a curve.
   *
   * Held apart from the name by a non-breaking space, and written as a code
   * point rather than as a literal so that it survives whatever encoding a
   * compiler reads this file in. A breaking one would let a name wrapped across
   * the lines of a cell leave the marker stranded on a line of its own.
   */
  QString PresetEditedMarker()
  {
    return QChar(0x00A0) + QStringLiteral("*");
  }

  /** \brief A preview's shape.
   *
   * Held constant so that the render and the cell it is drawn into never
   * disagree about the aspect ratio, whatever width the panel allows.
   */
  constexpr double PREVIEW_ASPECT = 86.0 / 110.0;

  /** \brief How many previews stand side by side however narrow the panel.
   *
   * There is no panel width to size cells against: the workbench gives the
   * panel a fraction of the window, so it is far narrower on a laptop than on
   * a workstation, and any fixed cell width means a different number of
   * previews per row on each. Fixing the count instead, and deriving the cells
   * from it, is what makes the grid read the same on both.
   */
  constexpr int MIN_COLUMNS = 3;

  /** \brief The cell width worth having, once there is room for it.
   *
   * Not a minimum: MIN_COLUMNS wins on a narrow panel. This only decides when
   * a wide panel has earned another column rather than larger previews.
   */
  constexpr int PREFERRED_CELL_WIDTH = 118;

  /** \brief The width of a preview standing beside a name rather than above it.
   *
   * Small enough that a row stays close to the height of a line of text, which
   * is the point of that presentation: a preview large enough to study is what
   * the grid is for, and one this size only says which colours a preset brings.
   */
  constexpr int COMPACT_PREVIEW_WIDTH = 32;

  /** \brief The margin between a preview and the edges of its cell. */
  constexpr int CELL_PADDING = 4;

  /** \brief How strongly the stand-in for a missing preview is drawn. */
  constexpr double PLACEHOLDER_OPACITY = 0.4;

  /** \brief Width left to the view to lay the cells out in.
   *
   * Determined by observation: the view fits one cell fewer than the viewport
   * would hold when the cells add up to all of it, and needs two pixels over
   * that to fit them all. A couple more than two is invisible and leaves room
   * for a style that reserves more.
   */
  constexpr int VIEWPORT_RESERVE = 4;

  /** \brief The width every preview is drawn at, once.
   *
   * Derived rather than chosen: a cell only grows to just under twice
   * PREFERRED_CELL_WIDTH before the panel is wide enough for another column,
   * so this is the widest a preview can ever be asked to appear at. Drawing at
   * that width means previews are only ever scaled down, and never have to be
   * drawn a second time because the panel was resized.
   */
  constexpr int PREVIEW_RENDER_WIDTH = 2 * PREFERRED_CELL_WIDTH - 2 * CELL_PADDING;

  /** \brief A preview's size given the width its cell allows. */
  QSize PreviewSize(int width)
  {
    return QSize(width, static_cast<int>(std::lround(width * PREVIEW_ASPECT)));
  }

  /** \brief A stand-in for a preview not drawn yet.
   *
   * Entries are sized to what they hold, so one that has only its name is
   * shorter than one with a preview. Filling them all in at the final size
   * from the start keeps the grid from being laid out twice - once for the
   * names, and again, entry by entry, as previews arrive.
   *
   * Drawn rather than left blank so that a cell without a preview reads as one
   * still to come. Nothing distinguishes "no image selected" from "previews are
   * being drawn" here, and nothing needs to: the view disables the list in the
   * first case, and Qt fades a disabled item's icon of its own accord.
   *
   * \param[in] size  The pixel size the cells reserve for a preview.
   * \param[in] color The theme's icon colour. Drawn at part opacity, since the
   *                  mark stands in for content rather than being content.
   */
  QIcon PlaceholderPreview(const QSize &size, const QColor &color)
  {
    QPixmap placeholder(size);
    placeholder.fill(Qt::transparent);

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(PLACEHOLDER_OPACITY);
    painter.setPen(color);

    // Half a pixel in on every side: a one-pixel pen straddles the path it is
    // given, so a frame on the pixmap's own edge would lose its outer half.
    const QRectF frame(0.5, 0.5, size.width() - 1.0, size.height() - 1.0);

    painter.drawRoundedRect(frame, 3.0, 3.0);
    painter.drawLine(frame.topRight(), frame.bottomLeft());

    return QIcon(placeholder);
  }

  /** \brief What the entry that loads a preset shows in place of a preview.
   *
   * The stand-in's frame, so that it reads as a cell of the same grid, with a
   * plus where the stand-in has its line: a cell still to be filled, and by
   * whoever clicks it.
   *
   * \param[in] size  The pixel size the cells reserve for a preview.
   * \param[in] color The theme's icon colour.
   */
  QIcon LoadPresetIcon(const QSize &size, const QColor &color)
  {
    QPixmap icon(size);
    icon.fill(Qt::transparent);

    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(PLACEHOLDER_OPACITY);
    painter.setPen(color);

    const QRectF frame(0.5, 0.5, size.width() - 1.0, size.height() - 1.0);

    painter.drawRoundedRect(frame, 3.0, 3.0);

    // Sized from the shorter side, so that the list's small previews still get
    // a plus rather than a smudge.
    const double arm = 0.2 * std::min(frame.width(), frame.height());
    const QPointF centre = frame.center();

    // At full strength, unlike the frame: the frame says the cell is empty, the
    // plus that clicking it does something, and a faded one reads as disabled.
    painter.setOpacity(1.0);
    painter.setPen(QPen(color, 2.0));
    painter.drawLine(centre - QPointF(arm, 0.0), centre + QPointF(arm, 0.0));
    painter.drawLine(centre - QPointF(0.0, arm), centre + QPointF(0.0, arm));

    return QIcon(icon);
  }

  /** \brief A colour stop as one entry of a list can show it.
   *
   * Outlined, because a stop whose colour is near the list's own background
   * would otherwise be an entry with nothing in front of its name.
   */
  QIcon ColorSwatch(const QColor &color)
  {
    QPixmap swatch(12, 12);
    swatch.fill(color);

    QPainter painter(&swatch);
    painter.setPen(Qt::gray);
    painter.drawRect(0, 0, swatch.width() - 1, swatch.height() - 1);

    return QIcon(swatch);
  }

  /** \brief The preset an entry stands for. */
  QString PresetName(const QListWidgetItem *item)
  {
    return item->data(PRESET_NAME_ROLE).toString();
  }

  /** \brief Where the preset an entry stands for came from. */
  PresetOrigin PresetOriginOf(const QListWidgetItem *item)
  {
    return static_cast<PresetOrigin>(item->data(PRESET_ORIGIN_ROLE).toInt());
  }

  /** \brief The file an entry was read from, empty for one of the embedded
   *         catalogue's.
   */
  QString PresetFile(const QListWidgetItem *item)
  {
    return item->data(PRESET_FILE_ROLE).toString();
  }

  /** \brief What a node records a preset of this origin under.
   *
   * The one place the recorded spelling is written, so that nothing else can
   * arrive at a different one.
   */
  QString PresetOriginPrefix(PresetOrigin origin)
  {
    return origin == PresetOrigin::File ? QStringLiteral("file:") : QStringLiteral("internal:");
  }

  /** \brief A preset as a node records it. */
  struct RecordedPreset
  {
    PresetOrigin origin;
    QString name;
  };

  /** \brief What a recorded value stands for.
   *
   * A value carrying no origin was written before origins were recorded, when a
   * preset from the embedded catalogue was the only kind there was.
   */
  RecordedPreset ParseRecordedPreset(const QString &recorded)
  {
    for (const auto origin : { PresetOrigin::Internal, PresetOrigin::File })
    {
      const QString prefix = PresetOriginPrefix(origin);

      if (recorded.startsWith(prefix))
        return { origin, recorded.mid(prefix.length()) };
    }

    return { PresetOrigin::Internal, recorded };
  }

  /** \brief The row holding the named preset, or -1.
   *
   * \param[in] presetName The name to look for. The empty name a node records
   *                       no preset under matches nothing, which is what leaves
   *                       the grid with no entry marked - not even the load
   *                       entry, which carries no name either.
   */
  int FindPresetRow(const QListWidget *presetList, const QString &presetName)
  {
    if (presetName.isEmpty())
      return -1;

    for (int i = 0; i < presetList->count(); ++i)
    {
      if (PresetName(presetList->item(i)) == presetName)
        return i;
    }

    return -1;
  }

  /** \brief The entry read from the given file, or nullptr.
   *
   * By file rather than by name: the name a file's preset gets is only its own
   * while no other preset holds it, and the file is what saving and loading are
   * asked about.
   *
   * \param[in] presetFile An absolute path. Never empty, so the entries that
   *                       stand for no file match nothing.
   */
  QListWidgetItem *FindPresetFileItem(const QListWidget *presetList, const QString &presetFile)
  {
    for (int i = 0; i < presetList->count(); ++i)
    {
      if (PresetFile(presetList->item(i)) == presetFile)
        return presetList->item(i);
    }

    return nullptr;
  }

  /** \brief Whether a slider has been carried off the value given here.
   *
   * Measured against half a step rather than by equality: ctkDoubleSlider
   * rounds a value onto an integer slider and re-bases an offset so as to
   * answer with what it was handed, and two such round trips need not come back
   * bit-identical. Half a step is also the smallest difference a slider can be
   * put at all.
   */
  bool DiffersFrom(const ctkDoubleSlider *slider, double neutral)
  {
    return std::abs(slider->value() - neutral) > 0.5 * slider->singleStep();
  }

  /** \brief Whether the node is rendered as a volume at all.
   *
   * Nothing outside the volume visualization controls writes the property, so it
   * doubles as the marker that someone deliberately configured this node there.
   * "TransferFunction" cannot serve that purpose: mitk::VolumeMapperVtkSmart3D
   * registers a default one on every image node, so its presence says nothing.
   */
  bool IsVolumeRenderingOn(const mitk::DataNode *node)
  {
    if (node == nullptr)
      return false;

    bool volumeRenderingOn = false;
    node->GetBoolProperty("volumerendering", volumeRenderingOn);

    return volumeRenderingOn;
  }

  /** \brief Whether the node's curve was chosen here but answers to no preset.
   *
   * The other half of the evidence that this node was configured here, for the
   * curves the recorded preset name cannot cover. See TF_CUSTOM_PROPERTY.
   */
  bool IsCustomTransferFunction(const mitk::DataNode *node)
  {
    if (node == nullptr)
      return false;

    bool customTransferFunction = false;
    node->GetBoolProperty(TF_CUSTOM_PROPERTY, customTransferFunction);

    return customTransferFunction;
  }

  /** \brief The intensity band the editor is scaled to.
   *
   * Not the image's outermost values: one saturated voxel is enough to set the
   * axis the canvas draws and all four sliders are sized against, leaving the
   * curve a pixel wide and a single slider step wider than the curve itself.
   * mitk::LevelWindow::SetAuto answers with the band instead, substituting the
   * second extreme for one that too few voxels carry to matter, and is what the
   * 2D views show the same image over.
   *
   * \return The band, the histogram's own bounds where the image names none, or
   *         nothing where neither can.
   */
  std::optional<std::array<double, 2>> WorkingRange(const mitk::Image *image,
                                                    const mitk::SimpleHistogram *histogram)
  {
    if (image != nullptr && image->IsInitialized())
    {
      mitk::LevelWindow band;

      // Over the whole volume rather than its central slice, which an outlier
      // can miss. Only the third argument carries meaning here; SetAuto documents
      // the second as unused.
      band.SetAuto(image, true, false);

      double lower = band.GetLowerWindowBound();
      double upper = band.GetUpperWindowBound();

      // SetAuto pads the range before deciding what to discard from it, and on
      // an image holding two distinct values the padding is all that survives:
      // the band then reaches past every voxel in the image. Bounded by the
      // histogram, which is the axis this had before the band was consulted, so
      // the worst the band can do is fail to narrow it.
      if (histogram != nullptr)
      {
        lower = std::max(lower, histogram->GetMin());
        upper = std::min(upper, histogram->GetMax());
      }

      if (lower < upper)
        return std::array{ lower, upper };
    }

    if (histogram != nullptr)
      return std::array{ histogram->GetMin(), histogram->GetMax() };

    return std::nullopt;
  }
}

QmitkVolumeTransferFunctionEditor::QmitkVolumeTransferFunctionEditor(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_Controls(std::make_unique<Ui::QmitkVolumeTransferFunctionEditor>()),
    // Drawn once at the width the widest cell could want, and scaled down to
    // whatever the current one allows, so that resizing the panel relays the
    // grid out rather than drawing every preview again. Built here rather than
    // in the body because InvalidateThumbnails releases the volume through it,
    // and the body calls that before it is done setting the widget up.
    m_ThumbnailRenderer(std::make_unique<QmitkVolumeThumbnailRenderer>(PreviewSize(PREVIEW_RENDER_WIDTH)))
{
  m_Controls->setupUi(this);

  auto *presetList = m_Controls->presetListWidget;

  // The entries state the size they occupy themselves, so there is nothing
  // left here for the view to measure and hold on to. Leaving this off is what
  // has it read that size again after every re-measure, rather than the one it
  // happened to see first.
  presetList->setUniformItemSizes(false);

  presetList->setResizeMode(QListView::Adjust);

  // Icon mode lets entries be dragged around by default. This is a menu.
  presetList->setMovement(QListView::Static);

  presetList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  presetList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  // The application stylesheet greys a disabled item's text but not the
  // selection behind it, so the preset in force would keep a full-strength
  // highlight while the editor is switched off. Translucent rather than a fixed
  // colour, since it has to lighten a dark background and darken a light one,
  // and QmitkStyleManager exposes only icon colours to ask the theme for.
  presetList->setStyleSheet(
    "QListWidget::item:selected:disabled { background-color: rgba(127, 127, 127, 90); }");

  // Taken before the remembered files are read in, which is what lets the loop
  // below tell the two apart: what the catalogue already held is its own, and
  // everything standing after it came from a file.
  const auto builtInPresetCount = m_Presets.GetPresetNames().size();

  // Before the grid is filled rather than after, so that what was saved in
  // earlier sessions is in the catalogue by the time the loop below reads it
  // and needs no entry of its own making.
  const QStringList presetFiles = this->LoadRememberedPresets();

  const auto presetNames = m_Presets.GetPresetNames();

  for (std::size_t i = 0; i < presetNames.size(); ++i)
  {
    const auto presetName = QString::fromStdString(presetNames[i]);
    const bool builtIn = i < builtInPresetCount;

    // The name goes in beside the text as well as in it, because the text is
    // what the edited marker is appended to. See PRESET_NAME_ROLE.
    auto *presetItem = new QListWidgetItem(presetName, presetList);
    presetItem->setData(PRESET_NAME_ROLE, presetName);
    presetItem->setData(PRESET_ORIGIN_ROLE, static_cast<int>(
      builtIn ? PresetOrigin::Internal : PresetOrigin::File));

    // The files were read in the order the catalogue took them, and it appends,
    // so the entries standing after the built-in ones are those files in order.
    if (!builtIn)
      presetItem->setData(PRESET_FILE_ROLE,
        presetFiles.value(static_cast<qsizetype>(i - builtInPresetCount)));
  }

  // Before the cells are measured below, so that it is measured and given its
  // icon along with them.
  m_LoadPresetItem = new QListWidgetItem(QStringLiteral("Load preset..."), presetList);
  m_LoadPresetItem->setToolTip("Add a preset from a transfer function saved to a JSON file.");

  // Enabled, so that it is not drawn greyed out, but never selected: the
  // selection marks the preset in force, and this is none.
  m_LoadPresetItem->setFlags(Qt::ItemIsEnabled);

  // Cells are measured from the names they have to hold, so the entries come
  // first, and the stand-in previews after them, since their size is what the
  // measurement settles. The grid of previews is what it opens in, and asking
  // for it is also what puts the first icon on the button offering the other.
  //
  // The panel is only ever as wide as the workbench window makes it, so the
  // cells are measured from it rather than fixed, and measured again whenever
  // it changes. Watching the viewport rather than overriding this widget's own
  // resizeEvent: the layout has not necessarily handed the list its new
  // geometry by the time that event arrives, and a stale width would be
  // measured.
  this->SetCompactPresetList(false);
  this->InvalidateThumbnails();

  presetList->viewport()->installEventFilter(this);

  // Dragging across the grid, by as little as a pixel, is a selection rectangle
  // to the view: it covers no entry, so the selection it draws is empty and the
  // one that was there is cleared. Ctrl-clicking the marked entry drops it just
  // as directly. Neither says which preset is applied - the current entry is
  // what records that - so the mark goes back on it.
  connect(presetList, &QListWidget::itemSelectionChanged, this,
    [this]
    {
      auto *currentPreset = m_Controls->presetListWidget->currentItem();

      if (currentPreset != nullptr && !currentPreset->isSelected())
        currentPreset->setSelected(true);
    });

  // A freshly filled list lands on its first entry, which would name a preset
  // nothing has applied.
  this->ClearPresetSelection();

  m_Controls->opacityShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->opacityHeightSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorShiftSlider->setOrientation(Qt::Horizontal);
  m_Controls->colorWidthSlider->setOrientation(Qt::Horizontal);

  // Set here rather than in the .ui: the resource is authored with a
  // placeholder fill that QmitkStyleManager swaps for the theme's icon colour,
  // so a direct reference from the .ui would draw it in that placeholder.
  m_Controls->resetTfButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/reset.svg")));

  // A click rather than the current entry changing: the current entry is also
  // set from what a node records, and reacting to that would re-apply the
  // preset and discard the curve the node was carrying.
  connect(m_Controls->presetListWidget, &QListWidget::itemClicked, this,
    [this](QListWidgetItem *item)
    {
      if (item != nullptr)
        this->OnPresetSelected(PresetName(item));
    });

  // The policy rather than an overridden contextMenuEvent, which is how the rest
  // of MITK asks a view for a menu.
  m_Controls->presetListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Controls->presetListWidget, &QListWidget::customContextMenuRequested,
    this, &QmitkVolumeTransferFunctionEditor::OnPresetContextMenu);

  connect(m_Controls->presetViewModeButton, &QToolButton::clicked, this,
    [this] { this->SetCompactPresetList(!m_CompactPresetList); });

  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::OpacityChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnCanvasOpacityChanged);
  connect(m_Controls->opacityShiftSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityShift);
  connect(m_Controls->opacityHeightSlider, &ctkDoubleSlider::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetOpacityHeight);
  connect(m_Controls->colorShiftSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnColorWindowChanged);
  connect(m_Controls->colorWidthSlider, &ctkDoubleSlider::valueChanged,
    this, &QmitkVolumeTransferFunctionEditor::OnColorWindowChanged);
  connect(m_Controls->resetTfButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnResetAdjustments);

  connect(m_Controls->editModeButton, &QToolButton::toggled,
    this, &QmitkVolumeTransferFunctionEditor::SetEditModeActive);

  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::PointsChanged,
    this, [this]
    {
      m_CurveEdited = true;
      this->ShowPresetEdited();
    });

  connect(m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::ColorStopsChanged,
    this, &QmitkVolumeTransferFunctionEditor::ShowColorStops);

  connect(m_Controls->fitAxisCheckBox, &QCheckBox::toggled,
    this, &QmitkVolumeTransferFunctionEditor::ApplyAxisRange);

  // Each of these hands the request straight to the canvas, which is where the
  // rules about what may happen to a stop live, so that a stop moved from here
  // and one dragged on the canvas cannot come out differently.
  connect(m_Controls->colorStopComboBox, &QComboBox::currentIndexChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetSelectedColorStop);
  connect(m_Controls->colorStopColorButton, &QPushButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnPickColorStopColor);
  connect(m_Controls->colorStopOffsetSpinBox, &QDoubleSpinBox::valueChanged,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::SetSelectedColorStopOffset);
  connect(m_Controls->removeColorStopButton, &QToolButton::clicked,
    m_Controls->combinedTfCanvas, &QmitkCombinedTransferFunctionCanvas::RemoveSelectedColorStop);
  connect(m_Controls->addColorStopButton, &QToolButton::clicked,
    this, &QmitkVolumeTransferFunctionEditor::OnAddColorStop);

  // Editing starts off.
  this->ShowEditMode();
}

QmitkVolumeTransferFunctionEditor::~QmitkVolumeTransferFunctionEditor() = default;

QStringList QmitkVolumeTransferFunctionEditor::LoadRememberedPresets()
{
  const QStringList rememberedFiles = RememberedPresetFiles();

  QStringList foundFiles;

  for (const auto &presetFile : rememberedFiles)
  {
    std::ifstream stream(presetFile.toStdString());

    if (!stream.is_open())
    {
      MITK_ERROR << "Transfer function preset file \"" << presetFile.toStdString()
                 << "\" is no longer there, and is no longer offered.";
      continue;
    }

    if (m_Presets.AddPreset(stream).empty())
    {
      MITK_ERROR << "Transfer function preset file \"" << presetFile.toStdString()
                 << "\" holds no preset, and is no longer offered.";
      continue;
    }

    foundFiles.append(presetFile);
  }

  // Only once something has gone, because flushing writes out every preference
  // the application holds, and the list is otherwise already what it says.
  if (foundFiles.size() != rememberedFiles.size())
    RememberPresetFiles(foundFiles);

  return foundFiles;
}

bool QmitkVolumeTransferFunctionEditor::eventFilter(QObject *watched, QEvent *event)
{
  auto *presetList = m_Controls->presetListWidget;

  if (watched == presetList->viewport())
  {
    if (event->type() == QEvent::Resize)
      this->UpdatePresetLayout();

    // A double click as well as a press, since the view takes a double click on
    // an entry it did not see pressed as a press of its own. The release is left
    // to the view: a press it did see, elsewhere, still has to be finished.
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
    {
      const auto *mouseEvent = static_cast<QMouseEvent *>(event);

      if (presetList->itemAt(mouseEvent->position().toPoint()) == m_LoadPresetItem)
      {
        if (event->type() == QEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton)
          this->LoadPreset();

        return true;
      }
    }
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkVolumeTransferFunctionEditor::changeEvent(QEvent *event)
{
  QWidget::changeEvent(event);

  if (event->type() != QEvent::EnabledChange)
    return;

  // Nothing else announces that the previews became worth drawing: switching
  // volume rendering on deliberately does not re-bind the node.
  if (this->isEnabled())
  {
    this->StartThumbnailGeneration();
    return;
  }

  // Switching volume rendering off is what greys the editor out, and an edit
  // cannot outlive the thing it was being made to. What was drawn stays on the
  // node: it was written into the function as it was drawn.
  this->SetEditModeActive(false);
}

void QmitkVolumeTransferFunctionEditor::SetCompactPresetList(bool compact)
{
  m_CompactPresetList = compact;

  auto *presetList = m_Controls->presetListWidget;

  presetList->setViewMode(compact ? QListView::ListMode : QListView::IconMode);

  // Setting a mode adjusts only the properties nobody has set explicitly, so
  // the movement, the resize mode and the scrolling from the constructor come
  // through it and the flow follows the mode. Wrapping was set there too, so it
  // has to be said again: a list keeps one entry per row rather than packing
  // them across. Word wrap is no mode's business either way - a name beside its
  // preview has the row to itself, and is cut short rather than broken in two.
  presetList->setWrapping(!compact);
  presetList->setWordWrap(!compact);

  this->UpdatePresetLayout();

  // The two presentations put the same entry at different heights, and what a
  // reader looks for first after a switch is the one in force.
  if (auto *currentPreset = presetList->currentItem(); currentPreset != nullptr)
    presetList->scrollToItem(currentPreset);

  // The icon names the presentation pressing the button brings rather than the
  // one in force, the way the view's rendering button names what pressing it
  // does.
  m_Controls->presetViewModeButton->setIcon(QmitkStyleManager::ThemeIcon(compact
    ? QStringLiteral(":/VolumeVisualizationUI/view-list-icons.svg")
    : QStringLiteral(":/VolumeVisualizationUI/view-list-details.svg")));

  m_Controls->presetViewModeButton->setToolTip(compact
    ? "Show the presets as a grid of previews."
    : "Show the presets as a list, with a smaller preview beside each name.");
}

void QmitkVolumeTransferFunctionEditor::UpdatePresetLayout()
{
  auto *presetList = m_Controls->presetListWidget;

  // The viewport rather than the list: it is what remains once the frame and
  // the vertical scroll bar are accounted for, and that bar is a column of
  // pixels on Windows but nothing at all on macOS, where it floats over the
  // content.
  const int available = presetList->viewport()->width();

  if (available <= 0)
    return;

  // The view keeps a margin of its own inside the viewport, so cells adding up
  // to the whole of it are one too many to fit: the last wraps and leaves most
  // of a cell empty. Two pixels is what it takes here, and a couple more than
  // that costs nothing visible while covering styles that take more.
  const int usable = available - VIEWPORT_RESERVE;

  QSize previewSize;
  QSize cellSize;

  if (m_CompactPresetList)
  {
    previewSize = PreviewSize(COMPACT_PREVIEW_WIDTH);

    // A row takes the width it is given rather than the width it wants: stated
    // here, the entry in force is marked across the whole panel instead of
    // only behind its name. Its height is the taller of what it has to hold,
    // since a preview this small can come out shorter than a line of text.
    cellSize = QSize(usable,
                     std::max(previewSize.height(), presetList->fontMetrics().height()) +
                       2 * CELL_PADDING);
  }
  else
  {
    const int columns = std::max(MIN_COLUMNS, usable / PREFERRED_CELL_WIDTH);

    // Dividing the width by the columns rather than the other way around is
    // what has the cells share out what there is, instead of leaving whatever
    // the last column did not need unused at the right.
    const int cellWidth = usable / columns;

    previewSize = PreviewSize(cellWidth - 2 * CELL_PADDING);

    // How many lines a name takes is not something to assume: the catalogued
    // ones are hyphenated and Qt breaks a line at a hyphen, so the count
    // follows from the width a cell has and from how wide the platform's
    // interface font draws the characters. The rect overload constrains
    // wrapping by its width and leaves the height free, which is the
    // measurement the delegate itself makes when it paints a name. The tallest
    // of them keeps every cell the same height and none of them too short.
    //
    // Measured with the edited marker on every name, though at most one wears
    // it: cells are sized here and not again when one appears, so a name the
    // marker carries onto another line would be cut off in a cell measured
    // without it.
    const QFontMetrics metrics = presetList->fontMetrics();

    const auto wrappedHeight = [&](const QString &text)
    {
      return metrics.boundingRect(QRect(0, 0, previewSize.width(), 0), Qt::TextWordWrap, text).height();
    };

    int nameHeight = metrics.lineSpacing();

    for (int i = 0; i < this->PresetCount(); ++i)
      nameHeight = std::max(nameHeight, wrappedHeight(PresetName(presetList->item(i)) + PresetEditedMarker()));

    // Its text is not a preset's name, and never wears the marker.
    nameHeight = std::max(nameHeight, wrappedHeight(m_LoadPresetItem->text()));

    cellSize = QSize(cellWidth, previewSize.height() + nameHeight + 2 * CELL_PADDING);
  }

  // The grid decides where a cell goes only for entries that fill it. Left to
  // size themselves, they come out a couple of pixels narrower - the padding
  // here is wider than the margin the delegate keeps of its own accord - and
  // the view then packs each row from its entries' widths rather than from the
  // grid, so the columns of one row do not line up with those of the next.
  // Stating the size every entry is going to occupy is what holds them in step.
  for (int i = 0; i < presetList->count(); ++i)
    presetList->item(i)->setSizeHint(cellSize);

  presetList->setIconSize(previewSize);

  // An invalid size is how a view is told it has no grid, which is what a
  // single column of entries sized to the panel already is.
  presetList->setGridSize(m_CompactPresetList ? QSize() : cellSize);

  // The size just settled on is the one the stand-ins were built for, and on
  // the first pass they were built for a viewport no layout had sized yet.
  this->RefreshPlaceholders();
}

void QmitkVolumeTransferFunctionEditor::SetDataNode(mitk::DataNode *node)
{
  // An edit belongs to the node that was current when it started, and leaving
  // is what records the curve on that node. This has to happen before the
  // function is dropped below, or the record is written against the wrong node
  // - or against none at all.
  this->SetEditModeActive(false);

  m_DataNode = node;
  m_AppliedTransferFunction = nullptr;

  // A drawing belongs to the node it was made on, and nothing recorded it there.
  m_CurveDrawnOver = false;

  // Previews belong to the image they were drawn from, so a different one
  // leaves them describing nothing that is on screen.
  if (m_ThumbnailImage != (node != nullptr ? node->GetDataAs<mitk::Image>() : nullptr))
    this->InvalidateThumbnails();

  this->AdoptTransferFunctionFromNode();

  this->StartThumbnailGeneration();
}

void QmitkVolumeTransferFunctionEditor::EnsureTransferFunction()
{
  if (m_AppliedTransferFunction.IsNotNull())
    return;

  auto node = m_DataNode.Lock();

  const auto presetName = QString::fromStdString(m_Presets.GetDefaultPresetName(
    node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr));

  // The grid was filled from the same catalog, so a name it vouches for has a
  // row. Nothing matches the empty name an empty catalog returns, which is the
  // one case this guards.
  const int presetIndex = FindPresetRow(m_Controls->presetListWidget, presetName);

  if (presetIndex < 0)
    return;

  m_Controls->presetListWidget->setCurrentRow(presetIndex);

  this->OnPresetSelected(presetName);
}

void QmitkVolumeTransferFunctionEditor::OnPresetSelected(const QString &presetName)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  const auto name = presetName.toStdString();

  auto blendMode = mitk::VolumeBlendMode::Composite;
  auto preset = m_Presets.CreateTransferFunction(name, blendMode);

  if (preset.IsNull())
    return;

  // Picking a preset starts a new recipe: the offsets the previous one left are
  // measured from a baseline this curve does not have. The name goes with them
  // and is written again immediately below.
  this->ForgetTransferFunctionRecipe(node);

  m_AppliedTransferFunction = preset;

  // The preset stands in place of whatever was drawn over it.
  m_CurveDrawnOver = false;

  // Recorded with where it came from, since the catalogue answers to a name
  // whichever kind it holds and the name alone would not say. Looked up rather
  // than handed in: AddPreset keeps names unique across the catalogue, so the
  // entry wearing this one is the only entry it could mean.
  const int presetRow = FindPresetRow(m_Controls->presetListWidget, presetName);

  const auto origin = presetRow >= 0
    ? PresetOriginOf(m_Controls->presetListWidget->item(presetRow))
    : PresetOrigin::Internal;

  const auto recordedPreset = (PresetOriginPrefix(origin) + presetName).toStdString();

  node->SetStringProperty(TF_PRESET_PROPERTY, recordedPreset.c_str());

  // The mode travels with the curve: a window authored for MIP renders as a
  // white shell under composite, and a tissue classifier projected flat says
  // nothing. Applying it unconditionally is what makes the grid mean one thing
  // - the preset as its author intended it - rather than depending on the mode
  // the node happened to be left in.
  this->ApplyBlendMode(blendMode);

  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::ApplyBlendMode(mitk::VolumeBlendMode blendMode)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  mitk::SetVolumeBlendMode(node.GetPointer(), blendMode);
}

void QmitkVolumeTransferFunctionEditor::AdoptTransferFunctionFromNode()
{
  auto node = m_DataNode.Lock();

  std::string recordedPreset;

  if (node.IsNotNull())
  {
    node->GetStringProperty(TF_PRESET_PROPERTY, recordedPreset);

    // A recorded preset and the custom marker are the direct evidence that this
    // node was set up here, and both survive the rendering flag being switched
    // off. Between them they cover every curve this widget applies. The flag
    // covers what predates them: nodes configured before the recipe existed, or
    // by the v1 view. What cannot serve as evidence is the TransferFunction
    // property itself - see IsVolumeRenderingOn. Adopting the mapper's default
    // would show a curve nobody chose and would also suppress
    // EnsureTransferFunction, which fires only while no function is held.
    if (!recordedPreset.empty() || IsCustomTransferFunction(node.GetPointer()) ||
        IsVolumeRenderingOn(node.GetPointer()))
    {
      if (const auto *tfProperty =
            dynamic_cast<const mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction")))
      {
        m_AppliedTransferFunction = tfProperty->GetValue();
      }
    }
  }

  // The name is the key either way: AddPreset keeps it unique across the
  // catalogue, and the origin recorded beside it is there to be read rather
  // than to be searched by.
  const auto presetName = ParseRecordedPreset(QString::fromStdString(recordedPreset)).name;

  const int presetIndex = FindPresetRow(m_Controls->presetListWidget, presetName);

  if (presetIndex < 0)
    this->ClearPresetSelection();
  else
    m_Controls->presetListWidget->setCurrentRow(presetIndex);

  // A node naming a preset the catalogue still offers is described by its
  // recipe, and the recipe is what comes back. Only a curve no preset describes
  // - one whose preset was removed, or one that came from outside this view - is
  // shown as it stands.
  if (presetIndex >= 0 && this->ReplayRecipe(presetName))
    return;

  this->ShowAppliedTransferFunction();
}

bool QmitkVolumeTransferFunctionEditor::ReplayRecipe(const QString &presetName)
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return false;

  float opacityShift = 0.0f;
  float opacityHeight = 0.0f;
  float colorShift = 0.0f;
  float colorWidth = 0.0f;

  // All four or none. The neutral colour width is the preset's own span rather
  // than zero, so a missing value cannot be told apart from a deliberate one and
  // a partial recipe cannot be completed with defaults. Read before the preset
  // is applied, which is what clears them.
  const bool hasOffsets =
    node->GetFloatProperty(TF_OPACITY_SHIFT_PROPERTY, opacityShift) &&
    node->GetFloatProperty(TF_OPACITY_HEIGHT_PROPERTY, opacityHeight) &&
    node->GetFloatProperty(TF_COLOR_SHIFT_PROPERTY, colorShift) &&
    node->GetFloatProperty(TF_COLOR_WIDTH_PROPERTY, colorWidth);

  // Re-apply the preset first, so the baselines the offsets are measured from
  // are the pristine curves again. Showing an offset against an already-adjusted
  // baseline would make the next drag apply the whole offset a second time.
  //
  // The recipe therefore wins over the stored curve, whether the two came apart
  // here - a curve drawn point by point over the preset - or elsewhere, through
  // the Properties view or the v1 view. Either way the curve is what gets
  // discarded. That is the price of being able to keep adjusting a preset, and
  // the same trade the lighting model's recorded id already makes.
  this->OnPresetSelected(presetName);

  if (!hasOffsets)
    return true;

  // Left to the sliders' own signals rather than applied directly: they are
  // already wired to the canvas and to OnColorWindowChanged, and each of those
  // reads both of its pair, so the order here does not matter.
  m_Controls->opacityShiftSlider->setValue(opacityShift);
  m_Controls->opacityHeightSlider->setValue(opacityHeight);
  m_Controls->colorShiftSlider->setValue(colorShift);
  m_Controls->colorWidthSlider->setValue(colorWidth);

  return true;
}

void QmitkVolumeTransferFunctionEditor::RecordAdjustOffsets()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull())
    return;

  // An offset is only meaningful next to the preset it was measured from, so a
  // node naming none has no baseline these four could describe. Writing them
  // anyway would leave a recipe nothing can re-execute on the node, and in every
  // scene saved from it.
  std::string presetName;

  if (!node->GetStringProperty(TF_PRESET_PROPERTY, presetName))
    return;

  node->SetFloatProperty(TF_OPACITY_SHIFT_PROPERTY,
    static_cast<float>(m_Controls->opacityShiftSlider->value()));
  node->SetFloatProperty(TF_OPACITY_HEIGHT_PROPERTY,
    static_cast<float>(m_Controls->opacityHeightSlider->value()));
  node->SetFloatProperty(TF_COLOR_SHIFT_PROPERTY,
    static_cast<float>(m_Controls->colorShiftSlider->value()));
  node->SetFloatProperty(TF_COLOR_WIDTH_PROPERTY,
    static_cast<float>(m_Controls->colorWidthSlider->value()));
}

void QmitkVolumeTransferFunctionEditor::ForgetAdjustOffsets(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  // Removing the keys rather than blanking them keeps absence as the single
  // meaning each of them carries, which is what the restore path reads.
  auto *properties = node->GetPropertyList();

  properties->DeleteProperty(TF_OPACITY_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_OPACITY_HEIGHT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_SHIFT_PROPERTY);
  properties->DeleteProperty(TF_COLOR_WIDTH_PROPERTY);
}

void QmitkVolumeTransferFunctionEditor::ForgetTransferFunctionRecipe(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  this->ForgetAdjustOffsets(node);

  auto *properties = node->GetPropertyList();

  properties->DeleteProperty(TF_PRESET_PROPERTY);
  properties->DeleteProperty(TF_CUSTOM_PROPERTY);
}

void QmitkVolumeTransferFunctionEditor::RecordCustomTransferFunction(mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  this->ForgetTransferFunctionRecipe(node);

  node->SetBoolProperty(TF_CUSTOM_PROPERTY, true);
}

void QmitkVolumeTransferFunctionEditor::ClearPresetSelection()
{
  m_Controls->presetListWidget->setCurrentRow(-1);
}

void QmitkVolumeTransferFunctionEditor::ApplyCurrentTransferFunction()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull() || m_AppliedTransferFunction.IsNull())
    return;

  // Preserve property identity: reuse the existing property, create it only once.
  auto *tfProperty = dynamic_cast<mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction"));

  if (tfProperty != nullptr)
  {
    tfProperty->SetValue(m_AppliedTransferFunction);
  }
  else
  {
    node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(m_AppliedTransferFunction));
  }

  this->ShowAppliedTransferFunction();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::ShowAppliedTransferFunction()
{
  auto node = m_DataNode.Lock();

  if (m_AppliedTransferFunction.IsNull())
  {
    // Not merely cosmetic: the canvas keeps the functions as raw pointers owned
    // by the transfer function just dropped, so leaving them in place leaves
    // them reachable after the node that owned them is deleted.
    m_Controls->combinedTfCanvas->Clear();
    m_DataRange = { 0.0, 0.0 };
  }
  else
  {
    auto *image = node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr;
    mitk::SimpleHistogram *histogram = (image != nullptr) ? m_HistogramCache[image] : nullptr;

    // One that failed to compute answers 0 and 1 for its bounds rather than
    // reporting the failure, and those would pass for a data range and collapse
    // the axis and all four sliders onto a one-unit span. Taken as absent, which
    // everything downstream already reads as "no range known".
    if (histogram != nullptr && !histogram->GetValid())
      histogram = nullptr;

    m_Controls->combinedTfCanvas->SetHistogram(histogram);
    m_Controls->combinedTfCanvas->SetColorTransferFunction(m_AppliedTransferFunction->GetColorTransferFunction());
    m_Controls->combinedTfCanvas->SetPiecewiseFunction(m_AppliedTransferFunction->GetScalarOpacityFunction());

    if (const auto range = WorkingRange(image, histogram); range.has_value())
    {
      m_DataRange = *range;
    }
    else
    {
      // Nothing measured the image, so the canvas is still on the function's own
      // range. Following it is what keeps ResetAdjustSliders from scaling the
      // sliders against whichever node was selected before.
      m_DataRange = { m_Controls->combinedTfCanvas->GetMin(),
                      m_Controls->combinedTfCanvas->GetMax() };
    }

    // The visible x-axis, so histogram, gradient and curve line up. (Both
    // branches above leave the canvas on the range SetPiecewiseFunction defaulted
    // it to - the function's own - so this must come after them.)
    this->ApplyAxisRange();

    m_Controls->combinedTfCanvas->SnapshotOpacityBaseline();
  }

  this->SnapshotAppliedTransferFunction();
  this->ResetAdjustSliders();

  // Whatever is on show is the baseline from here on, so an edit that led to it
  // is over rather than outstanding. Left set, it would mark the next preset
  // clicked as edited before anything touched it.
  m_CurveEdited = false;

  this->ShowPresetEdited();
  this->UpdateControlAvailability();
}

void QmitkVolumeTransferFunctionEditor::UpdateControlAvailability()
{
  auto node = m_DataNode.Lock();

  // The adjust controls and the canvas mean nothing without a function to act
  // on, and neither does editing, which starts from the applied curve. Picking
  // or loading a preset gets by with a node alone.
  //
  // While a curve is being edited the two that would replace it stand down:
  // the sliders replay a baseline snapshotted before the edits began, and a
  // preset, picked or loaded, would overwrite the curve outright.
  const bool hasNode = node.IsNotNull();
  const bool adjustable = hasNode && m_AppliedTransferFunction.IsNotNull();

  m_Controls->presetViewModeButton->setEnabled(hasNode);
  m_Controls->presetListWidget->setEnabled(hasNode && !m_EditModeActive);
  m_Controls->editModeButton->setEnabled(adjustable);
  m_Controls->adjustPresetPanel->setEnabled(adjustable && !m_EditModeActive);
  m_Controls->combinedTfCanvas->setEnabled(adjustable);
}

void QmitkVolumeTransferFunctionEditor::SnapshotAppliedTransferFunction()
{
  if (m_AppliedTransferFunction.IsNull())
  {
    m_BaseColorFn = nullptr;
    return;
  }

  // Keep the untouched copy of the color function to resample from:
  // DeepCopy preserves the color space and clamping, so windowing
  // stays faithful to the preset. Sampling bare RGB points instead would
  // interpolate in the wrong color space and shift the colors on the
  // first slider move.
  m_BaseColorFn = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_BaseColorFn->DeepCopy(m_AppliedTransferFunction->GetColorTransferFunction());
}

double QmitkVolumeTransferFunctionEditor::NeutralColorWidth() const
{
  // The width at which the colour window reproduces the baseline unchanged, and
  // so the value the width slider resets to. Falls back to the image's range for
  // a baseline that names none of its own.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);

  if (m_BaseColorFn == nullptr || m_BaseColorFn->GetSize() == 0)
    return dataWidth;

  const double *colorRange = m_BaseColorFn->GetRange();

  return std::max(1.0, colorRange[1] - colorRange[0]);
}

void QmitkVolumeTransferFunctionEditor::ResetAdjustSliders()
{
  const QSignalBlocker blockOpacityShift(m_Controls->opacityShiftSlider);
  const QSignalBlocker blockOpacityWidth(m_Controls->opacityHeightSlider);
  const QSignalBlocker blockShift(m_Controls->colorShiftSlider);
  const QSignalBlocker blockWidth(m_Controls->colorWidthSlider);

  // Scale the sliders to the image's own value range -- the same axis the canvas
  // draws -- so the window can be moved and sized across everything you see.
  const double dataWidth = std::max(1.0, m_DataRange[1] - m_DataRange[0]);
  const double colorSpan = this->NeutralColorWidth();

  // Slide the curve along the intensity range, reaches half the data span either side
  m_Controls->opacityShiftSlider->setMinimum(-0.5 * dataWidth);
  m_Controls->opacityShiftSlider->setMaximum(0.5 * dataWidth);
  m_Controls->opacityShiftSlider->setSingleStep(dataWidth / 1000.0);
  m_Controls->opacityShiftSlider->setValue(0.0);

  // Signed offset in [-1, 1]; small range needs a fine step (default is 1.0).
  m_Controls->opacityHeightSlider->setMinimum(-1.0);
  m_Controls->opacityHeightSlider->setMaximum(1.0);
  m_Controls->opacityHeightSlider->setSingleStep(0.01);
  m_Controls->opacityHeightSlider->setValue(0.0);

  // Shift moves the window center (level); 0 keeps the preset's own center.
  //
  // The window is colorSpan wide and slides along the image's own axis, so the
  // reach has to cover both before it can be carried clear of either end. Sized
  // to the preset alone, a preset authored over a narrower range than the image
  // - any MR- preset on an uncalibrated volume - stays stuck near where its
  // author put it however far the data extends.
  const double colorShiftReach = dataWidth + colorSpan;

  m_Controls->colorShiftSlider->setMinimum(-0.5 * colorShiftReach);
  m_Controls->colorShiftSlider->setMaximum(0.5 * colorShiftReach);
  m_Controls->colorShiftSlider->setSingleStep(colorShiftReach / 1000.0);
  m_Controls->colorShiftSlider->setValue(0.0);

  // Width is the window size in intensity units; default to the preset's color
  // span so a fresh preset maps 1:1, and allow narrowing/widening around it.
  //
  // Measured against the image as well as the preset, for the same reason the
  // shift is: a window that cannot be opened past the preset's own span can
  // never cover a volume wider than it. The window grows about the preset's
  // center rather than the image's - the CT- presets name absolute Hounsfield
  // values and have to keep them - so half of any widening falls outside the
  // data, and the shift is what carries it back.
  const double colorWidthReach = 2.0 * std::max(colorSpan, dataWidth);

  // Stepped, because ctkDoubleSlider defaults to a step of 1.0, which leaves a
  // preset whose span is small - DTI-FA-Brain's is 0.995 - two usable positions.
  m_Controls->colorWidthSlider->setMinimum(1.0);
  m_Controls->colorWidthSlider->setMaximum(colorWidthReach);
  m_Controls->colorWidthSlider->setSingleStep(colorWidthReach / 1000.0);
  m_Controls->colorWidthSlider->setValue(colorSpan);
}

void QmitkVolumeTransferFunctionEditor::OnColorWindowChanged()
{
  auto node = m_DataNode.Lock();

  if (m_AppliedTransferFunction.IsNull() || node.IsNull() || m_BaseColorFn == nullptr)
    return;

  if (m_DataRange[1] <= m_DataRange[0]) // no valid histogram range to span
    return;

  auto colorTable = mitk::ResampleColorWindow(m_BaseColorFn, m_DataRange[0], m_DataRange[1],
    m_Controls->colorShiftSlider->value(), m_Controls->colorWidthSlider->value());

  if (colorTable.empty())
    return;

  // Sorts the function once for the whole table, where handing the nodes over
  // one at a time re-sorted it on every insert. Clears what was there first, so
  // the table stands for the curve entire.
  m_AppliedTransferFunction->GetColorTransferFunction()->BuildFunctionFromTable(
    m_DataRange[0], m_DataRange[1], static_cast<int>(colorTable.size() / 3), colorTable.data());

  this->RecordAdjustOffsets();
  this->ShowPresetEdited();
  m_Controls->combinedTfCanvas->update();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::OnCanvasOpacityChanged()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  // Nothing to notify here: the canvas edited the scalar opacity function in
  // place, and what the ray caster re-uploads against is that function's own
  // modification time, which the edit already moved.
  this->RecordAdjustOffsets();
  this->ShowPresetEdited();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::OnResetAdjustments()
{
  if (m_AppliedTransferFunction.IsNull())
    return;

  // Reset returns the four offsets to neutral rather than reloading the preset.
  // The sliders are measured from whatever baseline the editor holds - a pristine
  // preset, a curve drawn over it, or one whose preset was removed - so neutral
  // restores that baseline either way and needs no preset to be named.
  //
  // Driven through the sliders rather than by rebuilding the function, so that
  // the canvas and the colour window follow and the handles end up where the
  // curve says they are. ResetAdjustSliders suppresses exactly these signals, by
  // design, which is why it cannot stand in here.
  m_Controls->opacityShiftSlider->setValue(0.0);
  m_Controls->opacityHeightSlider->setValue(0.0);
  m_Controls->colorShiftSlider->setValue(0.0);
  m_Controls->colorWidthSlider->setValue(this->NeutralColorWidth());
}

void QmitkVolumeTransferFunctionEditor::SetEditModeActive(bool active)
{
  // SetDataNode ends editing on every selection change, whether or not any was
  // under way. Everything below is a transition, so a request for the mode
  // already in force stops here rather than re-running it.
  if (active == m_EditModeActive)
    return;

  auto node = m_DataNode.Lock();

  if (active && (node.IsNull() || m_AppliedTransferFunction.IsNull() || m_BaseColorFn == nullptr))
  {
    // Nothing to put handles on. The button that asked has already gone down,
    // so it is put back where the state says it belongs.
    this->ShowEditMode();
    return;
  }

  m_EditModeActive = active;

  if (active)
  {
    m_CurveEdited = false;
    m_ColorHandlesRestored = false;

    // Nothing about the function changed, only what may now be done to it -
    // which is why this is not ShowAppliedTransferFunction: re-seeding here
    // would take the colour baseline from an already windowed function and
    // snap all four sliders to neutral for an edit not yet made.
    this->ShowEditMode();

    return;
  }

  this->ShowEditMode();

  // An untouched curve is still the preset it came from, and re-seeding for it
  // would cost the sliders their baseline for nothing.
  if (!m_CurveEdited)
    return;

  // Nothing about the drawing is recorded on the node: it goes on naming the
  // preset that was drawn over and the offsets in force, and rebuilding those is
  // what the next selection does - a drawing does not outlive it, and keeping
  // one means saving it as a preset of its own. Carried here instead, so that
  // the panel can go on saying the curve is not the preset after the re-seed
  // below has cleared m_CurveEdited.
  m_CurveDrawnOver = true;

  // Re-seeds the canvas and re-snapshots both baselines, so the sliders now
  // measure from the curve that was drawn rather than from the one it started
  // out as. Both are derived from their baseline rather than nudged, so without
  // this the first shift would rebuild the preset over the drawing.
  this->ApplyCurrentTransferFunction();
}

void QmitkVolumeTransferFunctionEditor::ShowEditMode()
{
  // A colour window bakes the function into hundreds of evenly spaced points,
  // which no one can take hold of. The stops are on show for as long as editing
  // is, rather than only while the colours are being asked for, so bringing them
  // back is part of entering it.
  if (m_EditModeActive)
    this->RestoreColorHandles();

  m_Controls->combinedTfCanvas->SetEditable(m_EditModeActive);

  {
    // The button is both what asks for the mode and what reports it, so letting
    // this through would come straight back as a request to change it.
    const QSignalBlocker blocker(m_Controls->editModeButton);
    m_Controls->editModeButton->setChecked(m_EditModeActive);
  }

  m_Controls->canvasHintLabel->setVisible(m_EditModeActive);
  m_Controls->colorStopPanel->setVisible(m_EditModeActive);
  m_Controls->fitAxisCheckBox->setVisible(m_EditModeActive);

  {
    // Each visit starts on the band however the last one was left: the axis
    // returns to it on the way out, so a box that remembered otherwise would
    // describe a view that is not there.
    const QSignalBlocker blocker(m_Controls->fitAxisCheckBox);
    m_Controls->fitAxisCheckBox->setChecked(true);
  }

  // The wider axis is an aid to editing, so it comes and goes with editing
  // rather than outlasting the control that explains it.
  this->ApplyAxisRange();

  // A context menu would name these; buttons cannot, and a gesture nothing
  // mentions is one nobody finds. Both functions answer at once now, so what has
  // to be said is which part of the canvas belongs to which.
  m_Controls->canvasHintLabel->setText(
    "<small>On the curve: left-click adds a point, drag moves it, right-click removes it. "
    "The markers along the bottom carry the colours: click one to select it, "
    "double-click to recolour it.</small>");

  this->ShowColorStops();

  // Which controls would replace the curve being edited depends on the mode
  // this just changed.
  this->UpdateControlAvailability();
}

void QmitkVolumeTransferFunctionEditor::ApplyAxisRange()
{
  // Nothing measured, so there is no axis to scale and nothing on the canvas to
  // scale it for.
  if (m_DataRange[1] <= m_DataRange[0])
    return;

  double lower = m_DataRange[0];
  double upper = m_DataRange[1];

  // Cleared, so the axis is let out to wherever the curve reaches; ticked, it
  // keeps to the band. Only while the curve is being edited: off the axis is
  // only a problem for what has to be taken hold of, and the band is the more
  // honest picture of where the image is.
  const bool wholeRange = m_EditModeActive &&
                          !m_Controls->fitAxisCheckBox->isChecked() &&
                          m_AppliedTransferFunction.IsNotNull();

  if (wholeRange)
  {
    // A function carrying no points answers 0 to both ends, which is not a range
    // to make room for.
    const auto widen = [&lower, &upper](const double *range)
    {
      if (range[0] < range[1])
      {
        lower = std::min(lower, range[0]);
        upper = std::max(upper, range[1]);
      }
    };

    widen(m_AppliedTransferFunction->GetColorTransferFunction()->GetRange());
    widen(m_AppliedTransferFunction->GetScalarOpacityFunction()->GetRange());
  }

  // SetMin/SetMax rather than the display bounds alone: the canvas clamps a drag
  // to the data range, so a point is only movable once that range reaches it.
  m_Controls->combinedTfCanvas->SetMin(lower);
  m_Controls->combinedTfCanvas->SetMax(upper);
  m_Controls->combinedTfCanvas->update();

  // Which stops the axis reaches decides what the offsets read, and whether a
  // marker stands on a value or on an edge.
  this->ShowColorStops();
}

void QmitkVolumeTransferFunctionEditor::ShowColorStops()
{
  auto *canvas = m_Controls->combinedTfCanvas;

  const int count = canvas->GetColorStopCount();
  const int selected = canvas->GetSelectedColorStop();

  // Both of these are what ask for a change as well as what report one, so
  // filling them in from the canvas would come straight back as a request to
  // change the canvas.
  const QSignalBlocker stopBlocker(m_Controls->colorStopComboBox);
  const QSignalBlocker offsetBlocker(m_Controls->colorStopOffsetSpinBox);

  // Rebuilt only when stops came or went: dragging one would otherwise empty and
  // refill the list on every mouse move. Entries carry a swatch and nothing
  // else, and stand in the order the stops do, which is what says which marker
  // each one is.
  if (m_Controls->colorStopComboBox->count() != count)
  {
    m_Controls->colorStopComboBox->clear();

    for (int i = 0; i < count; ++i)
      m_Controls->colorStopComboBox->addItem(QString());
  }

  for (int i = 0; i < count; ++i)
    m_Controls->colorStopComboBox->setItemIcon(i, ColorSwatch(canvas->GetColorStopColor(i)));

  m_Controls->colorStopComboBox->setCurrentIndex(selected);

  const bool hasSelection = selected != -1;

  m_Controls->colorStopComboBox->setEnabled(count > 0);
  m_Controls->colorStopColorButton->setEnabled(hasSelection);
  m_Controls->colorStopOffsetSpinBox->setEnabled(hasSelection);
  m_Controls->addColorStopButton->setEnabled(count > 0);

  // A gradient has to keep a colour to be a gradient at all.
  m_Controls->removeColorStopButton->setEnabled(hasSelection && count > 1);

  // The button is the colour rather than a control that names one, so with
  // nothing selected it has nothing to show and goes back to being a button.
  m_Controls->colorStopColorButton->setStyleSheet(hasSelection
    ? "background-color:" + canvas->GetColorStopColor(selected).name()
    : QString());

  m_Controls->colorStopOffsetSpinBox->setValue(
    hasSelection ? canvas->GetSelectedColorStopOffset() : 0.0);
}

void QmitkVolumeTransferFunctionEditor::ShowPresetEdited()
{
  auto *presetList = m_Controls->presetListWidget;

  const int editedRow = this->DiffersFromPreset() ? presetList->currentRow() : -1;

  // Every entry rather than the one row that can wear the marker, so that the
  // entry the selection has just left is not left wearing it too.
  for (int i = 0; i < this->PresetCount(); ++i)
  {
    auto *presetItem = presetList->item(i);

    const QString text =
      i == editedRow ? PresetName(presetItem) + PresetEditedMarker() : PresetName(presetItem);

    // An entry tells the view it changed whether or not it did, and this runs
    // on every step of a slider drag.
    if (presetItem->text() == text)
      continue;

    presetItem->setText(text);

    // A marker nothing names is one nobody can read, and the way back is not
    // Reset: that measures from the curve as it now stands.
    presetItem->setToolTip(i == editedRow
      ? "Moved away from this preset. Click it to go back to it."
      : QString());
  }
}

bool QmitkVolumeTransferFunctionEditor::DiffersFromPreset() const
{
  // The edit in progress, the one already left - which only this widget knows
  // about, a drawing being recorded nowhere - and a curve that answers to no
  // preset, which the node carries.
  if (m_CurveEdited || m_CurveDrawnOver ||
      IsCustomTransferFunction(m_DataNode.Lock().GetPointer()))
  {
    return true;
  }

  return DiffersFrom(m_Controls->opacityShiftSlider, 0.0) ||
         DiffersFrom(m_Controls->opacityHeightSlider, 0.0) ||
         DiffersFrom(m_Controls->colorShiftSlider, 0.0) ||
         DiffersFrom(m_Controls->colorWidthSlider, this->NeutralColorWidth());
}

void QmitkVolumeTransferFunctionEditor::OnPickColorStopColor()
{
  auto *canvas = m_Controls->combinedTfCanvas;

  const int selected = canvas->GetSelectedColorStop();

  if (selected == -1)
    return;

  const auto picked = QColorDialog::getColor(canvas->GetColorStopColor(selected), this);

  if (picked.isValid())
    canvas->SetSelectedColorStopColor(picked);
}

void QmitkVolumeTransferFunctionEditor::OnAddColorStop()
{
  auto *canvas = m_Controls->combinedTfCanvas;

  const int count = canvas->GetColorStopCount();

  if (count == 0)
    return;

  // The middle of the widest gap: where a stop is most use, and the one place it
  // cannot land on top of one that is already there. The stretches between the
  // outermost stops and the ends of the range count as gaps too, so that the
  // flat ends of the gradient can be reached as well.
  double where = 0.0;
  double widest = -1.0;

  const auto consider = [&where, &widest](double from, double to)
  {
    if (to - from > widest)
    {
      widest = to - from;
      where = 0.5 * (from + to);
    }
  };

  consider(m_DataRange[0], canvas->GetColorStopValue(0));

  for (int i = 0; i < count - 1; ++i)
    consider(canvas->GetColorStopValue(i), canvas->GetColorStopValue(i + 1));

  consider(canvas->GetColorStopValue(count - 1), m_DataRange[1]);

  canvas->AddColorStop(where);
}

vtkSmartPointer<vtkColorTransferFunction> QmitkVolumeTransferFunctionEditor::WindowedColorHandles() const
{
  auto handles = vtkSmartPointer<vtkColorTransferFunction>::New();

  // Brings the colour space and the clamping over along with the points, which
  // the window has to stay faithful to.
  handles->DeepCopy(m_BaseColorFn);

  mitk::ApplyColorWindow(handles, m_DataRange[0], m_DataRange[1],
    m_Controls->colorShiftSlider->value(), m_Controls->colorWidthSlider->value());

  return handles;
}

void QmitkVolumeTransferFunctionEditor::RestoreColorHandles()
{
  if (m_ColorHandlesRestored || m_AppliedTransferFunction.IsNull() || m_BaseColorFn == nullptr)
    return;

  // A colour window bakes itself into 256 evenly spaced RGB points, which no
  // one can take hold of. Laying the handles over them puts the same colours
  // back on the baseline's own nodes, so they become editable without moving.
  // Copied into the function the node already carries rather than put in its
  // place, so that nothing holding a pointer to it has to be told.
  m_AppliedTransferFunction->GetColorTransferFunction()->DeepCopy(this->WindowedColorHandles());

  m_ColorHandlesRestored = true;

  m_Controls->combinedTfCanvas->update();

  emit TransferFunctionChanged();
}

void QmitkVolumeTransferFunctionEditor::LoadPreset()
{
  if (m_DataNode.Lock().IsNull())
    return;

  const QString title = QStringLiteral("Load preset");

  // Beside the last preset saved or loaded, which is where the next one is most
  // likely to be.
  const QStringList rememberedFiles = RememberedPresetFiles();

  const QString directory = rememberedFiles.isEmpty()
    ? QString()
    : QFileInfo(rememberedFiles.last()).absolutePath();

  const auto fileName = QFileDialog::getOpenFileName(this, title, directory, "Transfer function (*.json)");

  if (fileName.isEmpty() || !ValidatePresetFilePath(this, title, fileName))
    return;

  // Absolute, as the remembered files are, so that a file already offered is
  // recognised however the dialog spelled it.
  const QString presetFile = QFileInfo(fileName).absoluteFilePath();

  auto *presetList = m_Controls->presetListWidget;

  // Applied rather than read a second time, which would offer the same file
  // twice, the second time under a numbered name.
  if (auto *knownItem = FindPresetFileItem(presetList, presetFile); knownItem != nullptr)
  {
    presetList->setCurrentItem(knownItem);
    presetList->scrollToItem(knownItem);

    this->OnPresetSelected(PresetName(knownItem));
    return;
  }

  if (!this->AddPresetFromFile(presetFile))
    QMessageBox::warning(this, title, "The file could not be read, or holds no transfer function preset.");
}

void QmitkVolumeTransferFunctionEditor::OnPresetContextMenu(const QPoint &pos)
{
  QMenu menu;

  // A greyed entry says only that it is greyed, and what it would take to reach
  // it is the whole of what there is to explain.
  menu.setToolTipsVisible(true);

  auto *saveAction = menu.addAction("Save as preset...");

  // The condition that marks an entry as edited, so that the menu and the marker
  // cannot disagree. It asks about the curve in force rather than the entry
  // under the cursor, which is what lets a curve whose preset was removed -
  // marking no entry at all - be saved as well.
  saveAction->setEnabled(m_AppliedTransferFunction.IsNotNull() && this->DiffersFromPreset());
  saveAction->setToolTip("Available once the curve has been moved away from its preset.");

  // The entry under the cursor rather than the one in force, which is what
  // saving asks about: what is removed is a preset, not the curve on show.
  auto *presetItem = m_Controls->presetListWidget->itemAt(pos);

  // Stands for no preset, and would otherwise read as one of the catalogue's.
  if (presetItem == m_LoadPresetItem)
    presetItem = nullptr;

  auto *removeAction = menu.addAction("Remove preset");

  removeAction->setEnabled(presetItem != nullptr &&
                           PresetOriginOf(presetItem) == PresetOrigin::File);
  removeAction->setToolTip("Right-click a preset saved from here. The catalogue's own cannot be removed.");

  // The position arrives relative to the viewport rather than to the widget.
  auto *chosenAction = menu.exec(m_Controls->presetListWidget->viewport()->mapToGlobal(pos));

  if (chosenAction == saveAction)
    this->SaveCustomPreset();
  else if (chosenAction == removeAction)
    this->RemoveCustomPreset(presetItem);
}

void QmitkVolumeTransferFunctionEditor::SaveCustomPreset()
{
  auto node = m_DataNode.Lock();

  if (node.IsNull() || m_AppliedTransferFunction.IsNull() || m_BaseColorFn == nullptr)
    return;

  auto *presetList = m_Controls->presetListWidget;
  auto *currentPreset = presetList->currentItem();

  // The file name becomes the entry's name, so the suggestion starts from the
  // preset the curve was carried away from, beside the last preset saved.
  const QString suggestedName =
    (currentPreset != nullptr ? PresetName(currentPreset) : QStringLiteral("transfer-function")) +
    QStringLiteral("-custom.json");

  const QStringList rememberedFiles = RememberedPresetFiles();

  const QString suggestion = rememberedFiles.isEmpty()
    ? suggestedName
    : QFileInfo(rememberedFiles.last()).absolutePath() + QLatin1Char('/') + suggestedName;

  auto fileName = QFileDialog::getSaveFileName(this, "Save transfer function", suggestion,
    "Transfer function (*.json)");

  if (fileName.isEmpty())
    return;

  if (!fileName.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive))
    fileName += QStringLiteral(".json");

  if (!ValidatePresetFilePath(this, "Save transfer function", fileName))
    return;

  // A copy, so that saving cannot alter the curve it is saving, and with the
  // colours as handles: a windowed function holds 256 evenly spaced samples, and
  // a preset made of those is one nobody could take hold of again. The opacity
  // curve needs no such care - nothing bakes that one into a table.
  auto savedFunction = mitk::TransferFunction::New();

  savedFunction->SetColorSpace(m_AppliedTransferFunction->GetColorSpace());
  savedFunction->SetColorTransferFunction(this->WindowedColorHandles());
  savedFunction->GetScalarOpacityFunction()->DeepCopy(
    m_AppliedTransferFunction->GetScalarOpacityFunction());

  const auto blendMode =
    mitk::GetVolumeBlendMode(node.GetPointer()).value_or(mitk::VolumeBlendMode::Composite);

  const QString presetName = QFileInfo(fileName).completeBaseName();

  {
    // Scoped, because a stream closes with its destructor and the file has to be
    // closed before it is read back below.
    std::ofstream stream(fileName.toStdString());

    if (!stream.is_open() ||
        !mitk::TransferFunctionPresets::SaveTransferFunction(stream, presetName.toStdString(),
          savedFunction.GetPointer(), blendMode))
    {
      QMessageBox::warning(this, "Save transfer function", "Could not write the file.");
      return;
    }
  }

  // Absolute, because the directory this was launched from is not the one it
  // will be launched from next time.
  const QString presetFile = QFileInfo(fileName).absoluteFilePath();

  // Saving over a file already offered replaces its entry: the file now holds
  // only the curve just written. Dropped before the file is read back, so that
  // the name it frees is the one the new entry gets rather than a numbered one.
  if (auto *knownItem = FindPresetFileItem(presetList, presetFile); knownItem != nullptr)
    this->DropPresetEntry(knownItem);

  if (!this->AddPresetFromFile(presetFile))
  {
    QMessageBox::warning(this, "Save transfer function",
      "The file was written, but could not be read back as a preset.");
  }
}

bool QmitkVolumeTransferFunctionEditor::AddPresetFromFile(const QString &presetFile)
{
  auto *presetList = m_Controls->presetListWidget;

  // Read from the file rather than added from memory, so that a preset added
  // now and one found at the next start arrive by the same route.
  std::ifstream stream(presetFile.toStdString());

  const auto addedName = stream.is_open()
    ? QString::fromStdString(m_Presets.AddPreset(stream))
    : QString();

  if (addedName.isEmpty())
    return false;

  // Asked before the entry is inserted, since inserting moves the end the
  // previews were drawn as far as.
  const bool thumbnailsComplete = m_NextThumbnailIndex >= this->PresetCount();

  auto *presetItem = new QListWidgetItem(addedName);
  presetItem->setData(PRESET_NAME_ROLE, addedName);
  presetItem->setData(PRESET_ORIGIN_ROLE, static_cast<int>(PresetOrigin::File));
  presetItem->setData(PRESET_FILE_ROLE, presetFile);

  // In front of the load entry, which stays last.
  presetList->insertItem(this->PresetCount(), presetItem);

  // Measures the cells against a name none of them was measured for, and leaves
  // the new entry a stand-in preview to hold its place.
  this->UpdatePresetLayout();

  // A finished run has stopped and nothing else would start it again; one still
  // under way reaches the new entry by itself. Either way the next preview due
  // is the one just inserted, so a single step draws it and stops.
  if (thumbnailsComplete)
  {
    const int run = m_ThumbnailRun;
    QTimer::singleShot(0, this, [this, run] { this->GenerateNextThumbnail(run); });
  }

  // A file already remembered moves to the end rather than being remembered
  // twice, which would offer it twice at the next start.
  QStringList presetFiles = RememberedPresetFiles();

  presetFiles.removeAll(presetFile);
  presetFiles.append(presetFile);

  RememberPresetFiles(presetFiles);

  // The curve now answers to a catalogue entry, so that entry is marked, and
  // for a curve saved from here the edited marker goes.
  presetList->setCurrentItem(presetItem);

  this->OnPresetSelected(addedName);

  return true;
}

void QmitkVolumeTransferFunctionEditor::RemoveCustomPreset(QListWidgetItem *presetItem)
{
  if (presetItem == nullptr || PresetOriginOf(presetItem) != PresetOrigin::File)
    return;

  // Both read while the entry is still there to read them from.
  const QString presetName = PresetName(presetItem);
  const QString presetFile = PresetFile(presetItem);

  QMessageBox confirmation(QMessageBox::Question, "Remove preset",
    QString("Remove \"%1\" from the presets?").arg(presetName), QMessageBox::Cancel, this);

  // Which file rather than only that there is one: the preset is that file, and
  // this is the last place that names it.
  confirmation.setInformativeText(
    QString("The file it was saved to is kept:\n%1").arg(presetFile));

  auto *removeButton = confirmation.addButton("Remove", QMessageBox::AcceptRole);

  // Asked at all because the grid is clicked in to apply a preset, over and
  // over, and a menu entry a hand's breadth from that click is not where a
  // preset should be lost. Defaulting to Cancel is the rest of the same thought.
  confirmation.setDefaultButton(QMessageBox::Cancel);
  confirmation.exec();

  if (confirmation.clickedButton() != removeButton)
    return;

  this->DropPresetEntry(presetItem);

  // The file is left where the user put it: it may be one they keep elsewhere
  // or share. What is dropped is the promise to look for it again.
  QStringList presetFiles = RememberedPresetFiles();

  presetFiles.removeAll(presetFile);

  RememberPresetFiles(presetFiles);
}

void QmitkVolumeTransferFunctionEditor::DropPresetEntry(QListWidgetItem *presetItem)
{
  auto *presetList = m_Controls->presetListWidget;

  const QString presetName = PresetName(presetItem);
  const int presetRow = presetList->row(presetItem);

  // The curve on show is left as it stands - what goes is the entry it answers
  // to, not the rendering. But a node naming a preset the catalogue no longer
  // holds has nothing to be rebuilt from, so it is recorded as carrying a curve
  // no preset describes.
  if (presetRow == presetList->currentRow())
  {
    auto node = m_DataNode.Lock();

    if (node.IsNotNull())
      this->RecordCustomTransferFunction(node);

    this->ClearPresetSelection();
  }

  m_Presets.RemovePreset(presetName.toStdString());

  // A preview still due stands one place further back once this entry is out of
  // the way. Without this the entry that takes its index is stepped over, and
  // keeps its stand-in for as long as the panel is open.
  if (presetRow < m_NextThumbnailIndex)
    --m_NextThumbnailIndex;

  // Taking an entry out of a list widget hands its ownership back, and nothing
  // else frees it.
  delete presetList->takeItem(presetRow);

  // The cells may have been measured against the name just taken out.
  this->UpdatePresetLayout();
}

int QmitkVolumeTransferFunctionEditor::PresetCount() const
{
  return m_Controls->presetListWidget->count() - 1;
}

void QmitkVolumeTransferFunctionEditor::InvalidateThumbnails()
{
  // Bumping the version is what abandons a generation already under way: its
  // next step finds the number changed and stops without queuing another.
  ++m_ThumbnailRun;

  // Before zero, because the volume has to be bound before anything is drawn,
  // and that is a step of its own.
  m_NextThumbnailIndex = -1;
  m_ThumbnailImage = nullptr;

  // The renderer keeps the volume it has bound alive, and nothing here applies
  // to it any more - including the case where the node it came with has just
  // been removed, which nothing else would free it on.
  m_ThumbnailRenderer->SetImage(nullptr);

  // Back to the stand-in rather than to nothing, so that clearing the previews
  // does not resize every entry and scatter the grid.
  this->RefreshPlaceholders();
}

void QmitkVolumeTransferFunctionEditor::RefreshPlaceholders()
{
  auto *presetList = m_Controls->presetListWidget;

  const QColor iconColor(QmitkStyleManager::GetIconColor());
  const QIcon placeholder = PlaceholderPreview(presetList->iconSize(), iconColor);

  // Previews are filled in one after another from the front, so the index of
  // the next one is also the first entry still showing a stand-in. Below zero
  // no volume is bound yet and every entry is one.
  for (int i = std::max(0, m_NextThumbnailIndex); i < this->PresetCount(); ++i)
    presetList->item(i)->setIcon(placeholder);

  m_LoadPresetItem->setIcon(LoadPresetIcon(presetList->iconSize(), iconColor));
}

void QmitkVolumeTransferFunctionEditor::StartThumbnailGeneration()
{
  auto node = m_DataNode.Lock();
  auto *image = node.IsNotNull() ? node->GetDataAs<mitk::Image>() : nullptr;

  if (image == nullptr)
    return;

  // Either the previews already describe this image or they are being drawn
  // for it; neither wants starting again.
  if (m_ThumbnailImage == image)
    return;

  // Whether previews can be drawn is asked of every image rather than once of
  // the machine: the ray caster refuses some volumes it is handed, RGB ones
  // among them, and one such refusal must not write off the images after it.
  // SetImage reports its own refusal, and GenerateNextThumbnail acts on it.
  this->InvalidateThumbnails();
  m_ThumbnailImage = image;

  const int run = m_ThumbnailRun;
  QTimer::singleShot(0, this, [this, run] { this->GenerateNextThumbnail(run); });
}

void QmitkVolumeTransferFunctionEditor::GenerateNextThumbnail(int run)
{
  if (run != m_ThumbnailRun)
    return;

  // Asked here rather than where generation is requested: a selection change
  // binds the node first and settles whether the editor applies to it second,
  // so only a turn of the event loop later is the answer the current one.
  if (!this->isEnabled())
  {
    this->InvalidateThumbnails();
    return;
  }

  auto *presetList = m_Controls->presetListWidget;

  if (m_NextThumbnailIndex < 0)
  {
    // Uploading the volume costs far more than drawing from it, so it gets a
    // turn of the event loop to itself rather than holding up the selection
    // change that asked for it.
    if (!m_ThumbnailRenderer->SetImage(m_ThumbnailImage.Lock().GetPointer()))
    {
      // Clearing the bound image matters: it is what lets a later attempt
      // start, rather than reading as a generation already finished.
      this->InvalidateThumbnails();
      return;
    }

    m_NextThumbnailIndex = 0;
  }
  else
  {
    auto *presetItem = presetList->item(m_NextThumbnailIndex);
    const auto presetName = PresetName(presetItem).toStdString();

    // Drawn in the mode the preset names, or the two MIP presets would preview
    // as the white shells their windows composite into, and the grid would
    // misrepresent exactly the entries hardest to picture.
    auto blendMode = mitk::VolumeBlendMode::Composite;

    if (auto transferFunction = m_Presets.CreateTransferFunction(presetName, blendMode);
        transferFunction.IsNotNull())
    {
      const auto thumbnail = m_ThumbnailRenderer->Render(transferFunction.GetPointer(), blendMode);

      if (!thumbnail.isNull())
        presetItem->setIcon(QIcon(thumbnail));
    }

    ++m_NextThumbnailIndex;
  }

  if (m_NextThumbnailIndex >= this->PresetCount())
    return;

  // Queued rather than looped: returning to the event loop between previews is
  // what keeps the panel responsive and lets the grid fill in while it is open.
  QTimer::singleShot(0, this, [this, run] { this->GenerateNextThumbnail(run); });
}
