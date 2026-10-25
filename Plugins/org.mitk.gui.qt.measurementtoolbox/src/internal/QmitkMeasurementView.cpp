/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMeasurementView.h"
#include "ui_QmitkMeasurementViewControls.h"

#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QColorDialog>
#include <QHeaderView>
#include <QPainter>
#include <QShortcut>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>

#include <mitkException.h>
#include <mitkExclusiveInteraction.h>
#include <mitkIPreferences.h>
#include <mitkIRenderWindowPart.h>
#include <mitkITKEventObserverGuard.h>
#include <mitkImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarAngle.h>
#include <mitkPlanarArrow.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarCross.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlanarFigureVtkMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPath.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarSubdivisionPolygon.h>
#include <mitkRenderWindowPartHelper.h>
#include <mitkRenderingManager.h>

#include <QmitkButtonOverlayWidget.h>
#include <QmitkIconTheme.h>
#include <QmitkNodeDescriptorManager.h>
#include <QmitkRenderWindow.h>

#include <usModuleRegistry.h>

#include <algorithm>
#include <array>
#include <map>
#include <cmath>
#include <optional>
#include <set>

namespace
{
  enum class ToolGroup
  {
    Distances,
    Angles,
    Areas
  };

  enum class FixedSize
  {
    None,
    Radius,
    RadiusAndThickness
  };

  struct Tool
  {
    const char* Type; /**< Class name of the figure, also names its icon. */
    const char* Name; /**< Display name, also the base of node names. */
    ToolGroup Group;
    FixedSize Size;
    mitk::PlanarFigure::Pointer (*Create)();
  };

  template <class T>
  mitk::PlanarFigure::Pointer CreateFigure()
  {
    return T::New().GetPointer();
  }

  const std::array<Tool, 13> Tools = {{
    { "PlanarLine", "Line", ToolGroup::Distances, FixedSize::None, &CreateFigure<mitk::PlanarLine> },
    { "PlanarCross", "Cross", ToolGroup::Distances, FixedSize::None, &CreateFigure<mitk::PlanarCross> },
    { "PlanarPath", "Path", ToolGroup::Distances, FixedSize::None, &CreateFigure<mitk::PlanarPath> },
    { "PlanarBezierCurve", "Bezier Curve", ToolGroup::Distances, FixedSize::None, &CreateFigure<mitk::PlanarBezierCurve> },
    { "PlanarArrow", "Arrow", ToolGroup::Distances, FixedSize::None, &CreateFigure<mitk::PlanarArrow> },
    { "PlanarAngle", "Angle", ToolGroup::Angles, FixedSize::None, &CreateFigure<mitk::PlanarAngle> },
    { "PlanarFourPointAngle", "Four-Point Angle", ToolGroup::Angles, FixedSize::None, &CreateFigure<mitk::PlanarFourPointAngle> },
    { "PlanarRectangle", "Rectangle", ToolGroup::Areas, FixedSize::None, &CreateFigure<mitk::PlanarRectangle> },
    { "PlanarCircle", "Circle", ToolGroup::Areas, FixedSize::Radius, &CreateFigure<mitk::PlanarCircle> },
    { "PlanarEllipse", "Ellipse", ToolGroup::Areas, FixedSize::None, &CreateFigure<mitk::PlanarEllipse> },
    { "PlanarDoubleEllipse", "Ring", ToolGroup::Areas, FixedSize::RadiusAndThickness, &CreateFigure<mitk::PlanarDoubleEllipse> },
    { "PlanarPolygon", "Polygon", ToolGroup::Areas, FixedSize::None, &CreateFigure<mitk::PlanarPolygon> },
    { "PlanarSubdivisionPolygon", "Subdivision Polygon", ToolGroup::Areas, FixedSize::None, &CreateFigure<mitk::PlanarSubdivisionPolygon> },
  }};

  const std::string ColorPreference = "color";
  const std::string LineWidthPreference = "line width";
  const std::string ShowNamePreference = "show name";
  const std::string ShowMeasurementsPreference = "show measurements";
  const std::string ContrastOutlinePreference = "contrast outline";
  const std::string LabelVisibilityPreference = "label visibility";
  const std::string ShowIn3DPreference = "show in 3d";

  /** The line width slider moves in steps of half a pixel. */
  constexpr float LineWidthStep = 0.5f;

  /** Sets the text of a tree cell only when it changes, to not repaint for nothing. */
  void SetText(QTreeWidgetItem* item, int column, const QString& text)
  {
    if (item->text(column) != text)
      item->setText(column, text);
  }

  /** The node of a row of the measurement tree; only compared, never dereferenced, since it may be gone. */
  const mitk::DataNode* GetNode(const QTreeWidgetItem* item)
  {
    const auto* figureItem = nullptr != item->parent() ? item->parent() : item;
    return reinterpret_cast<const mitk::DataNode*>(figureItem->data(0, Qt::UserRole).value<quintptr>());
  }

  /** Quotes a field of comma-separated values if it contains a separator, a quote or a line break (RFC 4180). */
  QString ToCsvField(QString text)
  {
    if (!text.contains(QLatin1Char(',')) && !text.contains(QLatin1Char('"')) && !text.contains(QLatin1Char('\n')) && !text.contains(QLatin1Char('\r')))
      return text;

    return QLatin1Char('"') + text.replace(QLatin1String("\""), QLatin1String("\"\"")) + QLatin1Char('"');
  }

  /** All render windows of a part share a single interaction reference geometry. */
  mitk::TimeGeometry::ConstPointer GetInteractionReferenceGeometry(const mitk::IRenderWindowPart* renderWindowPart)
  {
    const auto renderWindows = renderWindowPart->GetQmitkRenderWindows();

    for (auto* renderWindow : renderWindows)
    {
      if (auto* renderer = renderWindow->GetRenderer(); nullptr != renderer)
        return renderer->GetInteractionReferenceGeometry();
    }

    return nullptr;
  }

  /** Gives a figure placed by a span the size of the given radius and locks its shape, so that it is placed with a click. */
  void MakePreset(mitk::PlanarFigure* figure, double radius)
  {
    mitk::Point2D center;
    center.Fill(0.0);

    auto rim = center;
    rim[0] = radius;

    figure->BeginPlacement();
    figure->SetFromSpan(center, rim);
    figure->SetShapeLocked(true);
  }

  QString GetPlacementHint(const mitk::PlanarFigure* figure)
  {
    const auto numberOfPoints = figure->GetNumberOfControlPoints();

    if (figure->IsShapeLocked() && numberOfPoints > 0)
      return QStringLiteral("Click to place a figure of the fixed size. The tool stays armed for more. Press Esc to stop.");

    if (mitk::PlanarFigure::PlacementKind::Span == figure->GetPlacementKind())
    {
      return 0 == numberOfPoints
        ? QStringLiteral("Press and drag, or click twice, to place the figure. Press Esc to cancel.")
        : QStringLiteral("Release, or click a second time, to finish the figure. Press Esc to cancel.");
    }

    // The last point follows the mouse until it is placed as well.
    const auto numberOfPlacedPoints = numberOfPoints > 0 ? numberOfPoints - 1 : 0;
    const auto minimum = figure->GetMinimumNumberOfControlPoints();
    const auto maximum = figure->GetMaximumNumberOfControlPoints();

    QString hint;

    if (minimum == maximum)
    {
      hint = QStringLiteral("Click to place point %1 of %2.").arg(numberOfPlacedPoints + 1).arg(maximum);
    }
    else if (numberOfPlacedPoints < minimum)
    {
      hint = QStringLiteral("Click to place points, at least %1.").arg(minimum);
    }
    else
    {
      hint = QStringLiteral("Click to add points. Double-click to add a last one and finish, or press Enter to finish.");
    }

    if (numberOfPlacedPoints > 0)
      hint += QStringLiteral(" Backspace removes the last point.");

    return hint + QStringLiteral(" Press Esc to cancel.");
  }

  QString GetEditingHint(const mitk::PlanarFigure* figure)
  {
    if (figure->IsShapeLocked())
      return QStringLiteral("The shape of this figure is fixed. Drag it to move it.");

    auto hint = QStringLiteral("Drag a point to reshape the figure, or its outline to move it.");

    if (figure->CanInsertControlPoints())
      hint += QStringLiteral(" Ctrl+click on an edge to add a point, or on a point to remove it.");

    return hint;
  }
}

/** A figure that can be edited in this view. */
struct QmitkMeasurementFigure
{
  mitk::DataNode::Pointer Node;
  mitk::ITKEventObserverGuard EndPlacementObserver;
  mitk::ITKEventObserverGuard SelectObserver;
  mitk::ITKEventObserverGuard PointMovedObserver;
};

struct QmitkMeasurementViewData
{
  std::unique_ptr<Ui::QmitkMeasurementViewControls> m_Controls = std::make_unique<Ui::QmitkMeasurementViewControls>();
  QButtonGroup* m_ToolButtons = nullptr;
  QShortcut* m_CancelPlacementShortcut = nullptr;
  QmitkButtonOverlayWidget* m_AlignmentOverlay = nullptr;
  mitk::IRenderWindowPart* m_RenderWindowPart = nullptr;

  QColor m_FigureColor;

  QList<mitk::DataNode::Pointer> m_CurrentSelection;
  std::map<const mitk::DataNode*, QmitkMeasurementFigure> m_Figures;
  mitk::DataNode::Pointer m_SelectedImageNode;

  /** The node of the figure that is being placed, and the index of its tool. */
  mitk::DataNode::Pointer m_PendingNode;
  std::optional<int> m_PendingTool;

  mitk::ITKEventObserverGuard m_ViewsInitializedObserver;

  /** Set while a figure is being placed: the interaction reference geometry to hand
      back afterwards. Holds a null geometry if there was none to begin with. */
  std::optional<mitk::TimeGeometry::ConstPointer> m_RestoreInteractionReferenceGeometry;

  /** Active while a figure is being placed. */
  mitk::ExclusiveInteraction::Claim m_ExclusiveInteractionClaim;

  /** Observes the figure that is being placed, to cancel the placement once it gets deselected. */
  mitk::ITKEventObserverGuard m_PlacementSelectionObserver;

  /** Observes the figure that is being placed, to keep the hint in step. */
  mitk::ITKEventObserverGuard m_PlacementProgressObserver;

  bool m_IsMeasurementTreeUpdateScheduled = false;
};

const std::string QmitkMeasurementView::VIEW_ID = "org.mitk.views.measurement";

QmitkMeasurementView::QmitkMeasurementView()
  : d(std::make_unique<QmitkMeasurementViewData>())
{
}

QmitkMeasurementView::~QmitkMeasurementView()
{
  // Before unhooking from all figures below: a pending placement would
  // otherwise leave its half-created node behind in the data storage.
  this->CancelPlacement();

  for (auto& [node, figure] : d->m_Figures)
    figure.Node->SetDataInteractor(nullptr);
}

void QmitkMeasurementView::CreateQtPartControl(QWidget* parent)
{
  d->m_Controls->setupUi(parent);

  auto* referenceImageSelector = d->m_Controls->referenceImageSelector;
  referenceImageSelector->SetDataStorage(this->GetDataStorage());
  referenceImageSelector->SetNodePredicate(mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::Image>::New(),
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
  referenceImageSelector->SetSelectionIsOptional(true);
  referenceImageSelector->SetEmptyInfo(QStringLiteral("Please select a reference image"));
  referenceImageSelector->SetPopUpTitel(QStringLiteral("Select a reference image"));

  this->CreateToolButtons();
  this->LoadAppearance();

  auto* header = d->m_Controls->measurementsTree->header();
  header->setStretchLastSection(false);
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

  d->m_Controls->tools->setEnabled(false);
  d->m_Controls->thicknessSpinBox->setMaximum(d->m_Controls->radiusSpinBox->value());
  d->m_Controls->hintCard->hide();
  this->UpdateToolOptions();

  // Esc cancels a pending figure placement. A render window does not have
  // keyboard focus during placement, so an application-context shortcut is
  // used instead of a key event.
  d->m_CancelPlacementShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), parent);
  d->m_CancelPlacementShortcut->setContext(Qt::ApplicationShortcut);
  d->m_CancelPlacementShortcut->setEnabled(false);

  // The alignment overlay covers exactly the drawing controls, so the reference image
  // selector and the measurements stay usable while the views are not aligned.
  d->m_AlignmentOverlay = new QmitkButtonOverlayWidget(d->m_Controls->drawingControls);
  d->m_AlignmentOverlay->setVisible(false);
  d->m_AlignmentOverlay->SetOverlayText(QStringLiteral(
    "<p style=\"color:red; text-align:center\">"
      "The views are not aligned to the reference image.<br>"
      "Align the views to draw measurements."
    "</p>"));
  d->m_AlignmentOverlay->SetButtonText(" Align views");
  d->m_AlignmentOverlay->setOpacity(200);
  d->m_AlignmentOverlay->SetButtonIcon(QmitkIconTheme::GetIcon(QLatin1String(":/Qmitk/reset.svg")));

  this->CreateConnections();
  this->AddAllInteractors();

  d->m_ViewsInitializedObserver.Reset(mitk::RenderingManager::GetInstance(), mitk::RenderingManagerViewsInitializedEvent(),
    [this](const itk::EventObject&) { this->UpdateDrawingControls(); });

  // The view coordinator notifies listeners only once an editor becomes visible.
  // Pick up an editor that is already active at this point.
  if (auto* renderWindowPart = this->GetRenderWindowPart(); nullptr != renderWindowPart)
    this->RenderWindowPartActivated(renderWindowPart);

  // placed after CreateConnections to trigger update of the current selection
  referenceImageSelector->SetAutoSelectNewNodes(true);
}

void QmitkMeasurementView::CreateToolButtons()
{
  d->m_ToolButtons = new QButtonGroup(d->m_Controls->tools);

  // Unchecking the active tool cancels its placement, which an exclusive group would not allow.
  d->m_ToolButtons->setExclusive(false);

  for (int i = 0; i < static_cast<int>(Tools.size()); ++i)
  {
    const auto& tool = Tools[i];

    auto* button = new QToolButton;
    button->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/%1.svg").arg(QLatin1String(tool.Type))));
    button->setIconSize(QSize(32, 32));
    button->setToolTip(QString::fromUtf8(tool.Name));
    button->setCheckable(true);
    button->setAutoRaise(true);

    d->m_ToolButtons->addButton(button, i);

    auto* layout = ToolGroup::Distances == tool.Group ? d->m_Controls->distanceToolsLayout
      : ToolGroup::Angles == tool.Group ? d->m_Controls->angleToolsLayout
      : d->m_Controls->areaToolsLayout;

    // Before the trailing spacer
    layout->insertWidget(layout->count() - 1, button);
  }
}

void QmitkMeasurementView::CreateConnections()
{
  connect(d->m_Controls->referenceImageSelector, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkMeasurementView::OnCurrentSelectionChanged);
  connect(d->m_ToolButtons, &QButtonGroup::idClicked, this, &QmitkMeasurementView::OnToolClicked);
  connect(d->m_CancelPlacementShortcut, &QShortcut::activated, this, &QmitkMeasurementView::CancelPlacement);
  connect(d->m_AlignmentOverlay, &QmitkButtonOverlayWidget::Clicked, this, &QmitkMeasurementView::OnAlignViewsClicked);
  connect(d->m_Controls->copyButton, &QPushButton::clicked, this, &QmitkMeasurementView::OnCopyToClipboard);
  connect(d->m_Controls->measurementsTree, &QTreeWidget::itemSelectionChanged, this, &QmitkMeasurementView::OnMeasurementTreeSelectionChanged);
  connect(d->m_Controls->measurementsTree, &QTreeWidget::currentItemChanged, this, &QmitkMeasurementView::OnMeasurementTreeItemActivated);
  connect(d->m_Controls->measurementsTree, &QTreeWidget::itemClicked, this, &QmitkMeasurementView::OnMeasurementTreeItemActivated);

  connect(d->m_Controls->radiusSpinBox, &QDoubleSpinBox::valueChanged, d->m_Controls->thicknessSpinBox, &QDoubleSpinBox::setMaximum);
  connect(d->m_Controls->fixedSizeGroupBox, &QGroupBox::toggled, this, &QmitkMeasurementView::OnFixedSizeChanged);
  connect(d->m_Controls->radiusSpinBox, &QDoubleSpinBox::valueChanged, this, &QmitkMeasurementView::OnFixedSizeChanged);
  connect(d->m_Controls->thicknessSpinBox, &QDoubleSpinBox::valueChanged, this, &QmitkMeasurementView::OnFixedSizeChanged);

  connect(d->m_Controls->colorButton, &QToolButton::clicked, this, &QmitkMeasurementView::OnColorButtonClicked);
  connect(d->m_Controls->lineWidthSlider, &QSlider::valueChanged, this, [this]() {
    this->UpdateLineWidthLabel();
    this->OnAppearanceChanged();
  });
  connect(d->m_Controls->showNameCheckBox, &QCheckBox::toggled, this, &QmitkMeasurementView::OnAppearanceChanged);
  connect(d->m_Controls->showMeasurementsCheckBox, &QCheckBox::toggled, this, &QmitkMeasurementView::OnAppearanceChanged);
  connect(d->m_Controls->contrastOutlineCheckBox, &QCheckBox::toggled, this, &QmitkMeasurementView::OnAppearanceChanged);
  connect(d->m_Controls->showIn3DCheckBox, &QCheckBox::toggled, this, &QmitkMeasurementView::OnAppearanceChanged);
  connect(d->m_Controls->labelVisibilityComboBox, &QComboBox::currentIndexChanged, this, &QmitkMeasurementView::OnAppearanceChanged);
  connect(d->m_Controls->applyAppearanceButton, &QPushButton::clicked, this, &QmitkMeasurementView::OnApplyAppearanceClicked);
}

void QmitkMeasurementView::LoadAppearance()
{
  auto defaults = mitk::DataNode::New();
  mitk::PlanarFigureVtkMapper2D::SetDefaultProperties(defaults);
  mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(defaults);

  // The choices are the values of the property, so they read as in the Properties view.
  auto labelVisibility = mitk::PlanarFigureVisibilityProperty::New();

  for (auto it = labelVisibility->Begin(); it != labelVisibility->End(); ++it)
    d->m_Controls->labelVisibilityComboBox->addItem(QString::fromStdString(it->second), static_cast<int>(it->first));

  if (const auto* defaultLabelVisibility = dynamic_cast<const mitk::PlanarFigureVisibilityProperty*>(defaults->GetProperty("planarfigure.label")); nullptr != defaultLabelVisibility)
    labelVisibility->SetVisibility(defaultLabelVisibility->GetVisibility());

  auto labelVisibilityName = labelVisibility->GetValueAsString();

  auto showIn3D = false;
  defaults->GetBoolProperty("planarfigure.3drendering", showIn3D);

  float color[3] = { 1.0f, 1.0f, 1.0f };
  defaults->GetColor(color);

  auto lineWidth = 2.0f;
  defaults->GetFloatProperty("line width", lineWidth);

  auto showName = true;
  defaults->GetBoolProperty("planarfigure.drawname", showName);

  auto showMeasurements = true;
  defaults->GetBoolProperty("planarfigure.drawquantities", showMeasurements);

  auto contrastOutline = true;
  defaults->GetBoolProperty("planarfigure.drawcontrastoutline", contrastOutline);

  auto colorName = QColor::fromRgbF(color[0], color[1], color[2]).name().toStdString();

  if (const auto* preferences = this->GetPreferences(); nullptr != preferences)
  {
    colorName = preferences->Get(ColorPreference, colorName);
    lineWidth = preferences->GetFloat(LineWidthPreference, lineWidth);
    showName = preferences->GetBool(ShowNamePreference, showName);
    showMeasurements = preferences->GetBool(ShowMeasurementsPreference, showMeasurements);
    contrastOutline = preferences->GetBool(ContrastOutlinePreference, contrastOutline);
    labelVisibilityName = preferences->Get(LabelVisibilityPreference, labelVisibilityName);
    showIn3D = preferences->GetBool(ShowIn3DPreference, showIn3D);
  }

  auto* labelVisibilityComboBox = d->m_Controls->labelVisibilityComboBox;
  auto labelVisibilityIndex = labelVisibilityComboBox->findText(QString::fromStdString(labelVisibilityName));

  if (labelVisibilityIndex < 0)
    labelVisibilityIndex = labelVisibilityComboBox->findText(QString::fromStdString(labelVisibility->GetValueAsString()));

  labelVisibilityComboBox->setCurrentIndex(labelVisibilityIndex);

  d->m_FigureColor = QColor(QString::fromStdString(colorName));

  if (!d->m_FigureColor.isValid())
    d->m_FigureColor = QColor::fromRgbF(color[0], color[1], color[2]);

  d->m_Controls->lineWidthSlider->setValue(static_cast<int>(std::lround(lineWidth / LineWidthStep)));
  d->m_Controls->showNameCheckBox->setChecked(showName);
  d->m_Controls->showMeasurementsCheckBox->setChecked(showMeasurements);
  d->m_Controls->contrastOutlineCheckBox->setChecked(contrastOutline);
  d->m_Controls->showIn3DCheckBox->setChecked(showIn3D);

  this->UpdateColorButton();
  this->UpdateLineWidthLabel();
}

float QmitkMeasurementView::GetLineWidth() const
{
  return LineWidthStep * d->m_Controls->lineWidthSlider->value();
}

void QmitkMeasurementView::UpdateLineWidthLabel()
{
  d->m_Controls->lineWidthValueLabel->setText(QStringLiteral("%1 px").arg(this->GetLineWidth(), 0, 'f', 1));
}

void QmitkMeasurementView::SaveAppearance() const
{
  auto* preferences = this->GetPreferences();

  if (nullptr == preferences)
    return;

  preferences->Put(ColorPreference, d->m_FigureColor.name().toStdString());
  preferences->PutFloat(LineWidthPreference, this->GetLineWidth());
  preferences->PutBool(ShowNamePreference, d->m_Controls->showNameCheckBox->isChecked());
  preferences->PutBool(ShowMeasurementsPreference, d->m_Controls->showMeasurementsCheckBox->isChecked());
  preferences->PutBool(ContrastOutlinePreference, d->m_Controls->contrastOutlineCheckBox->isChecked());
  preferences->Put(LabelVisibilityPreference, d->m_Controls->labelVisibilityComboBox->currentText().toStdString());
  preferences->PutBool(ShowIn3DPreference, d->m_Controls->showIn3DCheckBox->isChecked());
  preferences->Flush();
}

void QmitkMeasurementView::ApplyAppearance(mitk::DataNode* node) const
{
  node->SetColor(d->m_FigureColor.redF(), d->m_FigureColor.greenF(), d->m_FigureColor.blueF());
  node->SetFloatProperty("line width", this->GetLineWidth());
  node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(
    static_cast<mitk::PlanarFigureVisibilityProperty::Visibility>(d->m_Controls->labelVisibilityComboBox->currentData().toInt())));
  node->SetBoolProperty("planarfigure.3drendering", d->m_Controls->showIn3DCheckBox->isChecked());
  node->SetBoolProperty("planarfigure.drawname", d->m_Controls->showNameCheckBox->isChecked());
  node->SetBoolProperty("planarfigure.drawquantities", d->m_Controls->showMeasurementsCheckBox->isChecked());
  node->SetBoolProperty("planarfigure.drawcontrastoutline", d->m_Controls->contrastOutlineCheckBox->isChecked());
}

void QmitkMeasurementView::UpdateColorButton()
{
  QPixmap swatch(d->m_Controls->colorButton->iconSize());
  swatch.fill(d->m_FigureColor);

  // Keeps colors close to the background apart from it.
  QPainter painter(&swatch);
  painter.setPen(d->m_Controls->colorButton->palette().color(QPalette::Mid));
  painter.drawRect(swatch.rect().adjusted(0, 0, -1, -1));
  painter.end();

  d->m_Controls->colorButton->setIcon(QIcon(swatch));
}

void QmitkMeasurementView::OnColorButtonClicked()
{
  const auto color = QColorDialog::getColor(d->m_FigureColor, d->m_Controls->colorButton, QStringLiteral("Color of New Figures"));

  if (!color.isValid())
    return;

  d->m_FigureColor = color;
  this->UpdateColorButton();
  this->OnAppearanceChanged();
}

void QmitkMeasurementView::OnAppearanceChanged()
{
  this->SaveAppearance();

  // The figure that is being placed is new, too.
  if (d->m_PendingNode.IsNotNull())
  {
    this->ApplyAppearance(d->m_PendingNode);
    this->RequestRenderWindowUpdate();
  }
}

void QmitkMeasurementView::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  // A pending figure sits on a plane of the current reference image and is added
  // below it in the data storage, so it cannot outlive a change of that image.
  this->CancelPlacement();

  const auto previousImageNode = d->m_SelectedImageNode;
  d->m_SelectedImageNode = nodes.empty() ? mitk::DataNode::Pointer() : nodes.front();

  // Aligning the views is reserved for a switch between two reference images. The
  // first one is picked by the auto-selection, when the view opens or when an image
  // appears while nothing is selected, and which one that is reflects no user intent,
  // so moving the views for it would be a surprise. As long as the views are not
  // aligned, the overlay offers to align them.
  const bool isSwitch = previousImageNode.IsNotNull() && d->m_SelectedImageNode.IsNotNull()
    && previousImageNode != d->m_SelectedImageNode;

  if (isSwitch && nullptr != d->m_RenderWindowPart)
  {
    const auto* referenceData = d->m_SelectedImageNode->GetData();

    if (nullptr != referenceData
      && !mitk::RenderWindowPartHelper::IsRenderWindowPartAlignedWithGeometry(d->m_RenderWindowPart, referenceData->GetGeometry()))
    {
      d->m_RenderWindowPart->InitializeViews(referenceData->GetTimeGeometry(), false);
    }
  }

  this->UpdateDrawingControls();
  this->ScheduleMeasurementTreeUpdate();
}

void QmitkMeasurementView::OnAlignViewsClicked()
{
  if (d->m_SelectedImageNode.IsNull() || nullptr == d->m_RenderWindowPart)
    return;

  const auto* referenceData = d->m_SelectedImageNode->GetData();

  if (nullptr == referenceData)
    return;

  // The views may currently show data located elsewhere, so also reset the camera.
  d->m_RenderWindowPart->InitializeViews(referenceData->GetTimeGeometry(), true);
}

void QmitkMeasurementView::UpdateDrawingControls()
{
  if (nullptr == d->m_AlignmentOverlay)
    return; // render window part notification before CreateQtPartControl()

  const auto* referenceData = d->m_SelectedImageNode.IsNotNull() ? d->m_SelectedImageNode->GetData() : nullptr;
  const bool hasReference = nullptr != referenceData;
  const bool isAligned = !hasReference
    || mitk::RenderWindowPartHelper::IsRenderWindowPartAlignedWithGeometry(d->m_RenderWindowPart, referenceData->GetGeometry());

  // The overlay only covers this view. A pending figure would still accept clicks
  // in the render windows and end up on the wrong plane.
  if (!isAligned)
    this->CancelPlacement();

  d->m_AlignmentOverlay->setVisible(hasReference && !isAligned);
  d->m_Controls->tools->setEnabled(hasReference && isAligned);

  this->UpdateHint();
}

void QmitkMeasurementView::UpdateToolOptions()
{
  const auto fixedSize = d->m_PendingTool.has_value() ? Tools[*d->m_PendingTool].Size : FixedSize::None;

  d->m_Controls->fixedSizeGroupBox->setVisible(FixedSize::None != fixedSize);
  d->m_Controls->thicknessLabel->setVisible(FixedSize::RadiusAndThickness == fixedSize);
  d->m_Controls->thicknessSpinBox->setVisible(FixedSize::RadiusAndThickness == fixedSize);
}

void QmitkMeasurementView::UpdateHint()
{
  if (nullptr == d->m_AlignmentOverlay)
    return;

  QString hint;

  if (d->m_PendingNode.IsNotNull())
  {
    if (const auto* figure = dynamic_cast<const mitk::PlanarFigure*>(d->m_PendingNode->GetData()); nullptr != figure)
      hint = GetPlacementHint(figure);
  }
  else if (d->m_Controls->tools->isEnabled())
  {
    const auto selectedFigures = this->GetSelectedFigures();

    hint = !selectedFigures.empty()
      ? GetEditingHint(static_cast<const mitk::PlanarFigure*>(selectedFigures.front()->GetData()))
      : QStringLiteral("Pick a tool to draw a measurement on the reference image.");
  }

  if (hint != d->m_Controls->hintCard->GetMessage())
    d->m_Controls->hintCard->SetMessage(hint);

  d->m_Controls->hintCard->setVisible(!hint.isEmpty());
}

std::vector<mitk::DataNode*> QmitkMeasurementView::GetSelectedFigures() const
{
  std::vector<mitk::DataNode*> figures;

  for (const auto& node : d->m_CurrentSelection)
  {
    if (node.IsNull() || d->m_Figures.count(node.GetPointer()) == 0)
      continue;

    if (const auto* figure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData()); nullptr != figure && figure->IsComplete())
      figures.push_back(node);
  }

  return figures;
}

void QmitkMeasurementView::OnSelectedFiguresChanged()
{
  if (nullptr == d->m_AlignmentOverlay)
    return; // selection before CreateQtPartControl()

  d->m_Controls->applyAppearanceButton->setEnabled(!this->GetSelectedFigures().empty());
  this->UpdateHint();
}

void QmitkMeasurementView::OnApplyAppearanceClicked()
{
  for (auto* node : this->GetSelectedFigures())
    this->ApplyAppearance(node);

  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  d->m_RenderWindowPart = renderWindowPart;
  this->UpdateDrawingControls();
}

void QmitkMeasurementView::RenderWindowPartDeactivated(mitk::IRenderWindowPart*)
{
  // Cancel while the part is still alive so ResetPlacement() can hand the
  // interaction reference geometry back to it.
  this->CancelPlacement();

  d->m_RenderWindowPart = nullptr;
  this->UpdateDrawingControls();
}

void QmitkMeasurementView::RenderWindowPartInputChanged(mitk::IRenderWindowPart*)
{
  this->UpdateDrawingControls();
}

void QmitkMeasurementView::NodeAdded(const mitk::DataNode* node)
{
  auto* planarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

  auto isPositionMarker = false;
  node->GetBoolProperty("isContourMarker", isPositionMarker);

  // Helper objects belong to other components, which interact with them on their own.
  auto isHelperObject = false;
  node->GetBoolProperty("helper object", isHelperObject);

  if (nullptr == planarFigure || isPositionMarker || isHelperObject)
    return;

  auto nonConstNode = const_cast<mitk::DataNode*>(node);
  mitk::PlanarFigureInteractor::Pointer interactor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

  if (interactor.IsNull())
  {
    interactor = mitk::PlanarFigureInteractor::New();
    auto planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");

    interactor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule);
    interactor->SetEventConfig("PlanarFigureConfig.xml", planarFigureModule);
  }

  interactor->SetDataNode(nonConstNode);

  auto& figure = d->m_Figures[node];
  figure.Node = nonConstNode;

  figure.EndPlacementObserver.Reset(planarFigure, mitk::EndPlacementPlanarFigureEvent(), [this](const itk::EventObject&) {
    const auto toolIndex = d->m_PendingTool;

    this->ResetPlacement();
    this->ScheduleMeasurementTreeUpdate();

    // A figure of fixed size takes a single click, so its tool stays armed for the next one.
    // Placed figures cannot be edited meanwhile, since the armed tool holds the exclusive interaction.
    if (toolIndex.has_value() && this->IsPlacedWithFixedSize(*toolIndex))
      this->StartPlacement(*toolIndex);
  });

  figure.SelectObserver.Reset(planarFigure, mitk::SelectPlanarFigureEvent(), [this, nonConstNode](const itk::EventObject&) {
    this->SelectNode(nonConstNode);
    this->OnSelectedFiguresChanged();
    this->RequestRenderWindowUpdate();
  });

  figure.PointMovedObserver.Reset(planarFigure, mitk::PointMovedPlanarFigureEvent(),
    [this](const itk::EventObject&) { this->ScheduleMeasurementTreeUpdate(); });

  this->ScheduleMeasurementTreeUpdate();
}

void QmitkMeasurementView::NodeChanged(const mitk::DataNode* node)
{
  // A figure may have been renamed, recolored or changed by other means than interaction.
  if (d->m_Figures.count(node) != 0)
    this->ScheduleMeasurementTreeUpdate();
}

void QmitkMeasurementView::NodeRemoved(const mitk::DataNode* node)
{
  if (node == d->m_PendingNode)
    this->ResetPlacement();

  if (auto it = d->m_Figures.find(node); it != d->m_Figures.end())
  {
    it->second.Node->SetDataInteractor(nullptr);
    d->m_Figures.erase(it);
  }

  // An unfinished figure goes with its parent.
  auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
  auto derivations = this->GetDataStorage()->GetDerivations(node, isPlanarFigure);

  for (const auto& derivation : *derivations)
  {
    const auto* planarFigure = dynamic_cast<const mitk::PlanarFigure*>(derivation->GetData());

    if (nullptr != planarFigure && !planarFigure->IsComplete())
      this->GetDataStorage()->Remove(derivation);
  }

  // Deferred, since the node is still in the data storage at this point.
  this->ScheduleMeasurementTreeUpdate();
}

void QmitkMeasurementView::SelectNode(mitk::DataNode* node)
{
  this->SelectNodes({ node });
  this->UpdateMeasurementTreeSelection();
}

void QmitkMeasurementView::SelectNodes(const QList<mitk::DataNode::Pointer>& nodes)
{
  for (auto& selectedNode : d->m_CurrentSelection)
    selectedNode->SetSelected(false);

  d->m_CurrentSelection = nodes;

  this->FireNodesSelected(nodes);
  this->SynchronizeDataManagerSelection();

  // Set "selected" last: SynchronizeDataManagerSelection updates the Data Manager
  // tree selection, whose NodeSelectionChanged handler re-derives every node's
  // "selected" property and transiently clears it for the picked node. Setting it
  // afterwards leaves the figure in the selected (red) appearance.
  for (auto& node : nodes)
    node->SetSelected(true);
}

void QmitkMeasurementView::ResetPlacement()
{
  d->m_PlacementSelectionObserver.Reset();
  d->m_PlacementProgressObserver.Reset();
  d->m_PendingNode = nullptr;
  d->m_PendingTool.reset();
  d->m_ExclusiveInteractionClaim.Reset();

  d->m_CancelPlacementShortcut->setEnabled(false);

  for (auto* button : d->m_ToolButtons->buttons())
    button->setChecked(false);

  if (d->m_RestoreInteractionReferenceGeometry.has_value())
  {
    this->SetInteractionReferenceGeometry(*d->m_RestoreInteractionReferenceGeometry);
    d->m_RestoreInteractionReferenceGeometry.reset();
  }

  this->UpdateToolOptions();
  this->OnSelectedFiguresChanged();
}

void QmitkMeasurementView::SetInteractionReferenceGeometry(const mitk::TimeGeometry* geometry)
{
  if (nullptr == d->m_RenderWindowPart)
    return;

  try
  {
    d->m_RenderWindowPart->SetInteractionReferenceGeometry(geometry);
  }
  catch (const mitk::Exception& e)
  {
    // Thrown when the selected time point lies outside the geometry's time range.
    MITK_WARN << "Could not set the interaction reference geometry: " << e.GetDescription();
  }
}

void QmitkMeasurementView::CancelPlacement()
{
  if (d->m_PendingNode.IsNull())
    return;

  // Keep the node alive: NodeRemoved() resets m_PendingNode while the
  // removal is still in progress.
  mitk::DataNode::Pointer node = d->m_PendingNode;
  this->GetDataStorage()->Remove(node);

  // A programmatic removal does not trigger OnSelectionChanged, so drop the
  // canceled node from the selection explicitly.
  d->m_CurrentSelection.removeAll(node);
  this->OnSelectedFiguresChanged();

  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::OnToolClicked(int toolIndex)
{
  if (d->m_ToolButtons->button(toolIndex)->isChecked())
  {
    this->StartPlacement(toolIndex);
  }
  else
  {
    this->CancelPlacement();
  }
}

void QmitkMeasurementView::OnFixedSizeChanged()
{
  // The size is applied when the placement starts, so start over with the new one.
  if (d->m_PendingTool.has_value() && FixedSize::None != Tools[*d->m_PendingTool].Size)
    this->StartPlacement(*d->m_PendingTool);
}

void QmitkMeasurementView::StartPlacement(int toolIndex)
{
  this->CancelPlacement();

  d->m_ExclusiveInteractionClaim = mitk::ExclusiveInteraction::Acquire([this]() {
    this->CancelPlacement();
    return true;
  });

  auto* button = d->m_ToolButtons->button(toolIndex);

  // The armed tool of another view keeps the exclusive interaction, e.g. to
  // keep its unconfirmed results.
  if (!d->m_ExclusiveInteractionClaim.IsActive())
  {
    button->setChecked(false);
    return;
  }

  const auto& tool = Tools[toolIndex];
  auto figure = tool.Create();
  this->ApplyFixedSize(figure, toolIndex);

  this->AddFigureToDataStorage(figure, tool.Name);
  d->m_PendingTool = toolIndex;

  // CancelPlacement() unchecks all tools, including the clicked one.
  button->setChecked(true);

  d->m_PlacementProgressObserver.Reset(figure, itk::ModifiedEvent(), [this](const itk::EventObject&) { this->UpdateHint(); });

  this->UpdateToolOptions();
  this->UpdateHint();
}

bool QmitkMeasurementView::IsPlacedWithFixedSize(int toolIndex) const
{
  return FixedSize::None != Tools[toolIndex].Size && d->m_Controls->fixedSizeGroupBox->isChecked();
}

void QmitkMeasurementView::ApplyFixedSize(mitk::PlanarFigure* figure, int toolIndex) const
{
  if (!this->IsPlacedWithFixedSize(toolIndex))
    return;

  const auto fixedSize = Tools[toolIndex].Size;

  const auto radius = d->m_Controls->radiusSpinBox->value();
  MakePreset(figure, radius);

  if (auto* ring = dynamic_cast<mitk::PlanarDoubleEllipse*>(figure); nullptr != ring && FixedSize::RadiusAndThickness == fixedSize)
    ring->SetThickness(std::min(d->m_Controls->thicknessSpinBox->value(), radius));
}

void QmitkMeasurementView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  d->m_CurrentSelection = nodes;
  this->UpdateMeasurementTreeSelection();

  // bug 16600: deselecting all planarfigures by clicking on datamanager when no node is selected
  if (d->m_CurrentSelection.empty())
  {
    auto planarFigures = this->GetAllPlanarFigures();

    for (const auto& node : *planarFigures)
      node->SetSelected(false);
  }

  this->OnSelectedFiguresChanged();
  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::OnCopyToClipboard()
{
  const auto* tree = d->m_Controls->measurementsTree;

  std::set<int> rows; // in the order of the tree

  for (auto* item : tree->selectedItems())
    rows.insert(tree->indexOfTopLevelItem(nullptr != item->parent() ? item->parent() : item));

  if (rows.empty())
  {
    for (int i = 0; i < tree->topLevelItemCount(); ++i)
      rows.insert(i);
  }

  auto text = QStringLiteral("Figure,Measurement,Value,Unit\n");

  for (const auto row : rows)
  {
    const auto it = d->m_Figures.find(GetNode(tree->topLevelItem(row)));

    if (it == d->m_Figures.end())
      continue;

    const auto& node = it->second.Node;
    const auto* figure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());

    if (nullptr == figure)
      continue;

    const auto name = ToCsvField(QString::fromStdString(node->GetName()));

    for (const auto& measurement : figure->GetMeasurements())
    {
      text += QStringLiteral("%1,%2,%3,%4\n")
        .arg(name)
        .arg(QString::fromStdString(mitk::PlanarFigure::GetDisplayName(measurement.Type)))
        .arg(QString::number(measurement.Value, 'f', 2))
        .arg(QString::fromStdString(mitk::PlanarFigure::GetUnitSymbol(measurement.Type)));
    }
  }

  QApplication::clipboard()->setText(text, QClipboard::Clipboard);
}

void QmitkMeasurementView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const std::string& baseName)
{
  auto dataStorage = this->GetDataStorage();

  auto newNode = mitk::DataNode::New();
  newNode->SetName(dataStorage->GetUniqueName(baseName));
  newNode->SetData(figure);
  this->ApplyAppearance(newNode);

  if (d->m_SelectedImageNode.IsNotNull())
  {
    dataStorage->Add(newNode, d->m_SelectedImageNode);
  }
  else
  {
    dataStorage->Add(newNode);
  }

  this->SelectNode(newNode);

  d->m_PendingNode = newNode;
  d->m_CancelPlacementShortcut->setEnabled(true);

  // A figure being placed is canceled once something else takes the selection,
  // e.g. the Data Manager when another node gets selected, so no unfinished
  // figure stays behind. The cancellation is deferred to not remove the figure
  // while its deselection is still processed.
  d->m_PlacementSelectionObserver.Reset(newNode, itk::ModifiedEvent(), [this](const itk::EventObject&) {
    if (d->m_PendingNode.IsNull() || d->m_PendingNode->IsSelected())
      return;

    QTimer::singleShot(0, this, [this]() {
      if (d->m_PendingNode.IsNotNull() && !d->m_PendingNode->IsSelected())
        this->CancelPlacement();
    });
  });

  // Decoupled render windows compare themselves against this geometry while the
  // figure is being placed and offer a per-window reset (see QmitkRenderWindow).
  // There is only one such geometry per renderer and the Segmentation view claims
  // it for as long as one of its tools is active, so borrow it for the placement
  // and hand it back afterwards instead of clearing it.
  const auto* referenceData = d->m_SelectedImageNode.IsNotNull() ? d->m_SelectedImageNode->GetData() : nullptr;

  if (nullptr != d->m_RenderWindowPart && nullptr != referenceData)
  {
    d->m_RestoreInteractionReferenceGeometry = GetInteractionReferenceGeometry(d->m_RenderWindowPart);
    this->SetInteractionReferenceGeometry(referenceData->GetTimeGeometry());
  }
}

void QmitkMeasurementView::ScheduleMeasurementTreeUpdate()
{
  // Before CreateQtPartControl(), or while an update is pending
  if (nullptr == d->m_Controls->measurementsTree || d->m_IsMeasurementTreeUpdateScheduled)
    return;

  d->m_IsMeasurementTreeUpdateScheduled = true;
  QTimer::singleShot(0, d->m_Controls->measurementsTree, [this]() { this->UpdateMeasurementTree(); });
}

std::vector<mitk::DataNode*> QmitkMeasurementView::GetMeasuredFigures() const
{
  std::vector<mitk::DataNode*> figures;

  if (d->m_SelectedImageNode.IsNull())
    return figures;

  auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
  auto derivations = this->GetDataStorage()->GetDerivations(d->m_SelectedImageNode, isPlanarFigure, false);

  for (const auto& node : *derivations)
  {
    if (d->m_Figures.count(node) == 0)
      continue;

    const auto* figure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());

    if (nullptr != figure && figure->IsComplete())
      figures.push_back(node);
  }

  return figures;
}

void QmitkMeasurementView::UpdateMeasurementTree()
{
  d->m_IsMeasurementTreeUpdateScheduled = false;

  auto* tree = d->m_Controls->measurementsTree;
  const QSignalBlocker blocker(tree);

  const auto figures = this->GetMeasuredFigures();
  const auto numberOfFigures = static_cast<int>(figures.size());

  for (int i = 0; i < numberOfFigures; ++i)
  {
    auto* node = figures[i];

    QTreeWidgetItem* item = nullptr;

    for (int j = i; j < tree->topLevelItemCount(); ++j)
    {
      if (GetNode(tree->topLevelItem(j)) == node)
      {
        item = j == i ? tree->topLevelItem(j) : tree->takeTopLevelItem(j);
        break;
      }
    }

    if (nullptr == item)
    {
      item = new QTreeWidgetItem;
      item->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<quintptr>(node)));
    }

    if (tree->indexOfTopLevelItem(item) != i)
    {
      tree->insertTopLevelItem(i, item);
      item->setExpanded(true);
    }

    SetText(item, 0, QString::fromStdString(node->GetName()));

    if (auto* descriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(node); nullptr != descriptor)
      item->setIcon(0, descriptor->GetIcon(node));

    const auto measurements = static_cast<const mitk::PlanarFigure*>(node->GetData())->GetMeasurements();
    const auto numberOfMeasurements = static_cast<int>(measurements.size());

    while (item->childCount() > numberOfMeasurements)
      delete item->takeChild(item->childCount() - 1);

    while (item->childCount() < numberOfMeasurements)
      (new QTreeWidgetItem(item))->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

    for (int j = 0; j < numberOfMeasurements; ++j)
    {
      auto* child = item->child(j);
      SetText(child, 0, QString::fromStdString(mitk::PlanarFigure::GetDisplayName(measurements[j].Type)));
      SetText(child, 1, QString::fromStdString(mitk::PlanarFigure::FormatValue(measurements[j])));
    }
  }

  while (tree->topLevelItemCount() > numberOfFigures)
    delete tree->takeTopLevelItem(numberOfFigures);

  this->UpdateMeasurementTreeSelection();
}

void QmitkMeasurementView::UpdateMeasurementTreeSelection()
{
  auto* tree = d->m_Controls->measurementsTree;

  if (nullptr == tree)
    return; // selection before CreateQtPartControl()

  const QSignalBlocker blocker(tree);

  tree->clearSelection();

  QTreeWidgetItem* firstSelectedItem = nullptr;

  for (int i = 0; i < tree->topLevelItemCount(); ++i)
  {
    auto* item = tree->topLevelItem(i);
    const auto* node = GetNode(item);

    if (std::find(d->m_CurrentSelection.begin(), d->m_CurrentSelection.end(), node) == d->m_CurrentSelection.end())
      continue;

    item->setSelected(true);

    if (nullptr == firstSelectedItem)
      firstSelectedItem = item;
  }

  if (nullptr != firstSelectedItem)
    tree->scrollToItem(firstSelectedItem);
}

void QmitkMeasurementView::OnMeasurementTreeSelectionChanged()
{
  QList<mitk::DataNode::Pointer> nodes;

  for (const auto* item : d->m_Controls->measurementsTree->selectedItems())
  {
    if (auto it = d->m_Figures.find(GetNode(item)); it != d->m_Figures.end() && !nodes.contains(it->second.Node))
      nodes.append(it->second.Node);
  }

  this->SelectNodes(nodes);
  this->OnSelectedFiguresChanged();
  this->RequestRenderWindowUpdate();
}

void QmitkMeasurementView::OnMeasurementTreeItemActivated()
{
  const auto* item = d->m_Controls->measurementsTree->currentItem();

  if (nullptr == item)
    return;

  if (auto it = d->m_Figures.find(GetNode(item)); it != d->m_Figures.end())
    this->ShowFigure(it->second.Node);
}

void QmitkMeasurementView::ShowFigure(const mitk::DataNode* node)
{
  if (nullptr == d->m_RenderWindowPart)
    return;

  const auto* figure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());
  const auto* plane = nullptr != figure ? figure->GetPlaneGeometry() : nullptr;

  if (nullptr == plane || !figure->IsComplete())
    return;

  const auto points = figure->GetControlPoints();

  mitk::Vector2D sum;
  sum.Fill(0.0);

  for (const auto& point : points)
    sum += point.GetVectorFromOrigin();

  mitk::Point2D planeCenter;
  planeCenter.Fill(0.0);
  planeCenter += sum / static_cast<double>(points.size());

  mitk::Point3D center;
  plane->Map(planeCenter, center);

  d->m_RenderWindowPart->SetSelectedPosition(center);

  // A figure drawn on a rotated slice lies in no plane of views that are aligned to the image again.
  auto normal = plane->GetNormal();
  normal.Normalize();

  QmitkRenderWindow* closestRenderWindow = nullptr;
  double closestAlignment = -1.0;

  for (auto* renderWindow : d->m_RenderWindowPart->GetQmitkRenderWindows())
  {
    auto* renderer = renderWindow->GetRenderer();

    if (nullptr == renderer || mitk::BaseRenderer::Standard2D != renderer->GetMapperID())
      continue;

    const auto* rendererPlane = renderer->GetCurrentWorldPlaneGeometry();

    if (nullptr == rendererPlane)
      continue;

    auto rendererNormal = rendererPlane->GetNormal();
    rendererNormal.Normalize();

    const auto alignment = std::abs(normal * rendererNormal);

    // Planes of rotated views carry the rounding errors of the rotation.
    if (alignment > 1.0 - 1e-6)
      return; // shown already

    if (alignment > closestAlignment)
    {
      closestAlignment = alignment;
      closestRenderWindow = renderWindow;
    }
  }

  if (nullptr != closestRenderWindow)
  {
    closestRenderWindow->GetSliceNavigationController()->ReorientSlices(center, plane->GetNormal());
    this->RequestRenderWindowUpdate();
  }
}

void QmitkMeasurementView::AddAllInteractors()
{
  auto planarFigures = this->GetAllPlanarFigures();

  for (const auto& node : *planarFigures)
    this->NodeAdded(node);
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkMeasurementView::GetAllPlanarFigures() const
{
  auto isPlanarFigure = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();
  auto isHelperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
  auto isNotHelperButPlanarFigure = mitk::NodePredicateAnd::New(isPlanarFigure, mitk::NodePredicateNot::New(isHelperObject));

  return this->GetDataStorage()->GetSubset(isNotHelperButPlanarFigure);
}
