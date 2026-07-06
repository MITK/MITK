/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNCellOverlay.h"

#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>
#include <QmitkSliceNavigationWidget.h>
#include <QmitkStepperAdapter.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkBaseRenderer.h>
#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkRenderingManager.h>
#include <mitkTimeNavigationController.h>

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindow.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QDoubleSpinBox>
#include <QFontMetrics>
#include <QFormLayout>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
  constexpr int PassiveRibbonWidth = 2;
  constexpr int ActiveRibbonWidth = 14;
  constexpr int RibbonEndHandleHeight = 16;
  constexpr int ReadoutMargin = 8;
  constexpr int HueDotDiameter = 8;
  constexpr int ChipSize = 14;
  constexpr int EdgeStripThickness = 20;
  constexpr int EdgeActivationDistance = 16;
  constexpr int TopStripHeight = 4;
  constexpr int SliceTickHeight = 8;

  const QColor IdleText(255, 255, 255, 140);    // 55 % white
  const QColor ActiveText(255, 255, 255, 216);  // 85 % white

  QFont ReadoutFont(const QFont& base)
  {
    QFont font(base);
    font.setPointSize(10);
    // Tabular numerals keep the readout from jittering while values change.
    font.setFeature(QFont::Tag("tnum"), 1);
    return font;
  }

  QFont LabelFont(const QFont& base)
  {
    QFont font(base);
    font.setPointSize(8);
    font.setFeature(QFont::Tag("tnum"), 1);
    return font;
  }

  QString FormatValue(double value)
  {
    return QString::number(std::llround(value));
  }
}

QmitkMxNCellOverlay::QmitkMxNCellOverlay(QmitkRenderWindowWidget* cell,
                                         QmitkMxNMultiWidget* editor,
                                         QmitkRenderWindowProximity* proximity)
  : QmitkOverlayWidget(cell)
  , m_Cell(cell)
  , m_Editor(editor)
  , m_Proximity(proximity)
{
  if (nullptr == cell)
  {
    mitkThrow() << "QmitkMxNCellOverlay: cell must not be null.";
  }
  if (nullptr == editor)
  {
    mitkThrow() << "QmitkMxNCellOverlay: editor must not be null.";
  }
  if (nullptr == proximity)
  {
    mitkThrow() << "QmitkMxNCellOverlay: proximity controller must not be null.";
  }
  if (nullptr == cell->GetRenderWindow() || nullptr == cell->GetRenderWindow()->GetVtkRenderWindow())
  {
    mitkThrow() << "QmitkMxNCellOverlay: the cell has no render window yet.";
  }

  this->setFocusPolicy(Qt::NoFocus);

  // While the overlay is interactive (masked), pointer events land here
  // instead of on the render window; feeding them back keeps the proximity
  // state alive while the pointer rests on the furniture itself.
  proximity->AddEventSource(this);

  m_RibbonRegion = proximity->RegisterRegion([this]() { return this->RibbonRect(true); });
  m_ReadoutRegion = proximity->RegisterRegion([this]() { return this->ReadoutRect(); });
  m_BottomRegion = proximity->RegisterRegion([this]() { return this->BottomStripRect(); },
                                             EdgeActivationDistance);
  m_TopRegion = proximity->RegisterRegion([this]() { return this->TopStripRect(); },
                                          EdgeActivationDistance);
  connect(proximity, &QmitkRenderWindowProximity::StateChanged,
          this, &QmitkMxNCellOverlay::OnProximityStateChanged);

  // Context-menu handling needs the render window's own mouse stream (the
  // overlay is transparent there); the base class already filters the cell.
  cell->GetRenderWindow()->installEventFilter(this);

  // Keep a reference of our own: the observer must be removable in the
  // destructor regardless of the Qt child destruction order.
  m_VtkRenderWindow = cell->GetRenderWindow()->GetVtkRenderWindow();

  auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
  callback->SetClientData(this);
  callback->SetCallback(&QmitkMxNCellOverlay::OnVtkRenderEnd);
  m_VtkObserverTag = m_VtkRenderWindow->AddObserver(vtkCommand::EndEvent, callback);

  this->ScheduleValueRefresh();
}

QmitkMxNCellOverlay::~QmitkMxNCellOverlay()
{
  if (nullptr != m_VtkRenderWindow)
  {
    m_VtkRenderWindow->RemoveObserver(m_VtkObserverTag);
  }
}

void QmitkMxNCellOverlay::SetReadoutVisible(bool visible)
{
  if (visible == m_ReadoutVisible)
  {
    return;
  }

  m_ReadoutVisible = visible;
  this->UpdateInteractivity();
  this->update();
}

void QmitkMxNCellOverlay::SetCleanView(bool cleanView)
{
  if (cleanView == m_CleanView)
  {
    return;
  }

  m_CleanView = cleanView;
  this->UpdateInteractivity();
  this->update();
}

QRect QmitkMxNCellOverlay::RenderWindowRect() const
{
  auto* renderWindow = m_Cell->GetRenderWindow();
  return nullptr != renderWindow ? renderWindow->geometry() : QRect();
}

QRect QmitkMxNCellOverlay::RibbonRect(bool active) const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return QRect();
  }

  const int width = active ? ActiveRibbonWidth : PassiveRibbonWidth;
  return QRect(area.right() - width + 1, area.top(), width, area.height());
}

QRect QmitkMxNCellOverlay::ReadoutRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid() || !m_HasLevelWindow)
  {
    return QRect();
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const QString text = QStringLiteral("W %1 L %2")
    .arg(FormatValue(m_LevelWindow.GetWindow()), FormatValue(m_LevelWindow.GetLevel()));
  const int width = metrics.horizontalAdvance(text) + HueDotDiameter + 3 * ReadoutMargin / 2;
  const int height = metrics.height() + 4;

  return QRect(area.left() + ReadoutMargin, area.bottom() - ReadoutMargin - height, width, height);
}

QRect QmitkMxNCellOverlay::ColormapChipRect() const
{
  const QRect readout = this->ReadoutRect();
  if (!readout.isValid())
  {
    return QRect();
  }

  return QRect(readout.right() + ReadoutMargin,
               readout.center().y() - ChipSize / 2, ChipSize, ChipSize);
}

QRect QmitkMxNCellOverlay::SliceReadoutRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid() || m_SliceSteps == 0)
  {
    return QRect();
  }

  const QFontMetrics metrics(ReadoutFont(this->font()));
  const QString text = QStringLiteral("%1/%2").arg(m_SlicePosition + 1).arg(m_SliceSteps);
  const int width = metrics.horizontalAdvance(text) + ReadoutMargin;
  const int height = metrics.height() + 4;

  return QRect(area.right() - ActiveRibbonWidth - ReadoutMargin - width,
               area.bottom() - ReadoutMargin - height, width, height);
}

QRect QmitkMxNCellOverlay::BottomStripRect() const
{
  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return QRect();
  }

  // Leaves the W/L readout corner to its own region and the ribbon its edge.
  const QRect readout = this->ReadoutRect();
  const int left = (readout.isValid() ? readout.right() : area.left()) + 2 * ReadoutMargin;
  return QRect(left, area.bottom() - EdgeStripThickness,
               area.right() - ActiveRibbonWidth - left, EdgeStripThickness);
}

QRect QmitkMxNCellOverlay::TopStripRect() const
{
  return QRect(0, 0, this->width(), EdgeStripThickness);
}

bool QmitkMxNCellOverlay::IsRevealed(QmitkRenderWindowProximity::State state, bool alwaysOn) const
{
  if (m_CleanView)
  {
    return false;
  }
  return alwaysOn || state != QmitkRenderWindowProximity::State::Idle;
}

void QmitkMxNCellOverlay::OnProximityStateChanged(QmitkRenderWindowProximity::RegionId id,
                                                  QmitkRenderWindowProximity::State state)
{
  if (id == m_RibbonRegion)
  {
    m_RibbonState = state;
  }
  else if (id == m_ReadoutRegion)
  {
    m_ReadoutState = state;
  }
  else if (id == m_BottomRegion)
  {
    m_BottomState = state;
    this->UpdateSliceSlider();
  }
  else if (id == m_TopRegion)
  {
    m_TopState = state;
    m_Cell->ShowUtilityWidget(state == QmitkRenderWindowProximity::State::Active);
  }
  else
  {
    return;
  }

  this->UpdateInteractivity();
  this->update();
}

void QmitkMxNCellOverlay::UpdateSliceSlider()
{
  const bool show = !m_CleanView
    && m_BottomState == QmitkRenderWindowProximity::State::Active && m_SliceSteps > 1;

  if (show && nullptr == m_SliceSlider)
  {
    auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
    if (nullptr == renderer || nullptr == renderer->GetSliceNavigationController())
    {
      return;
    }
    // The standard slider, floating over the image (a child of the cell,
    // not of this overlay, so the overlay's input mask does not apply).
    auto* slider = new QmitkSliceNavigationWidget(m_Cell);
    new QmitkStepperAdapter(slider, renderer->GetSliceNavigationController()->GetStepper());
    m_SliceSlider = slider;
    if (!m_Proximity.isNull())
    {
      m_Proximity->AddEventSource(m_SliceSlider);
    }
  }

  if (nullptr == m_SliceSlider)
  {
    return;
  }

  if (show)
  {
    const QRect area = this->RenderWindowRect();
    const int height = m_SliceSlider->sizeHint().height();
    m_SliceSlider->setGeometry(area.left() + ReadoutMargin,
                               area.bottom() - EdgeStripThickness - height,
                               area.width() - 2 * ReadoutMargin - ActiveRibbonWidth, height);
    m_SliceSlider->show();
    m_SliceSlider->raise();
  }
  else
  {
    m_SliceSlider->hide();
  }
}

void QmitkMxNCellOverlay::UpdateInteractivity()
{
  using State = QmitkRenderWindowProximity::State;

  const bool interactive = !m_CleanView
    && (m_RibbonState == State::Active || m_ReadoutState == State::Active
        || m_DragMode != DragMode::None);

  if (!interactive)
  {
    this->clearMask();
    this->setTransparentForMouseEvents(true);
    return;
  }

  // The mask clips painting as well as input, so it must cover every piece
  // of currently visible furniture, not only the active one.
  QRegion mask;
  mask += this->RibbonRect(m_RibbonState == State::Active || m_DragMode != DragMode::None);
  if (this->IsRevealed(m_ReadoutState, m_ReadoutVisible))
  {
    mask += this->ReadoutRect();
    mask += this->ColormapChipRect();
  }
  // Paint-only furniture (breadcrumbs, edge strips) still needs mask
  // coverage or it would vanish while another region is interactive.
  mask += this->SliceReadoutRect();
  const QRect area = this->RenderWindowRect();
  if (area.isValid())
  {
    mask += QRect(area.left(), area.bottom() - SliceTickHeight, area.width(), SliceTickHeight);
  }
  mask += QRect(0, 0, this->width(), TopStripHeight);

  this->setMask(mask);
  this->setTransparentForMouseEvents(false);
  this->setMouseTracking(true);
}

void QmitkMxNCellOverlay::paintEvent(QPaintEvent* /*event*/)
{
  if (m_CleanView)
  {
    return;
  }

  const QRect area = this->RenderWindowRect();
  if (!area.isValid())
  {
    return;
  }

  using State = QmitkRenderWindowProximity::State;

  QPainter painter(this);

  // LUT ribbon (right edge, actual colormap)
  const bool ribbonActive = m_RibbonState == State::Active || m_DragMode != DragMode::None;
  const QRect ribbon = this->RibbonRect(ribbonActive);
  if (!m_LutStrip.isNull())
  {
    painter.drawImage(ribbon, m_LutStrip);
  }
  else
  {
    painter.fillRect(ribbon, QColor(255, 255, 255, 40));
  }

  if (ribbonActive && m_HasLevelWindow)
  {
    painter.setFont(LabelFont(this->font()));
    painter.setPen(ActiveText);
    const QFontMetrics metrics(painter.font());
    const QString upper = FormatValue(m_LevelWindow.GetUpperWindowBound());
    const QString lower = FormatValue(m_LevelWindow.GetLowerWindowBound());
    painter.drawText(
      QPoint(ribbon.left() - 4 - metrics.horizontalAdvance(upper), ribbon.top() + metrics.ascent() + 2),
      upper);
    painter.drawText(
      QPoint(ribbon.left() - 4 - metrics.horizontalAdvance(lower), ribbon.bottom() - 4), lower);

    // End handles: short contrasting notches marking the draggable bounds.
    painter.fillRect(QRect(ribbon.left(), ribbon.top(), ribbon.width(), 2), ActiveText);
    painter.fillRect(QRect(ribbon.left(), ribbon.bottom() - 1, ribbon.width(), 2), ActiveText);
  }

  // Corner readout bottom-left
  if (m_HasLevelWindow && this->IsRevealed(m_ReadoutState, m_ReadoutVisible))
  {
    const QRect readout = this->ReadoutRect();
    painter.setFont(ReadoutFont(this->font()));
    painter.setPen(m_ReadoutState == State::Active ? ActiveText : IdleText);
    const QString text = QStringLiteral("W %1 L %2")
      .arg(FormatValue(m_LevelWindow.GetWindow()), FormatValue(m_LevelWindow.GetLevel()));
    painter.drawText(readout, Qt::AlignLeft | Qt::AlignVCenter, text);

    // Hue dot: the cell's windowing/LUT group identity wins the single dot
    // slot (it lives beside the W/L readout); otherwise a navigation link
    // no seam can show claims it, as the pointer to the layout editor.
    const auto windowId = m_Cell->GetWidgetName();
    std::optional<std::string> dotGroup;
    auto link = m_Editor->GetSyncLink(windowId, QmitkMxNSyncDimension::Windowing);
    if (!link.has_value())
    {
      link = m_Editor->GetSyncLink(windowId, QmitkMxNSyncDimension::Lut);
    }
    if (link.has_value())
    {
      dotGroup = link->group;
    }
    else
    {
      try
      {
        dotGroup = m_Editor->GetNonAdjacentNavGroup(windowId);
      }
      catch (const mitk::Exception&)
      {
        // Transient mid-layout-change state; the next refresh repaints.
      }
    }
    if (dotGroup.has_value())
    {
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setPen(Qt::NoPen);
      painter.setBrush(m_Editor->GetSyncGroupColor(*dotGroup));
      const int dotX = readout.right() - HueDotDiameter;
      const int dotY = readout.center().y() - HueDotDiameter / 2;
      painter.drawEllipse(dotX, dotY, HueDotDiameter, HueDotDiameter);
      painter.setRenderHint(QPainter::Antialiasing, false);
    }

    // Colormap chip (active layer): a miniature of the current LUT.
    if (m_ReadoutState == State::Active || ribbonActive)
    {
      const QRect chip = this->ColormapChipRect();
      if (!m_LutStrip.isNull())
      {
        painter.drawImage(chip, m_LutStrip);
      }
      else
      {
        painter.fillRect(chip, QColor(255, 255, 255, 40));
      }
      painter.setPen(QPen(m_ReadoutState == State::Active ? ActiveText : IdleText, 1));
      painter.drawRect(chip.adjusted(0, 0, -1, -1));
    }
  }

  // Slice/time breadcrumbs: whisper-quiet, revealed with the pointer in the
  // cell (all regions leave Idle together while the pointer is inside).
  const bool pointerInCell = m_BottomState != State::Idle;
  if (pointerInCell && m_SliceSteps > 0)
  {
    painter.setFont(ReadoutFont(this->font()));
    painter.setPen(m_BottomState == State::Active ? ActiveText : IdleText);
    painter.drawText(this->SliceReadoutRect(), Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("%1/%2").arg(m_SlicePosition + 1).arg(m_SliceSteps));

    // Bottom hairline with the slice position tick.
    const int hairlineY = area.bottom() - 1;
    painter.fillRect(QRect(area.left(), hairlineY, area.width(), 1), QColor(255, 255, 255, 40));
    if (m_SliceSteps > 1)
    {
      const int tickX = area.left()
        + static_cast<int>((static_cast<double>(m_SlicePosition) / (m_SliceSteps - 1))
                           * (area.width() - 2));
      painter.fillRect(QRect(tickX, area.bottom() - SliceTickHeight, 2, SliceTickHeight),
                       IdleText);
    }

    // The dashed time hairline exists only for time-resolved data.
    if (m_TimeSteps > 1)
    {
      QPen dashed(QColor(255, 255, 255, 40), 1, Qt::DashLine);
      painter.setPen(dashed);
      const int timeY = area.bottom() - SliceTickHeight - 3;
      painter.drawLine(area.left(), timeY, area.right(), timeY);
      const int tickX = area.left()
        + static_cast<int>((static_cast<double>(m_TimePosition) / (m_TimeSteps - 1))
                           * (area.width() - 2));
      painter.fillRect(QRect(tickX, timeY - 2, 2, 5), IdleText);
    }
  }

  // Collapsed utility toolbar strip along the cell's top edge.
  if (m_TopState != State::Active)
  {
    painter.fillRect(QRect(0, 0, this->width(), TopStripHeight),
                     QColor(255, 255, 255, pointerInCell ? 76 : 40));
  }
}

void QmitkMxNCellOverlay::mousePressEvent(QMouseEvent* event)
{
  const QPoint position = event->pos();

  if (event->button() == Qt::LeftButton && this->ColormapChipRect().contains(position))
  {
    event->accept();
    this->OpenColormapMenu();
    return;
  }

  const QRect ribbon = this->RibbonRect(true);
  if (event->button() == Qt::LeftButton && ribbon.contains(position)
      && m_HasLevelWindow && m_TopNode.IsNotNull())
  {
    if (position.y() < ribbon.top() + RibbonEndHandleHeight)
    {
      m_DragMode = DragMode::UpperBound;
    }
    else if (position.y() > ribbon.bottom() - RibbonEndHandleHeight)
    {
      m_DragMode = DragMode::LowerBound;
    }
    else
    {
      m_DragMode = DragMode::Level;
    }
    m_LastDragPosition = position;
    m_DragScale = this->DragScale();
    event->accept();
    return;
  }

  event->ignore();
}

void QmitkMxNCellOverlay::mouseMoveEvent(QMouseEvent* event)
{
  if (m_DragMode == DragMode::None)
  {
    const QRect ribbon = this->RibbonRect(true);
    this->setCursor(ribbon.contains(event->pos()) ? Qt::SizeVerCursor : Qt::ArrowCursor);
    event->ignore();
    return;
  }

  const int dy = event->pos().y() - m_LastDragPosition.y();
  m_LastDragPosition = event->pos();
  if (dy == 0 || m_TopNode.IsNull())
  {
    event->accept();
    return;
  }

  // Screen-up increases the value; the scale is pinned at drag start so the
  // gesture stays linear while the window changes under it.
  const double valueDelta = -dy * m_DragScale;
  double levelDelta = 0.0;
  double windowDelta = 0.0;

  switch (m_DragMode)
  {
    case DragMode::Level:
      levelDelta = valueDelta;
      break;
    case DragMode::UpperBound:
      levelDelta = valueDelta / 2.0;
      windowDelta = valueDelta;
      break;
    case DragMode::LowerBound:
      levelDelta = valueDelta / 2.0;
      windowDelta = -valueDelta;
      break;
    default:
      break;
  }

  // Keep the window positive; collapsing it to zero would flip the bounds.
  if (m_HasLevelWindow && m_LevelWindow.GetWindow() + windowDelta < 1.0)
  {
    event->accept();
    return;
  }

  try
  {
    m_Editor->AdjustLevelWindow(m_Cell->GetWidgetName(), m_TopNode, levelDelta, windowDelta);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Level/window drag ignored: " << e.GetDescription();
    m_DragMode = DragMode::None;
  }

  // Update the local cache eagerly so consecutive drag steps clamp against
  // the value just written; the render-end refresh re-syncs authoritatively.
  if (m_HasLevelWindow)
  {
    m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel() + levelDelta,
                                 m_LevelWindow.GetWindow() + windowDelta);
    this->update();
  }

  event->accept();
}

void QmitkMxNCellOverlay::mouseReleaseEvent(QMouseEvent* event)
{
  if (m_DragMode != DragMode::None)
  {
    m_DragMode = DragMode::None;
    this->UpdateInteractivity();
    event->accept();
    return;
  }

  event->ignore();
}

void QmitkMxNCellOverlay::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton && this->ReadoutRect().contains(event->pos())
      && m_HasLevelWindow && m_TopNode.IsNotNull())
  {
    event->accept();
    this->OpenNumericEntry();
    return;
  }

  event->ignore();
}

void QmitkMxNCellOverlay::OpenNumericEntry()
{
  auto* popup = new QFrame(this, Qt::Popup);
  popup->setAttribute(Qt::WA_DeleteOnClose);
  popup->setFrameShape(QFrame::StyledPanel);

  auto* layout = new QFormLayout(popup);
  layout->setContentsMargins(6, 6, 6, 6);

  auto* levelBox = new QDoubleSpinBox(popup);
  levelBox->setRange(-1.0e9, 1.0e9);
  levelBox->setDecimals(1);
  levelBox->setValue(m_LevelWindow.GetLevel());

  auto* windowBox = new QDoubleSpinBox(popup);
  windowBox->setRange(1.0e-3, 1.0e9);
  windowBox->setDecimals(1);
  windowBox->setValue(m_LevelWindow.GetWindow());

  layout->addRow(tr("Level"), levelBox);
  layout->addRow(tr("Window"), windowBox);

  const auto commit = [this, levelBox, windowBox]()
  {
    if (m_TopNode.IsNull())
    {
      return;
    }
    try
    {
      m_Editor->SetLevelWindow(m_Cell->GetWidgetName(), m_TopNode,
                               mitk::LevelWindow(levelBox->value(), windowBox->value()));
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Numeric level/window entry ignored: " << e.GetDescription();
    }
  };
  connect(levelBox, &QDoubleSpinBox::editingFinished, this, commit);
  connect(windowBox, &QDoubleSpinBox::editingFinished, this, commit);

  const QRect readout = this->ReadoutRect();
  popup->adjustSize();
  popup->move(this->mapToGlobal(QPoint(readout.left(), readout.top() - popup->sizeHint().height() - 4)));
  popup->show();
  levelBox->setFocus();
  levelBox->selectAll();
}

void QmitkMxNCellOverlay::OpenColormapMenu()
{
  if (m_TopNode.IsNull())
  {
    return;
  }

  QMenu menu(this);
  const auto& names = mitk::LookupTable::typenameList;
  for (std::size_t i = 0; i < names.size(); ++i)
  {
    auto* action = menu.addAction(QString::fromStdString(names[i]));
    action->setData(static_cast<int>(i));
  }

  auto* chosen = menu.exec(this->mapToGlobal(this->ColormapChipRect().bottomLeft()));
  if (nullptr == chosen)
  {
    return;
  }

  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(static_cast<mitk::LookupTable::LookupTableType>(chosen->data().toInt()));

  try
  {
    m_Editor->SetLookupTable(m_Cell->GetWidgetName(), m_TopNode, lookupTable);
  }
  catch (const mitk::Exception& e)
  {
    MITK_WARN << "Colormap selection ignored: " << e.GetDescription();
  }
}

double QmitkMxNCellOverlay::DragScale() const
{
  const int height = std::max(1, this->RenderWindowRect().height());
  const double window = m_HasLevelWindow ? std::max(1.0, m_LevelWindow.GetWindow()) : 256.0;
  return window / height;
}

void QmitkMxNCellOverlay::OnVtkRenderEnd(vtkObject* /*caller*/, unsigned long /*eventId*/,
                                         void* clientData, void* /*callData*/)
{
  static_cast<QmitkMxNCellOverlay*>(clientData)->ScheduleValueRefresh();
}

void QmitkMxNCellOverlay::ScheduleValueRefresh()
{
  if (m_RefreshPending)
  {
    return;
  }

  m_RefreshPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    m_RefreshPending = false;
    this->RefreshValues();
  });
}

void QmitkMxNCellOverlay::RefreshValues()
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }

  const auto node = this->ResolveTopImageNode();

  mitk::LevelWindow levelWindow;
  const bool hasLevelWindow = node.IsNotNull() && node->GetLevelWindow(levelWindow, renderer);

  vtkMTimeType lutMTime = 0;
  if (node.IsNotNull())
  {
    if (auto* lutProperty = dynamic_cast<mitk::LookupTableProperty*>(
          node->GetProperty("LookupTable", renderer)))
    {
      if (lutProperty->GetLookupTable() != nullptr
          && lutProperty->GetLookupTable()->GetVtkLookupTable() != nullptr)
      {
        lutMTime = lutProperty->GetLookupTable()->GetVtkLookupTable()->GetMTime();
      }
    }
  }

  unsigned int slicePosition = 0;
  unsigned int sliceSteps = 0;
  if (auto* sliceStepper = renderer->GetSliceNavigationController()->GetStepper())
  {
    slicePosition = sliceStepper->GetPos();
    sliceSteps = sliceStepper->GetSteps();
  }
  unsigned int timePosition = 0;
  unsigned int timeSteps = 0;
  if (auto* timeNavigation = mitk::RenderingManager::GetInstance()->GetTimeNavigationController())
  {
    if (auto* timeStepper = timeNavigation->GetStepper())
    {
      timePosition = timeStepper->GetPos();
      timeSteps = timeStepper->GetSteps();
    }
  }

  const bool changed = node != m_TopNode || hasLevelWindow != m_HasLevelWindow
    || lutMTime != m_LutMTime
    || slicePosition != m_SlicePosition || sliceSteps != m_SliceSteps
    || timePosition != m_TimePosition || timeSteps != m_TimeSteps
    || (hasLevelWindow
        && (levelWindow.GetLevel() != m_LevelWindow.GetLevel()
            || levelWindow.GetWindow() != m_LevelWindow.GetWindow()));

  if (!changed)
  {
    return;
  }

  m_TopNode = node;
  m_HasLevelWindow = hasLevelWindow;
  if (hasLevelWindow)
  {
    m_LevelWindow = levelWindow;
  }
  m_LutMTime = lutMTime;
  m_SlicePosition = slicePosition;
  m_SliceSteps = sliceSteps;
  m_TimePosition = timePosition;
  m_TimeSteps = timeSteps;

  this->RebuildLutStrip();
  this->UpdateInteractivity();
  this->update();
}

mitk::DataNode::Pointer QmitkMxNCellOverlay::ResolveTopImageNode() const
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer || nullptr == renderer->GetDataStorage())
  {
    return nullptr;
  }

  const auto imageNodes = renderer->GetDataStorage()->GetSubset(
    mitk::NodePredicateDataType::New("Image"));

  mitk::DataNode::Pointer topNode;
  int topLayer = std::numeric_limits<int>::min();
  for (const auto& node : *imageNodes)
  {
    if (node.IsNull() || !node->IsVisible(renderer))
    {
      continue;
    }
    int layer = 0;
    node->GetIntProperty("layer", layer, renderer);
    if (topNode.IsNull() || layer > topLayer)
    {
      topNode = node;
      topLayer = layer;
    }
  }

  return topNode;
}

void QmitkMxNCellOverlay::RebuildLutStrip()
{
  m_LutStrip = QImage();

  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (m_TopNode.IsNull() || nullptr == renderer)
  {
    return;
  }

  auto* lutProperty = dynamic_cast<mitk::LookupTableProperty*>(
    m_TopNode->GetProperty("LookupTable", renderer));
  vtkLookupTable* lookupTable = nullptr;
  if (nullptr != lutProperty && lutProperty->GetLookupTable() != nullptr)
  {
    lookupTable = lutProperty->GetLookupTable()->GetVtkLookupTable();
  }

  constexpr int Samples = 256;
  QImage strip(1, Samples, QImage::Format_RGB32);
  for (int i = 0; i < Samples; ++i)
  {
    // Top row = highest value, matching the ribbon's top = upper bound.
    const double t = 1.0 - static_cast<double>(i) / (Samples - 1);
    QColor color;
    if (nullptr != lookupTable)
    {
      const double* range = lookupTable->GetRange();
      double rgb[3] = { 0.0, 0.0, 0.0 };
      lookupTable->GetColor(range[0] + t * (range[1] - range[0]), rgb);
      color = QColor::fromRgbF(rgb[0], rgb[1], rgb[2]);
    }
    else
    {
      // No LUT property: the mapper renders grayscale through level/window.
      color = QColor::fromRgbF(t, t, t);
    }
    strip.setPixel(0, i, color.rgb());
  }

  m_LutStrip = strip;
}

bool QmitkMxNCellOverlay::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Cell->GetRenderWindow())
  {
    if (event->type() == QEvent::MouseButtonPress
        && static_cast<QMouseEvent*>(event)->button() == Qt::RightButton)
    {
      m_RightPressPosition = static_cast<QMouseEvent*>(event)->pos();
    }
    else if (event->type() == QEvent::ContextMenu)
    {
      // The right button doubles as the zoom / windowing gesture; only a
      // click without drag is a context-menu request. Consuming the event in
      // both cases keeps a gesture's release from popping the menu.
      auto* contextEvent = static_cast<QContextMenuEvent*>(event);
      if (!m_CleanView
          && (contextEvent->pos() - m_RightPressPosition).manhattanLength()
               < QApplication::startDragDistance())
      {
        this->OpenContextMenu(contextEvent->globalPos());
      }
      return true;
    }
  }

  return QmitkOverlayWidget::eventFilter(watched, event);
}

void QmitkMxNCellOverlay::OpenContextMenu(const QPoint& globalPosition)
{
  auto* renderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
  if (nullptr == renderer)
  {
    return;
  }
  const auto windowId = m_Cell->GetWidgetName();

  QMenu menu(this);

  // Per-renderer data visibility: toggles affect only this cell.
  if (auto dataStorage = renderer->GetDataStorage(); dataStorage.IsNotNull())
  {
    const auto noHelperObjects = mitk::NodePredicateAnd::New();
    noHelperObjects->AddPredicate(
      mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
    noHelperObjects->AddPredicate(
      mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));

    auto* visibilityMenu = menu.addMenu(tr("Shown data"));
    for (const auto& node : *dataStorage->GetSubset(noHelperObjects))
    {
      if (node.IsNull())
      {
        continue;
      }
      auto* action = visibilityMenu->addAction(QString::fromStdString(node->GetName()));
      action->setCheckable(true);
      action->setChecked(node->IsVisible(renderer));
      mitk::DataNode::Pointer heldNode = node;
      connect(action, &QAction::toggled, this, [this, heldNode](bool visible)
      {
        auto* localRenderer = mitk::BaseRenderer::GetInstance(m_VtkRenderWindow);
        if (nullptr != localRenderer)
        {
          heldNode->SetVisibility(visible, localRenderer);
          mitk::RenderingManager::GetInstance()->RequestUpdate(m_VtkRenderWindow);
        }
      });
    }
    visibilityMenu->setEnabled(!visibilityMenu->isEmpty());
  }

  auto* directionMenu = menu.addMenu(tr("View direction"));
  const std::pair<const char*, mitk::AnatomicalPlane> planes[] = {
    { "Axial", mitk::AnatomicalPlane::Axial },
    { "Coronal", mitk::AnatomicalPlane::Coronal },
    { "Sagittal", mitk::AnatomicalPlane::Sagittal },
  };
  for (const auto& [label, plane] : planes)
  {
    auto* action = directionMenu->addAction(tr(label));
    const auto planeValue = plane;
    connect(action, &QAction::triggered, this, [this, windowId, planeValue]()
    {
      try
      {
        m_Editor->SetViewDirection(windowId, planeValue);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Context menu: view direction ignored: " << e.GetDescription();
      }
    });
  }

  menu.addSeparator();

  auto* reinitAction = menu.addAction(tr("Reinit group geometry"));
  connect(reinitAction, &QAction::triggered, this, [this, windowId]()
  {
    try
    {
      m_Editor->ReinitSyncGroupGeometry(windowId);
    }
    catch (const mitk::Exception& e)
    {
      MITK_WARN << "Context menu: geometry reinit ignored: " << e.GetDescription();
    }
  });

  auto* reconvergeAction = menu.addAction(tr("Re-converge sync groups"));
  bool hasOffsetLink = false;
  for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                QmitkMxNSyncDimension::Pan })
  {
    hasOffsetLink = hasOffsetLink || m_Editor->GetSyncLink(windowId, dimension).has_value();
  }
  reconvergeAction->setEnabled(hasOffsetLink);
  connect(reconvergeAction, &QAction::triggered, this, [this, windowId]()
  {
    for (const auto dimension : { QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Pan })
    {
      const auto link = m_Editor->GetSyncLink(windowId, dimension);
      if (!link.has_value())
      {
        continue;
      }
      try
      {
        m_Editor->ReconvergeSyncGroup(dimension, link->group);
      }
      catch (const mitk::Exception& e)
      {
        MITK_WARN << "Context menu: re-converge ignored: " << e.GetDescription();
      }
    }
  });

  menu.addSeparator();

  auto* editorAction = menu.addAction(tr("Open layout editor"));
  connect(editorAction, &QAction::triggered, this, [this]()
  {
    m_Editor->RequestLayoutEditor();
  });

  auto* cleanViewAction = menu.addAction(tr("Clean view"));
  cleanViewAction->setCheckable(true);
  cleanViewAction->setChecked(m_Editor->IsCleanView());
  connect(cleanViewAction, &QAction::toggled, this, [this](bool cleanView)
  {
    m_Editor->SetCleanView(cleanView);
  });

  menu.exec(globalPosition);
}
