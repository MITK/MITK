/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSelectionWidget.h>

// mitk
#include <mitkLabelSetImageConverter.h>

#include <QmitkMultiLabelInspector.h>
#include <QmitkMultiLabelSelectionLabel.h>

// Qt
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>
#include <QEvent>
#include <QEnterEvent>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QPushButton>

#include <ui_QmitkMultiLabelSelectionWidget.h>


QmitkMultiLabelSelectionWidget::QmitkMultiLabelSelectionWidget(QWidget* parent/* = nullptr*/)
  : QWidget(parent),
  m_Controls(new Ui::QmitkMultiLabelSelectionWidget),
  m_Popup(new QmitkMultiLabelInspectorPopup()),
  m_HoverTimer(new QTimer(this)),
  m_CheckMousePositionTimer(new QTimer(this)),
  m_PopupVisible(false)
{
  m_Controls->setupUi(this);

  m_Controls->labelSelection->SetEmptyInfo(QStringLiteral("<font class=\"warning\">Select label to proceed.</font>"));
  m_Popup->GetInspector()->SetMultiSelectionMode(false);

  m_Controls->labelSelection->installEventFilter(this);
  m_Controls->btnChange->installEventFilter(this);
  // Alternative approach would be to use signal/slot for button:
  // connect(m_Controls->button, &QPushButton::clicked, this, &QmitkMultiLabelSelectionWidget::showPopup);
  // but the event filter provides more direct and unified handling of all options that
  // could trigger the pop-up.

  m_HoverTimer->setSingleShot(true);
  m_HoverTimer->setInterval(200);
  connect(m_HoverTimer.get(), &QTimer::timeout, this, &QmitkMultiLabelSelectionWidget::ShowPopup);

  // Configure mouse position check timer
  // This timer approach creates a more robust solution for handling mouse movements
  // between widgets and prevents unintended pop-up closing
  m_CheckMousePositionTimer->setInterval(100); // Check every 100ms
  connect(m_CheckMousePositionTimer.get(), &QTimer::timeout, this, &QmitkMultiLabelSelectionWidget::CheckMousePosition);

  connect(m_Popup.get(), &QmitkMultiLabelInspectorPopup::SelectionFinished,
    this, &QmitkMultiLabelSelectionWidget::OnSelectionFinished);

  connect(m_Popup->GetInspector(), &QmitkMultiLabelInspector::CurrentSelectionChanged,
    m_Controls->labelSelection, &QmitkMultiLabelSelectionLabel::SetSelectedLabels);

  //// Set popup to be a top-level widget, but not an independent window
  //m_PopupInspector->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
}

QmitkMultiLabelSelectionWidget::~QmitkMultiLabelSelectionWidget()
{
}

bool QmitkMultiLabelSelectionWidget::eventFilter(QObject* watched, QEvent* event)
{
  // Using event filter to provide uniform handling of both widgets (label and button)
  // This allows us to handle both hover and click with a single mechanism
  if (watched == m_Controls->labelSelection &&
    (event->type() == QEvent::Enter))
  {
    m_HoverTimer->start();
    return false;
  }
  else if ((watched == m_Controls->labelSelection || watched == m_Controls->btnChange) &&
    (event->type() == QEvent::MouseButtonPress))
  {
    // Immediately show popup on click, and cancel hover timer to prevent
    // potential race conditions
    m_HoverTimer->stop();
    ShowPopup();
    return false;
  }

  return QWidget::eventFilter(watched, event);
}

void QmitkMultiLabelSelectionWidget::enterEvent(QEnterEvent* event)
{
  QWidget::enterEvent(event);
  m_HoverTimer->start();
}

void QmitkMultiLabelSelectionWidget::leaveEvent(QEvent* event)
{
  QWidget::leaveEvent(event);
  m_HoverTimer->stop();

  // Don't hide immediately on leave, use the check timer
  // to verify the mouse isn't over the popup. This prevents
  // the popup from closing when moving from the main widget to popup
  if (m_PopupVisible)
  {
    m_CheckMousePositionTimer->start();
  }
}

void QmitkMultiLabelSelectionWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if (m_PopupVisible)
  {
    this->PositionPopup();
  }
}

void QmitkMultiLabelSelectionWidget::ShowPopup()
{
  if (!m_PopupVisible)
  {
    this->PositionPopup();
    m_Popup->show();
    m_PopupVisible = true;
    m_CheckMousePositionTimer->start();
  }
}

void QmitkMultiLabelSelectionWidget::HidePopup()
{
  if (m_PopupVisible)
  {
    m_Popup->hide();
    m_PopupVisible = false;
    m_CheckMousePositionTimer->stop();
  }
}

void QmitkMultiLabelSelectionWidget::OnSelectionFinished()
{
  this->HidePopup();
  emit CurrentSelectionChanged(this->GetSelectedLabels());
}

void QmitkMultiLabelSelectionWidget::CheckMousePosition()
{
  // This timer-based approach provides more robust handling of mouse movements
  // than relying solely on enter/leave events. It handles cases where:
  // 1. Mouse moves quickly between widgets
  // 2. Mouse briefly exits both widgets
  // 3. Mouse moves to another application window
  // 4. Normal event propagation is interrupted

  if (!m_PopupVisible)
  {
    m_CheckMousePositionTimer->stop();
    return;
  }

  QPoint globalMousePos = QCursor::pos();
  bool mouseOverWidget = rect().contains(mapFromGlobal(globalMousePos));
  bool mouseOverPopup = m_Popup->rect().contains(m_Popup->mapFromGlobal(globalMousePos));

  if (!mouseOverWidget && !mouseOverPopup)
  {
    this->HidePopup();
  }
}

void QmitkMultiLabelSelectionWidget::OnPopupLabelsChanged(const LabelValueVectorType& selectedLabels)
{
}

void QmitkMultiLabelSelectionWidget::PositionPopup()
{
  QPoint popupPos = mapToGlobal(QPoint(0, height()));

  // Ensure the popup doesn't go off screen
  QRect screenGeometry = QApplication::screenAt(popupPos)->geometry();
  int popupRight = popupPos.x() + width();
  int screenRight = screenGeometry.right();

  if (popupRight > screenRight) {
    popupPos.setX(screenRight - width());
  }

  m_Popup->setFixedWidth(width());
  m_Popup->move(popupPos);
}


void QmitkMultiLabelSelectionWidget::Initialize()
{
}

void QmitkMultiLabelSelectionWidget::SetMultiSelectionMode(bool multiMode)
{
  m_Popup->GetInspector()->SetMultiSelectionMode(multiMode);
}

bool QmitkMultiLabelSelectionWidget::GetMultiSelectionMode() const
{
  return m_Popup->GetInspector()->GetMultiSelectionMode();
}

void QmitkMultiLabelSelectionWidget::SetHighlightingActivated(bool visiblityMod)
{
  m_Controls->labelSelection->SetHighlightingActivated(visiblityMod);
}

bool QmitkMultiLabelSelectionWidget::GetHighlightingActivated() const
{
  return m_Controls->labelSelection->GetHighlightingActivated();
}

void QmitkMultiLabelSelectionWidget::SetEmptyInfo(QString info)
{
  m_Controls->labelSelection->SetEmptyInfo(info);
}

QString QmitkMultiLabelSelectionWidget::GetEmptyInfo() const
{
  return m_Controls->labelSelection->GetEmptyInfo();
}

void QmitkMultiLabelSelectionWidget::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  m_Controls->labelSelection->SetMultiLabelSegmentation(segmentation);
  m_Popup->GetInspector()->SetMultiLabelSegmentation(segmentation);
  this->Initialize();
}

mitk::LabelSetImage* QmitkMultiLabelSelectionWidget::GetMultiLabelSegmentation() const
{
  return m_Controls->labelSelection->GetMultiLabelSegmentation();
}

void QmitkMultiLabelSelectionWidget::SetMultiLabelNode(mitk::DataNode* node)
{
  m_Controls->labelSelection->SetMultiLabelNode(node);
  m_Popup->GetInspector()->SetMultiLabelNode(node);
}

mitk::DataNode* QmitkMultiLabelSelectionWidget::GetMultiLabelNode() const
{
  return m_Controls->labelSelection->GetMultiLabelNode();
}

void QmitkMultiLabelSelectionWidget::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  m_Controls->labelSelection->SetSelectedLabels(selectedLabels);
  m_Popup->GetInspector()->SetSelectedLabels(selectedLabels);
}

void QmitkMultiLabelSelectionWidget::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelSelectionWidget::LabelValueVectorType QmitkMultiLabelSelectionWidget::GetSelectedLabels() const
{
  return m_Popup->GetInspector()->GetSelectedLabels();
}

mitk::Image::Pointer QmitkMultiLabelSelectionWidget::CreateSelectedLabelMask() const
{
  if (this->GetMultiSelectionMode())
  {
    mitkThrow() << "Currently CreateSelectedLabelMask is only implemented for QmitkMultiLabelSelectionWidget::GetMultiSelectionMode()==false.";
  }

  if (nullptr != this->GetMultiLabelSegmentation() && !this->GetSelectedLabels().empty())
    return mitk::CreateLabelMask(this->GetMultiLabelSegmentation(), this->GetSelectedLabels().front());

  return nullptr;
}




QmitkMultiLabelInspectorPopup::QmitkMultiLabelInspectorPopup(QWidget* parent)
  : QWidget(parent)
  , m_Inspector(new QmitkMultiLabelInspector)
{
  // Setup layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(1, 1, 1, 1);
  layout->addWidget(m_Inspector.get());

  m_Inspector->SetAllowLabelModification(false);
  m_Inspector->SetAllowLockModification(false);
  m_Inspector->SetAllowVisibilityModification(false);

  // Set size policy
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  // Set popup to be a top-level widget, but not an independent window
  setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

  connect(m_Inspector.get(), &QmitkMultiLabelInspector::CurrentSelectionChanged,
    this, &QmitkMultiLabelInspectorPopup::OnPopupLabelsChanged);

}

QmitkMultiLabelInspectorPopup::~QmitkMultiLabelInspectorPopup() = default;

QmitkMultiLabelInspector* QmitkMultiLabelInspectorPopup::GetInspector() const
{
  return m_Inspector.get();
}

void QmitkMultiLabelInspectorPopup::OnPopupLabelsChanged(const LabelValueVectorType& selectedLabels)
{
  if (!m_Inspector->GetMultiSelectionMode())
  { //if we only do single selection, we can emit finish as soon as the selection is changed.
    emit SelectionFinished();
  }
}

void QmitkMultiLabelInspectorPopup::leaveEvent(QEvent* event)
{
  // This leaveEvent provides one way to close the popup - when mouse
  // explicitly leaves the popup boundaries. However, it only handles
  // this specific case.
  //
  // The parent widget also uses a timer-based approach (checkMousePosition)
  // for more robust handling of various edge cases where the mouse might
  // be moving between widgets or briefly exit both.
  //QWidget::leaveEvent(event);
  //emit selectionFinished();
}