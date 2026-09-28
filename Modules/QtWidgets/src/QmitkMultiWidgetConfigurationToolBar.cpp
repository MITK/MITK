/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiWidgetConfigurationToolBar.h>

// mitk qt widgets module
#include <QmitkAbstractMultiWidget.h>
#include <QmitkIconTheme.h>
#include <QmitkMultiWidgetLayoutSelectionWidget.h>

QmitkMultiWidgetConfigurationToolBar::QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget)
  : QToolBar(multiWidget)
  , m_MultiWidget(multiWidget)
{
  QToolBar::setOrientation(Qt::Vertical);

  InitializeToolBar();
}

QmitkMultiWidgetConfigurationToolBar::~QmitkMultiWidgetConfigurationToolBar()
{
  // nothing here
}

void QmitkMultiWidgetConfigurationToolBar::InitializeToolBar()
{
  // create popup to show a widget to modify the multi widget layout
  m_LayoutSelectionPopup = new QmitkMultiWidgetLayoutSelectionWidget(this);
  m_LayoutSelectionPopup->hide();

  AddButtons();

  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet, this, &QmitkMultiWidgetConfigurationToolBar::LayoutSet);
  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout, this, &QmitkMultiWidgetConfigurationToolBar::SetDataBasedLayout);
  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::SaveLayout, this, &QmitkMultiWidgetConfigurationToolBar::SaveLayout);
  connect(m_LayoutSelectionPopup, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout, this, &QmitkMultiWidgetConfigurationToolBar::LoadLayout);
}

void QmitkMultiWidgetConfigurationToolBar::AddButtons()
{
  QAction* setLayoutAction = new QAction(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwLayout.svg")), tr("Set multi widget layout"), this);
  connect(setLayoutAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSetLayout);
  QToolBar::addAction(setLayoutAction);

  m_SynchronizeAction = new QAction(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwDesynchronized.svg")), tr("Synchronize render windows"), this);
  m_SynchronizeAction->setCheckable(true);
  m_SynchronizeAction->setChecked(false);
  connect(m_SynchronizeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnSynchronize);
  QToolBar::addAction(m_SynchronizeAction);

  m_InteractionSchemeChangeAction = new QAction(this);
  m_InteractionSchemeChangeAction->setCheckable(true);
  this->UpdateInteractionSchemeAction(false);
  connect(m_InteractionSchemeChangeAction, &QAction::triggered, this, &QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged);
  QToolBar::addAction(m_InteractionSchemeChangeAction);
}

void QmitkMultiWidgetConfigurationToolBar::UpdateInteractionSchemeAction(bool pacs)
{
  m_InteractionSchemeChangeAction->setChecked(pacs);

  if (pacs)
  {
    m_InteractionSchemeChangeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwPACS.svg")));
    m_InteractionSchemeChangeAction->setText(tr("Change to MITK interaction"));
  }
  else
  {
    m_InteractionSchemeChangeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwMITK.svg")));
    m_InteractionSchemeChangeAction->setText(tr("Change to PACS interaction"));
  }
}

void QmitkMultiWidgetConfigurationToolBar::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (m_LayoutSelectionPopup == nullptr)
    return;
  m_LayoutSelectionPopup->SetDataStorage(dataStorage);
}

void QmitkMultiWidgetConfigurationToolBar::OnSetLayout()
{
  if (nullptr != m_MultiWidget)
  {
    m_LayoutSelectionPopup->setWindowFlags(Qt::Popup);
    m_LayoutSelectionPopup->move(this->cursor().pos().x() - m_LayoutSelectionPopup->width(), this->cursor().pos().y());
    m_LayoutSelectionPopup->show();
  }
}

void QmitkMultiWidgetConfigurationToolBar::OnSynchronize()
{
  bool synchronized = m_SynchronizeAction->isChecked();
  if (synchronized)
  {
    m_SynchronizeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwSynchronized.svg")));
    m_SynchronizeAction->setText(tr("Desynchronize render windows"));
  }
  else
  {
    m_SynchronizeAction->setIcon(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwDesynchronized.svg")));
    m_SynchronizeAction->setText(tr("Synchronize render windows"));
  }

  m_SynchronizeAction->setChecked(synchronized);
  emit Synchronized(synchronized);
}

void QmitkMultiWidgetConfigurationToolBar::OnInteractionSchemeChanged()
{
  const bool pacs = m_InteractionSchemeChangeAction->isChecked();

  this->UpdateInteractionSchemeAction(pacs);

  emit InteractionSchemeChanged(pacs
    ? mitk::InteractionSchemeSwitcher::PACSStandard
    : mitk::InteractionSchemeSwitcher::MITKStandard);
}

void QmitkMultiWidgetConfigurationToolBar::SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  this->UpdateInteractionSchemeAction(QmitkAbstractMultiWidget::IsPACSScheme(scheme));
}
