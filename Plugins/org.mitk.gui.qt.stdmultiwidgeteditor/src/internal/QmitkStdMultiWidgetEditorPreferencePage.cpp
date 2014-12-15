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

#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryServiceRegistry.h>
#include <mitkExceptionMacro.h>
#include <QColorDialog>
#include <ui_QmitkStdMultiWidgetEditorPreferencePage.h>
#include "QmitkStdMultiWidgetEditorPreferencePage.h"

#include <QmitkStdMultiWidgetEditor.h>

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage()
  : m_Preferences(NULL),
    m_Ui(new Ui::QmitkStdMultiWidgetEditorPreferencePage),
    m_Control(NULL)
{
}

QmitkStdMultiWidgetEditorPreferencePage::~QmitkStdMultiWidgetEditorPreferencePage()
{
}

void QmitkStdMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);

  m_Ui->setupUi(m_Control);

  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_Preferences = prefService->GetSystemPreferences()->Node(QmitkStdMultiWidgetEditor::EDITOR_ID);

  QObject::connect( m_Ui->m_ColorButton1, SIGNAL( clicked() )
    , this, SLOT( FirstColorChanged() ) );

  QObject::connect( m_Ui->m_ColorButton2, SIGNAL( clicked() )
    , this, SLOT( SecondColorChanged() ) );

  QObject::connect( m_Ui->m_ResetButton, SIGNAL( clicked() )
    , this, SLOT( ResetColors() ) );

  QObject::connect( m_Ui->m_RenderingMode, SIGNAL(activated(int) )
    , this, SLOT( ChangeRenderingMode(int) ) );

  QObject::connect( m_Ui->m_RenderWindowDecorationColor, SIGNAL( clicked() )
    , this, SLOT( WidgetColorChanged() ) );

  this->Update();
}

QWidget* QmitkStdMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkStdMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkStdMultiWidgetEditorPreferencePage::PerformCancel()
{
}

bool QmitkStdMultiWidgetEditorPreferencePage::PerformOk()
{
  m_Preferences->Put("first background color style sheet", m_FirstColorStyleSheet.toStdString());
  m_Preferences->Put("second background color style sheet", m_SecondColorStyleSheet.toStdString());
  m_Preferences->Put("widget1 color style sheet", m_Widget1ColorStyleSheet.toStdString());
  m_Preferences->PutByteArray("first background color", m_FirstColor);
  m_Preferences->PutByteArray("second background color", m_SecondColor);
  m_Preferences->PutByteArray("widget1 color", m_Widget1Color);
  m_Preferences->PutBool("Use constrained zooming and padding"
                                        , m_Ui->m_EnableFlexibleZooming->isChecked());
  m_Preferences->PutBool("Show level/window widget", m_Ui->m_ShowLevelWindowWidget->isChecked());
  m_Preferences->PutBool("PACS like mouse interaction", m_Ui->m_PACSLikeMouseMode->isChecked());
  m_Preferences->PutInt("Rendering Mode", m_Ui->m_RenderingMode->currentIndex());

  return true;
}

void QmitkStdMultiWidgetEditorPreferencePage::Update()
{
  m_Ui->m_EnableFlexibleZooming->setChecked(m_Preferences->GetBool("Use constrained zooming and padding", true));
  m_Ui->m_ShowLevelWindowWidget->setChecked(m_Preferences->GetBool("Show level/window widget", true));
  m_Ui->m_PACSLikeMouseMode->setChecked(m_Preferences->GetBool("PACS like mouse interaction", false));
  m_FirstColorStyleSheet = QString::fromStdString(m_Preferences->Get("first background color style sheet", ""));
  m_SecondColorStyleSheet = QString::fromStdString(m_Preferences->Get("second background color style sheet", ""));
  m_Widget1ColorStyleSheet = QString::fromStdString(m_Preferences->Get("widget1 color style sheet", ""));
  m_FirstColor = m_Preferences->GetByteArray("first background color", "");
  m_SecondColor = m_Preferences->GetByteArray("second background color", "");
  m_Widget1Color = m_Preferences->GetByteArray("widget1 color", "");
  int index= m_Preferences->GetInt("Rendering Mode",0);
  m_Ui->m_RenderingMode->setCurrentIndex(index);

  if (m_FirstColorStyleSheet=="")
  {
    m_FirstColorStyleSheet = "background-color:rgb(25,25,25)";
  }
  if (m_SecondColorStyleSheet=="")
  {
    m_SecondColorStyleSheet = "background-color:rgb(127,127,127)";
  }
  if (m_FirstColor=="")
  {
    m_FirstColor = "#191919";
  }
  if (m_SecondColor=="")
  {
    m_SecondColor = "#7F7F7F";
  }
  if (m_Widget1ColorStyleSheet=="")
  {
    m_Widget1ColorStyleSheet = "background-color:rgb(255,0,0)";
  }
  if (m_Widget1Color=="")
  {
    m_Widget1ColorStyleSheet = "FF0000";
  }

  m_Ui->m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_Ui->m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);
  m_Ui->m_RenderWindowDecorationColor->setStyleSheet(m_Widget1ColorStyleSheet);
}

void QmitkStdMultiWidgetEditorPreferencePage::FirstColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_Ui->m_ColorButton1->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_Ui->m_ColorButton1->setStyleSheet(styleSheet);

  m_FirstColorStyleSheet = styleSheet;
  QStringList firstColor;
  firstColor << color.name();
  m_FirstColor = firstColor.replaceInStrings(";","\\;").join(";").toStdString();
 }

void QmitkStdMultiWidgetEditorPreferencePage::SecondColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_Ui->m_ColorButton2->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_Ui->m_ColorButton2->setStyleSheet(styleSheet);

  m_SecondColorStyleSheet = styleSheet;
  QStringList secondColor;
  secondColor << color.name();
  m_SecondColor = secondColor.replaceInStrings(";","\\;").join(";").toStdString();
}

void QmitkStdMultiWidgetEditorPreferencePage::WidgetColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_Ui->m_RenderWindowDecorationColor->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_Ui->m_RenderWindowDecorationColor->setStyleSheet(styleSheet);

  m_Widget1ColorStyleSheet = styleSheet;
  QStringList secondColor;
  secondColor << color.name();
  m_Widget1Color = secondColor.replaceInStrings(";","\\;").join(";").toStdString();
}

void QmitkStdMultiWidgetEditorPreferencePage::ResetColors()
{
  m_FirstColorStyleSheet = "background-color:rgb(25,25,25)";
  m_SecondColorStyleSheet = "background-color:rgb(127,127,127)";
  m_FirstColor = "#191919";
  m_SecondColor = "#7F7F7F";
  m_Ui->m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_Ui->m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);
}

void QmitkStdMultiWidgetEditorPreferencePage::ChangeRenderingMode(int i)
{
  if( i == 0 )
  {
    m_CurrentRenderingMode = "Standard";
  }
  else if( i == 1 )
  {
   m_CurrentRenderingMode = "Multisampling";
  }
  else if( i == 2 )
  {
    m_CurrentRenderingMode = "DepthPeeling";
  }
}
