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

  QObject::connect( m_Ui->m_RenderWindowChooser, SIGNAL(activated(int) )
                    , this, SLOT( OnWidgetComboBoxChanged(int) ) );
  QObject::connect( m_Ui->m_RenderWindowDecorationText, SIGNAL(textChanged(QString) )
                    , this, SLOT( AnnotationTextChanged(QString) ) );

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
  m_Preferences->PutByteArray("first background color", m_FirstColor);
  m_Preferences->PutByteArray("second background color", m_SecondColor);
  m_Preferences->PutByteArray("widget1 color", m_Widget1Color);
  m_Preferences->PutByteArray("widget2 color", m_Widget2Color);
  m_Preferences->PutByteArray("widget3 color", m_Widget3Color);
  m_Preferences->PutByteArray("widget4 color", m_Widget4Color);
  m_Preferences->PutByteArray("widget1 corner annotation", m_Widget1Annotation);
  m_Preferences->PutByteArray("widget2 corner annotation", m_Widget2Annotation);
  m_Preferences->PutByteArray("widget3 corner annotation", m_Widget3Annotation);
  m_Preferences->PutByteArray("widget4 corner annotation", m_Widget4Annotation);
  m_Preferences->PutInt("crosshair gap size", m_Ui->m_CrosshairGapSize->value());

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
  m_FirstColor = m_Preferences->GetByteArray("first background color", "#191919");
  m_SecondColor = m_Preferences->GetByteArray("second background color", "7F7F7F");
  m_Widget1Color = m_Preferences->GetByteArray("widget1 color", "#FF0000");
  m_Widget2Color = m_Preferences->GetByteArray("widget2 color", "#00FF00");
  m_Widget3Color = m_Preferences->GetByteArray("widget3 color", "#0000FF");
  m_Widget4Color = m_Preferences->GetByteArray("widget4 color", "#FFFF00");

  m_Ui->m_CrosshairGapSize->setValue(m_Preferences->GetInt("crosshair gap size", 32));

  m_Widget1Annotation = m_Preferences->GetByteArray("widget1 corner annotation", "Axial");
  m_Widget2Annotation = m_Preferences->GetByteArray("widget2 corner annotation", "Sagittal");
  m_Widget3Annotation = m_Preferences->GetByteArray("widget3 corner annotation", "Coronal");
  m_Widget4Annotation = m_Preferences->GetByteArray("widget4 corner annotation", "3D");

  QColor firstBackgroundColor = this->StringToColor(m_FirstColor);
  QColor secondBackgroundColor = this->StringToColor(m_SecondColor);
  QColor widgetColor = this->StringToColor(m_Widget1Color);

  this->SetStyleSheetToColorChooserButton(firstBackgroundColor, m_Ui->m_ColorButton1);
  this->SetStyleSheetToColorChooserButton(secondBackgroundColor, m_Ui->m_ColorButton2);
  this->SetStyleSheetToColorChooserButton(widgetColor, m_Ui->m_RenderWindowDecorationColor);

  m_Ui->m_RenderWindowDecorationText->setText(QString::fromStdString(m_Widget1Annotation));

  int index= m_Preferences->GetInt("Rendering Mode",0);
  m_Ui->m_RenderingMode->setCurrentIndex(index);
}

QColor QmitkStdMultiWidgetEditorPreferencePage::StringToColor(std::string colorInHex)
{
  QString colorQtString = QString::fromStdString(colorInHex);
  colorQtString = colorQtString.split("#").last();

  QString red = colorQtString.at(0);
  red.append(colorQtString.at(1));
  QString green = colorQtString.at(2);
  green.append(colorQtString.at(3));
  QString blue = colorQtString.at(4);
  blue.append(colorQtString.at(5));

  bool flag = true;
  unsigned int uired = red.toUInt(&flag, 16);
  unsigned int uigreen = green.toUInt(&flag, 16);
  unsigned int uiblue = blue.toUInt(&flag, 16);
  QColor color = QColor( uired, uigreen, uiblue);
  return color;
}

void QmitkStdMultiWidgetEditorPreferencePage::FirstColorChanged()
{
  QColor color = QColorDialog::getColor();
  this->SetStyleSheetToColorChooserButton(color, m_Ui->m_ColorButton1);

  QStringList firstColor;
  firstColor << color.name();
  m_FirstColor = firstColor.replaceInStrings(";","\\;").join(";").toStdString();
}

void QmitkStdMultiWidgetEditorPreferencePage::SetStyleSheetToColorChooserButton(QColor backgroundcolor,
                                                                                QPushButton* button)
{
  button->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(backgroundcolor.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(backgroundcolor.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(backgroundcolor.blue()));
  styleSheet.append(")");
  button->setStyleSheet(styleSheet);
}

void QmitkStdMultiWidgetEditorPreferencePage::SecondColorChanged()
{
  QColor color = QColorDialog::getColor();
  this->SetStyleSheetToColorChooserButton(color, m_Ui->m_ColorButton2);

  QStringList secondColor;
  secondColor << color.name();
  m_SecondColor = secondColor.replaceInStrings(";","\\;").join(";").toStdString();
}

void QmitkStdMultiWidgetEditorPreferencePage::WidgetColorChanged()
{
  QColor color = QColorDialog::getColor();
  this->SetStyleSheetToColorChooserButton(color, m_Ui->m_RenderWindowDecorationColor);
  m_Ui->m_RenderWindowDecorationColor->setAutoFillBackground(true);

  QStringList qtColor;
  qtColor << color.name();

  switch (m_Ui->m_RenderWindowChooser->currentIndex()) {
  case 0: //widget 1
    m_Widget1Color = qtColor.replaceInStrings(";","\\;").join(";").toStdString();
    break;
  case 1: //widget 1
    m_Widget2Color = qtColor.replaceInStrings(";","\\;").join(";").toStdString();
    break;
  case 2: //widget 1
    m_Widget3Color = qtColor.replaceInStrings(";","\\;").join(";").toStdString();
    break;
  case 3: //widget 1
    m_Widget4Color = qtColor.replaceInStrings(";","\\;").join(";").toStdString();
    break;
  default:
    MITK_INFO << "error selected wrong index.";
    break;
  }
}

void QmitkStdMultiWidgetEditorPreferencePage::AnnotationTextChanged(QString text)
{
  switch (m_Ui->m_RenderWindowChooser->currentIndex()) {
  case 0: //widget 1
    m_Widget1Annotation = text.toStdString();
    break;
  case 1: //widget 1
    m_Widget2Annotation = text.toStdString();
    break;
  case 2: //widget 1
    m_Widget3Annotation = text.toStdString();
    break;
  case 3: //widget 1
    m_Widget4Annotation = text.toStdString();
    break;
  default:
    MITK_INFO << "error selected wrong index.";
    break;
  }
}

void QmitkStdMultiWidgetEditorPreferencePage::ResetColors()
{
  //default gradient background values
  m_FirstColor = "#191919";
  m_SecondColor = "#7F7F7F";
  QColor firstColor = this->StringToColor(m_FirstColor);
  this->SetStyleSheetToColorChooserButton(firstColor, m_Ui->m_ColorButton1);
  QColor secondColor = this->StringToColor(m_SecondColor);
  this->SetStyleSheetToColorChooserButton(secondColor, m_Ui->m_ColorButton2);
}

void QmitkStdMultiWidgetEditorPreferencePage::OnWidgetComboBoxChanged(int i)
{
  QColor widgetColor;
  switch (i) {
  case 0: //widget 1
    widgetColor = StringToColor(m_Widget1Color);
    m_Ui->m_RenderWindowDecorationText->setText(QString::fromStdString(m_Widget1Annotation));
    break;
  case 1: //widget 1
    widgetColor = StringToColor(m_Widget2Color);
    m_Ui->m_RenderWindowDecorationText->setText(QString::fromStdString(m_Widget2Annotation));
    break;
  case 2: //widget 1
    widgetColor = StringToColor(m_Widget3Color);
    m_Ui->m_RenderWindowDecorationText->setText(QString::fromStdString(m_Widget3Annotation));
    break;
  case 3: //widget 1
    widgetColor = StringToColor(m_Widget4Color);
    m_Ui->m_RenderWindowDecorationText->setText(QString::fromStdString(m_Widget4Annotation));
    break;
  default:
    MITK_INFO << "error selected wrong index.";
    break;
  }
  this->SetStyleSheetToColorChooserButton(widgetColor, m_Ui->m_RenderWindowDecorationColor);
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
