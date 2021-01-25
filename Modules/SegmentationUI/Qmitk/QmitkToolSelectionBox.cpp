/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//#define MBILOG_ENABLE_DEBUG 1

#include <QmitkStyleManager.h>
#include "QmitkToolSelectionBox.h"
#include "QmitkToolGUI.h"
#include "mitkBaseRenderer.h"

#include <QList>
#include <qapplication.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <queue>

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include "mitkToolManagerProvider.h"

QmitkToolSelectionBox::QmitkToolSelectionBox(QWidget *parent, mitk::DataStorage *)
  : QWidget(parent),
    m_SelfCall(false),
    m_DisplayedGroups("default"),
    m_LayoutColumns(2),
    m_ShowNames(true),
    m_GenerateAccelerators(false),
    m_ToolGUIWidget(nullptr),
    m_LastToolGUI(nullptr),
    m_ToolButtonGroup(nullptr),
    m_ButtonLayout(nullptr),
    m_EnabledMode(EnabledWithReferenceAndWorkingDataVisible)
{
  QFont currentFont = QWidget::font();
  currentFont.setBold(true);
  QWidget::setFont(currentFont);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  // QButtonGroup
  m_ToolButtonGroup = new QButtonGroup(this);
  // some features of QButtonGroup
  m_ToolButtonGroup->setExclusive(false); // mutually exclusive toggle buttons

  RecreateButtons();

  QWidget::setContentsMargins(0, 0, 0, 0);
  if (layout() != nullptr)
  {
    layout()->setContentsMargins(0, 0, 0, 0);
  }

  // reactions to signals
  connect(m_ToolButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(toolButtonClicked(int)));

  // reactions to ToolManager events

  m_ToolManager->ActiveToolChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerToolModified);
  m_ToolManager->ReferenceDataChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified);
  m_ToolManager->WorkingDataChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified);

  // show active tool
  SetOrUnsetButtonForActiveTool();

  QWidget::setEnabled(false);
}

QmitkToolSelectionBox::~QmitkToolSelectionBox()
{
  m_ToolManager->ActiveToolChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerToolModified);
  m_ToolManager->ReferenceDataChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified);
  m_ToolManager->WorkingDataChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified);
}

void QmitkToolSelectionBox::SetEnabledMode(EnabledMode mode)
{
  m_EnabledMode = mode;
  SetGUIEnabledAccordingToToolManagerState();
}

mitk::ToolManager *QmitkToolSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolSelectionBox::SetToolManager(
  mitk::ToolManager &newManager) // no nullptr pointer allowed here, a manager is required
{
  // say bye to the old manager
  m_ToolManager->ActiveToolChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerToolModified);
  m_ToolManager->ReferenceDataChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified);
  m_ToolManager->WorkingDataChanged -=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified);

  if (QWidget::isEnabled())
  {
    m_ToolManager->UnregisterClient();
  }

  m_ToolManager = &newManager;
  RecreateButtons();

  // greet the new one
  m_ToolManager->ActiveToolChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerToolModified);
  m_ToolManager->ReferenceDataChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified);
  m_ToolManager->WorkingDataChanged +=
    mitk::MessageDelegate<QmitkToolSelectionBox>(this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified);

  if (QWidget::isEnabled())
  {
    m_ToolManager->RegisterClient();
  }

  // ask the new one what the situation is like
  SetOrUnsetButtonForActiveTool();
}

void QmitkToolSelectionBox::toolButtonClicked(int id)
{
  if (!QWidget::isEnabled())
    return; // this method could be triggered from the constructor, when we are still disabled

  MITK_DEBUG << "toolButtonClicked(" << id << "): id translates to tool ID " << m_ToolIDForButtonID[id];

  QToolButton *toolButton = dynamic_cast<QToolButton *>(m_ToolButtonGroup->buttons().at(id));
  if (toolButton)
  {
    if ((m_ButtonIDForToolID.find(m_ToolManager->GetActiveToolID()) !=
         m_ButtonIDForToolID.end()) // if we have this tool in our box
        &&
        (m_ButtonIDForToolID[m_ToolManager->GetActiveToolID()] ==
         id)) // the tool corresponding to this button is already active
    {
      // disable this button, disable all tools
      toolButton->setChecked(false);
      m_ToolManager->ActivateTool(-1); // disable everything
    }
    else
    {
      // enable the corresponding tool
      m_SelfCall = true;

      m_ToolManager->ActivateTool(m_ToolIDForButtonID[id]);

      m_SelfCall = false;
    }
  }
}

void QmitkToolSelectionBox::OnToolManagerToolModified()
{
  SetOrUnsetButtonForActiveTool();
}

void QmitkToolSelectionBox::SetOrUnsetButtonForActiveTool()
{
  // we want to emit a signal in any case, whether we selected ourselves or somebody else changes "our" tool manager.
  // --> emit before check on m_SelfCall
  int id = m_ToolManager->GetActiveToolID();

  // don't emit signal for shape model tools
  bool emitSignal = true;
  mitk::Tool *tool = m_ToolManager->GetActiveTool();
  if (tool && std::string(tool->GetGroup()) == "organ_segmentation")
    emitSignal = false;

  if (emitSignal)
    emit ToolSelected(id);

  // delete old GUI (if any)
  if (m_LastToolGUI && m_ToolGUIWidget)
  {
    if (m_ToolGUIWidget->layout())
    {
      m_ToolGUIWidget->layout()->removeWidget(m_LastToolGUI);
    }

    m_LastToolGUI->setParent(nullptr);
    delete m_LastToolGUI; // will hopefully notify parent and layouts
    m_LastToolGUI = nullptr;

    QLayout *layout = m_ToolGUIWidget->layout();
    if (layout)
    {
      layout->activate();
    }
  }

  QToolButton *toolButton(nullptr);

  if (m_ButtonIDForToolID.find(id) != m_ButtonIDForToolID.end()) // if this tool is in our box
  {
    toolButton = dynamic_cast<QToolButton *>(m_ToolButtonGroup->buttons().at(m_ButtonIDForToolID[id]));
  }

  if (toolButton)
  {
    // mmueller
    // uncheck all other buttons
    QAbstractButton *tmpBtn = nullptr;
    QList<QAbstractButton *>::iterator it;
    for (int i = 0; i < m_ToolButtonGroup->buttons().size(); ++i)
    {
      tmpBtn = m_ToolButtonGroup->buttons().at(i);
      if (tmpBtn != toolButton)
        dynamic_cast<QToolButton *>(tmpBtn)->setChecked(false);
    }

    toolButton->setChecked(true);

    if (m_ToolGUIWidget && tool)
    {
      // create and reparent new GUI (if any)
      itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI").GetPointer(); // prefix and postfix

      if (possibleGUI.IsNull())
        possibleGUI = tool->GetGUI("", "GUI").GetPointer();

      QmitkToolGUI *gui = dynamic_cast<QmitkToolGUI *>(possibleGUI.GetPointer());

      //!
      m_LastToolGUI = gui;
      if (gui)
      {
        gui->SetTool(tool);

        gui->setParent(m_ToolGUIWidget);
        gui->move(gui->geometry().topLeft());
        gui->show();

        QLayout *layout = m_ToolGUIWidget->layout();
        if (!layout)
        {
          layout = new QVBoxLayout(m_ToolGUIWidget);
        }
        if (layout)
        {
          layout->addWidget(gui);
          layout->activate();
        }
      }
    }
  }
  else
  {
    // disable all buttons
    QToolButton *selectedToolButton = dynamic_cast<QToolButton *>(m_ToolButtonGroup->checkedButton());
    if (selectedToolButton)
    {
      selectedToolButton->setChecked(false);
    }
  }
}

void QmitkToolSelectionBox::OnToolManagerReferenceDataModified()
{
  if (m_SelfCall)
    return;

  MITK_DEBUG << "OnToolManagerReferenceDataModified()";

  this->UpdateButtonsEnabledState();
  this->SetGUIEnabledAccordingToToolManagerState();
}

void QmitkToolSelectionBox::OnToolManagerWorkingDataModified()
{
  if (m_SelfCall)
    return;

  MITK_DEBUG << "OnToolManagerWorkingDataModified()";

  this->UpdateButtonsEnabledState();
  this->SetGUIEnabledAccordingToToolManagerState();
}

/**
 Implementes the logic, which decides, when tools are activated/deactivated.
*/
void QmitkToolSelectionBox::SetGUIEnabledAccordingToToolManagerState()
{
  mitk::DataNode *referenceNode = m_ToolManager->GetReferenceData(0);
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);

  bool enabled = true;

  switch (m_EnabledMode)
  {
    default:
    case EnabledWithReferenceAndWorkingDataVisible:
      enabled = referenceNode && workingNode &&
                referenceNode->IsVisible(
                  mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget0"))) &&
                workingNode->IsVisible(
                  mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget0"))) &&
                isVisible();
      break;
    case EnabledWithReferenceData:
      enabled = referenceNode && isVisible();
      break;
    case EnabledWithWorkingData:
      enabled = workingNode && isVisible();
      break;
    case AlwaysEnabled:
      enabled = isVisible();
      break;
  }

  if (QWidget::isEnabled() == enabled)
    return; // nothing to change

  QWidget::setEnabled(enabled);
  if (enabled)
  {
    m_ToolManager->RegisterClient();

    int id = m_ToolManager->GetActiveToolID();
    emit ToolSelected(id);
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
    m_ToolManager->UnregisterClient();

    emit ToolSelected(-1);
  }
}

/**
 External enableization...
*/
void QmitkToolSelectionBox::setEnabled(bool /*enable*/)
{
  SetGUIEnabledAccordingToToolManagerState();
}

void QmitkToolSelectionBox::UpdateButtonsEnabledState()
{
  auto buttons = m_ToolButtonGroup->buttons();

  const auto refDataNode = m_ToolManager->GetReferenceData(0);
  const mitk::BaseData* refData = nullptr;
  if (nullptr != refDataNode)
  {
    refData = refDataNode->GetData();
  }

  const auto workingDataNode = m_ToolManager->GetWorkingData(0);
  const mitk::BaseData* workingData = nullptr;
  if (nullptr != workingDataNode)
  {
    workingData = workingDataNode->GetData();
  }

  for (const auto& button : qAsConst(buttons))
  {
    const auto buttonID = m_ToolButtonGroup->id(button);
    const auto toolID = m_ToolIDForButtonID[buttonID];
    const auto tool = m_ToolManager->GetToolById(toolID);

    button->setEnabled(tool->CanHandle(refData, workingData));
  }
}

void QmitkToolSelectionBox::RecreateButtons()
{
  if (m_ToolManager.IsNull())
    return;

  QList<QAbstractButton *> l = m_ToolButtonGroup->buttons();
  // remove all buttons that are there
  QList<QAbstractButton *>::iterator it;
  QAbstractButton *btn;

  for (it = l.begin(); it != l.end(); ++it)
  {
    btn = *it;
    m_ToolButtonGroup->removeButton(btn);
    delete btn;
  }

  mitk::ToolManager::ToolVectorTypeConst allPossibleTools = m_ToolManager->GetTools();
  mitk::ToolManager::ToolVectorTypeConst allTools;

  typedef std::pair<std::string::size_type, const mitk::Tool *> SortPairType;
  typedef std::priority_queue<SortPairType> SortedToolQueueType;
  SortedToolQueueType toolPositions;

  // clear and sort all tools
  // step one: find name/group of all tools in m_DisplayedGroups string. remember these positions for all tools.
  for (mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allPossibleTools.begin();
       iter != allPossibleTools.end();
       ++iter)
  {
    const mitk::Tool *tool = *iter;

    std::string::size_type namePos = m_DisplayedGroups.find(std::string("'") + tool->GetName() + "'");
    std::string::size_type groupPos = m_DisplayedGroups.find(std::string("'") + tool->GetGroup() + "'");

    if (!m_DisplayedGroups.empty() && namePos == std::string::npos && groupPos == std::string::npos)
      continue; // skip

    if (m_DisplayedGroups.empty() && std::string(tool->GetName()).length() > 0)
    {
      namePos = static_cast<std::string::size_type>(tool->GetName()[0]);
    }

    SortPairType thisPair = std::make_pair(namePos < groupPos ? namePos : groupPos, *iter);
    toolPositions.push(thisPair);
  }

  // step two: sort tools according to previously found positions in m_DisplayedGroups
  MITK_DEBUG << "Sorting order of tools (lower number --> earlier in button group)";
  while (!toolPositions.empty())
  {
    SortPairType thisPair = toolPositions.top();
    MITK_DEBUG << "Position " << thisPair.first << " : " << thisPair.second->GetName();

    allTools.push_back(thisPair.second);
    toolPositions.pop();
  }
  std::reverse(allTools.begin(), allTools.end());

  MITK_DEBUG << "Sorted tools:";
  for (mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin(); iter != allTools.end(); ++iter)
  {
    MITK_DEBUG << (*iter)->GetName();
  }

  if (m_ButtonLayout == nullptr)
    m_ButtonLayout = new QGridLayout;

  int row(0);
  int column(-1);

  int currentButtonID(0);
  m_ButtonIDForToolID.clear();
  m_ToolIDForButtonID.clear();
  QToolButton *button = nullptr;

  MITK_DEBUG << "Creating buttons for tools";
  // fill group box with buttons
  for (mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin(); iter != allTools.end(); ++iter)
  {
    const mitk::Tool *tool = *iter;
    int currentToolID(m_ToolManager->GetToolID(tool));

    ++column;
    // new line if we are at the maximum columns
    if (column == m_LayoutColumns)
    {
      ++row;
      column = 0;
    }

    button = new QToolButton;
    button->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    // add new button to the group
    MITK_DEBUG << "Adding button with ID " << currentToolID;
    m_ToolButtonGroup->addButton(button, currentButtonID);
    // ... and to the layout
    MITK_DEBUG << "Adding button in row/column " << row << "/" << column;
    m_ButtonLayout->addWidget(button, row, column);

    if (m_LayoutColumns == 1)
    {
      button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else
    {
      button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    }

    button->setCheckable(true);

    if (currentToolID == m_ToolManager->GetActiveToolID())
      button->setChecked(true);

    QString label;
    if (m_GenerateAccelerators)
    {
      label += "&";
    }
    label += tool->GetName();
    QString tooltip = tool->GetName();
    MITK_DEBUG << tool->GetName() << ", " << label.toLocal8Bit().constData() << ", '"
               << tooltip.toLocal8Bit().constData();

    if (m_ShowNames)
    {
      button->setText(label); // a label
      button->setToolTip(tooltip);

      QFont currentFont = button->font();
      currentFont.setBold(false);
      button->setFont(currentFont);
    }

    us::ModuleResource iconResource = tool->GetIconResource();

    if (!iconResource.IsValid())
    {
      button->setIcon(QIcon(QPixmap(tool->GetXPM())));
    }
    else
    {
      auto isSVG = "svg" == iconResource.GetSuffix();
      auto openmode = isSVG ? std::ios_base::in : std::ios_base::binary;

      us::ModuleResourceStream resourceStream(iconResource, openmode);
      resourceStream.seekg(0, std::ios::end);
      std::ios::pos_type length = resourceStream.tellg();
      resourceStream.seekg(0, std::ios::beg);

      char *data = new char[length];
      resourceStream.read(data, length);

      if (isSVG)
      {
        button->setIcon(QmitkStyleManager::ThemeIcon(QByteArray::fromRawData(data, length)));
      }
      else
      {
        QPixmap pixmap;
        pixmap.loadFromData(QByteArray::fromRawData(data, length));
        button->setIcon(QIcon(pixmap));
      }

      delete[] data;

      if (m_ShowNames)
      {
        if (m_LayoutColumns == 1)
          button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        else
          button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        button->setIconSize(QSize(24, 24));
      }
      else
      {
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setIconSize(QSize(32, 32));
        button->setToolTip(tooltip);
      }
    }

    if (m_GenerateAccelerators)
    {
      QString firstLetter = QString(tool->GetName());
      firstLetter.truncate(1);
      button->setShortcut(
        firstLetter); // a keyboard shortcut (just the first letter of the given name w/o any CTRL or something)
    }

    m_ButtonIDForToolID[currentToolID] = currentButtonID;
    m_ToolIDForButtonID[currentButtonID] = currentToolID;

    MITK_DEBUG << "m_ButtonIDForToolID[" << currentToolID << "] == " << currentButtonID;
    MITK_DEBUG << "m_ToolIDForButtonID[" << currentButtonID << "] == " << currentToolID;

    tool->GUIProcessEventsMessage += mitk::MessageDelegate<QmitkToolSelectionBox>(
      this, &QmitkToolSelectionBox::OnToolGUIProcessEventsMessage); // will never add a listener twice, so we don't have
                                                                    // to check here
    tool->ErrorMessage += mitk::MessageDelegate1<QmitkToolSelectionBox, std::string>(
      this,
      &QmitkToolSelectionBox::OnToolErrorMessage); // will never add a listener twice, so we don't have to check here
    tool->GeneralMessage +=
      mitk::MessageDelegate1<QmitkToolSelectionBox, std::string>(this, &QmitkToolSelectionBox::OnGeneralToolMessage);

    ++currentButtonID;
  }
  // setting grid layout for this groupbox
  this->setLayout(m_ButtonLayout);

  this->UpdateButtonsEnabledState();
  // this->update();
}

void QmitkToolSelectionBox::OnToolGUIProcessEventsMessage()
{
  qApp->processEvents();
}

void QmitkToolSelectionBox::OnToolErrorMessage(std::string s)
{
  QMessageBox::critical(
    this, "MITK", QString(s.c_str()), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void QmitkToolSelectionBox::OnGeneralToolMessage(std::string s)
{
  QMessageBox::information(
    this, "MITK", QString(s.c_str()), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void QmitkToolSelectionBox::SetDisplayedToolGroups(const std::string &toolGroups)
{
  if (m_DisplayedGroups != toolGroups)
  {
    QString q_DisplayedGroups = toolGroups.c_str();
    // quote all unquoted single words
    q_DisplayedGroups = q_DisplayedGroups.replace(QRegExp("\\b(\\w+)\\b|'([^']+)'"), "'\\1\\2'");
    MITK_DEBUG << "m_DisplayedGroups was \"" << toolGroups << "\"";

    m_DisplayedGroups = q_DisplayedGroups.toLocal8Bit().constData();
    MITK_DEBUG << "m_DisplayedGroups is \"" << m_DisplayedGroups << "\"";

    RecreateButtons();
    SetOrUnsetButtonForActiveTool();
  }
}

void QmitkToolSelectionBox::SetLayoutColumns(int columns)
{
  if (columns > 0 && columns != m_LayoutColumns)
  {
    m_LayoutColumns = columns;
    RecreateButtons();
  }
}

void QmitkToolSelectionBox::SetShowNames(bool show)
{
  if (show != m_ShowNames)
  {
    m_ShowNames = show;
    RecreateButtons();
  }
}

void QmitkToolSelectionBox::SetGenerateAccelerators(bool accel)
{
  if (accel != m_GenerateAccelerators)
  {
    m_GenerateAccelerators = accel;
    RecreateButtons();
  }
}

void QmitkToolSelectionBox::SetToolGUIArea(QWidget *parentWidget)
{
  m_ToolGUIWidget = parentWidget;
}

void QmitkToolSelectionBox::setTitle(const QString & /*title*/)
{
}

void QmitkToolSelectionBox::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);
  SetGUIEnabledAccordingToToolManagerState();
}

void QmitkToolSelectionBox::hideEvent(QHideEvent *e)
{
  QWidget::hideEvent(e);
  SetGUIEnabledAccordingToToolManagerState();
}
