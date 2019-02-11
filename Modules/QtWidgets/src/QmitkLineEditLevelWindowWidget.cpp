/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkLineEditLevelWindowWidget.h"

// mitk core
#include <mitkRenderingManager.h>

// mitk qt widgets
#include <QmitkLevelWindowWidgetContextMenu.h>

// qt
#include <QLayout>
#include <QLineEdit>
#include <QValidator>

// itk
#include <itkCommand.h>

// c++
#include <limits>
#include <sstream>

QmitkLineEditLevelWindowWidget::QmitkLineEditLevelWindowWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  m_Manager = mitk::LevelWindowManager::New();

  itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command =
    itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  m_IsObserverTagSet = true;

  m_Contextmenu = new QmitkLevelWindowWidgetContextMenu(this);

  auto layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);

  m_LevelInput = new QLineEdit(this);
  m_LevelInput->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
  m_LevelInput->setToolTip("Edit this field to change the center of the levelwindow.");

  m_WindowInput = new QLineEdit(this);
  m_WindowInput->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
  m_WindowInput->setToolTip(
    "Edit this field to change the span of the levelwindow. This number describes the whole span around the center.");

  layout->addWidget(m_LevelInput);
  layout->addWidget(m_WindowInput);

  // signals and slots connections
  connect(m_LevelInput, SIGNAL(editingFinished()), this, SLOT(SetLevelValue()));
  connect(m_WindowInput, SIGNAL(editingFinished()), this, SLOT(SetWindowValue()));

  // Validator for both LineEdit-widgets, to limit the valid input-range to int.
  QValidator *validatorWindowInput = new QDoubleValidator(0, std::numeric_limits<double>::max(), 2, this);
  m_WindowInput->setValidator(validatorWindowInput);

  this->hide();
}

QmitkLineEditLevelWindowWidget::~QmitkLineEditLevelWindowWidget()
{
  if (m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
}

void QmitkLineEditLevelWindowWidget::OnPropertyModified(const itk::EventObject &)
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();
    QString level;
    QString window;
    if (m_LevelWindow.IsFloatingValues())
    {
      std::stringstream ssLevel;
      std::stringstream ssWindow;
      ssLevel << std::setprecision(3) << m_LevelWindow.GetLevel();
      ssWindow << std::setprecision(3) << m_LevelWindow.GetWindow();
      level = ssLevel.str().c_str();
      window = ssWindow.str().c_str();
    }
    else
    {
      level.setNum((int)(m_LevelWindow.GetLevel()));
      window.setNum((int)(m_LevelWindow.GetWindow()));
    }
    m_LevelInput->setText(level);
    m_WindowInput->setText(window);
    m_LevelInput->setEnabled(!m_LevelWindow.IsFixed());
    m_WindowInput->setEnabled(!m_LevelWindow.IsFixed());
    this->show();
  }
  catch (...)
  {
    try
    {
      this->hide();
    }
    catch (...)
    {
    }
  }
}

void QmitkLineEditLevelWindowWidget::SetLevelWindowManager(mitk::LevelWindowManager *levelWindowManager)
{
  if (m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  m_Manager = levelWindowManager;
  if (m_Manager.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command =
      itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
    command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;
  }
}

void QmitkLineEditLevelWindowWidget::SetDataStorage(mitk::DataStorage *ds)
{
  m_Manager->SetDataStorage(ds);
}

void QmitkLineEditLevelWindowWidget::SetLevelValue()
{
  double level = m_LevelInput->text().toDouble();
  m_LevelWindow.SetLevelWindow(level, m_LevelWindow.GetWindow());
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLineEditLevelWindowWidget::SetWindowValue()
{
  double window = m_WindowInput->text().toDouble();
  m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), window);
  m_Manager->SetLevelWindow(m_LevelWindow);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLineEditLevelWindowWidget::contextMenuEvent(QContextMenuEvent *)
{
  m_Contextmenu->SetLevelWindowManager(m_Manager.GetPointer());
  m_Contextmenu->GetContextMenu();
}

mitk::LevelWindowManager *QmitkLineEditLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
