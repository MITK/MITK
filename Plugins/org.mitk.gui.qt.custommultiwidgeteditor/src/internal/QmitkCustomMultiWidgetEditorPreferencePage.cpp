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

#include <ui_QmitkCustomMultiWidgetEditorPreferencePage.h>
#include "QmitkCustomMultiWidgetEditorPreferencePage.h"
#include <QmitkCustomMultiWidgetEditor.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <QColorDialog>

QmitkCustomMultiWidgetEditorPreferencePage::QmitkCustomMultiWidgetEditorPreferencePage()
  : m_Preferences(nullptr)
{
  // nothing here
}

QmitkCustomMultiWidgetEditorPreferencePage::~QmitkCustomMultiWidgetEditorPreferencePage()
{
  //nothing here
}

void QmitkCustomMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);

  m_Ui.setupUi(m_MainControl);

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  Q_ASSERT(prefService);

  m_Preferences = prefService->GetSystemPreferences()->Node(QmitkCustomMultiWidgetEditor::EDITOR_ID);

  connect(m_Ui.m_ColorButton1, SIGNAL(clicked()), SLOT(ColorChooserButtonClicked()));
  connect(m_Ui.m_ColorButton2, SIGNAL(clicked()), SLOT(ColorChooserButtonClicked()));
  connect(m_Ui.m_ResetButton, SIGNAL(clicked()), SLOT(ResetPreferencesAndGUI()));
  connect(m_Ui.m_RenderingMode, SIGNAL(activated(int)), SLOT(ChangeRenderingMode(int)));
  connect(m_Ui.m_RenderWindowDecorationColor, SIGNAL(clicked()), SLOT(ColorChooserButtonClicked()));
  connect(m_Ui.m_RenderWindowChooser, SIGNAL(activated(int)), SLOT(OnWidgetComboBoxChanged(int)));
  connect(m_Ui.m_RenderWindowDecorationText, SIGNAL(textChanged(QString)), SLOT(AnnotationTextChanged(QString)));

  Update();
}

QWidget* QmitkCustomMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

void QmitkCustomMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkCustomMultiWidgetEditorPreferencePage::PerformCancel()
{
}

bool QmitkCustomMultiWidgetEditorPreferencePage::PerformOk()
{
  m_Preferences->Put("widget1 corner annotation", m_WidgetAnnotation[0]);
  m_Preferences->Put("widget2 corner annotation", m_WidgetAnnotation[1]);
  m_Preferences->Put("widget3 corner annotation", m_WidgetAnnotation[2]);
  m_Preferences->Put("widget4 corner annotation", m_WidgetAnnotation[3]);

  m_Preferences->Put("widget1 decoration color", m_WidgetDecorationColor[0]);
  m_Preferences->Put("widget2 decoration color", m_WidgetDecorationColor[1]);
  m_Preferences->Put("widget3 decoration color", m_WidgetDecorationColor[2]);
  m_Preferences->Put("widget4 decoration color", m_WidgetDecorationColor[3]);

  m_Preferences->Put("widget1 first background color", m_WidgetBackgroundColor1[0]);
  m_Preferences->Put("widget2 first background color", m_WidgetBackgroundColor1[1]);
  m_Preferences->Put("widget3 first background color", m_WidgetBackgroundColor1[2]);
  m_Preferences->Put("widget4 first background color", m_WidgetBackgroundColor1[3]);
  m_Preferences->Put("widget1 second background color", m_WidgetBackgroundColor2[0]);
  m_Preferences->Put("widget2 second background color", m_WidgetBackgroundColor2[1]);
  m_Preferences->Put("widget3 second background color", m_WidgetBackgroundColor2[2]);
  m_Preferences->Put("widget4 second background color", m_WidgetBackgroundColor2[3]);
  m_Preferences->PutInt("crosshair gap size", m_Ui.m_CrosshairGapSize->value());

  m_Preferences->PutBool("Use constrained zooming and panning", m_Ui.m_EnableFlexibleZooming->isChecked());
  m_Preferences->PutBool("Show level/window widget", m_Ui.m_ShowLevelWindowWidget->isChecked());
  m_Preferences->PutBool("PACS like mouse interaction", m_Ui.m_PACSLikeMouseMode->isChecked());
  m_Preferences->PutInt("Rendering Mode", m_Ui.m_RenderingMode->currentIndex());

  return true;
}

void QmitkCustomMultiWidgetEditorPreferencePage::Update()
{
  //Note: there should be default preferences already defined in the
  //QmitkStdMultiWidgetEditor::InitializePreferences(). Therefore,
  //all default values here are not relevant.
  //gradient background colors
  m_WidgetBackgroundColor1[0] = m_Preferences->Get("widget1 first background color", "#000000");
  m_WidgetBackgroundColor2[0] = m_Preferences->Get("widget1 second background color", "#000000");
  m_WidgetBackgroundColor1[1] = m_Preferences->Get("widget2 first background color", "#000000");
  m_WidgetBackgroundColor2[1] = m_Preferences->Get("widget2 second background color", "#000000");
  m_WidgetBackgroundColor1[2] = m_Preferences->Get("widget3 first background color", "#000000");
  m_WidgetBackgroundColor2[2] = m_Preferences->Get("widget3 second background color", "#000000");
  m_WidgetBackgroundColor1[3] = m_Preferences->Get("widget4 first background color", "#191919");
  m_WidgetBackgroundColor2[3] = m_Preferences->Get("widget4 second background color", "#7F7F7F");

  //decoration colors
  m_WidgetDecorationColor[0] = m_Preferences->Get("widget1 decoration color", "#FF0000");
  m_WidgetDecorationColor[1] = m_Preferences->Get("widget2 decoration color", "#00FF00");
  m_WidgetDecorationColor[2] = m_Preferences->Get("widget3 decoration color", "#0000FF");
  m_WidgetDecorationColor[3] = m_Preferences->Get("widget4 decoration color", "#FFFF00");

  //annotation text
  m_WidgetAnnotation[0] = m_Preferences->Get("widget1 corner annotation", "Axial");
  m_WidgetAnnotation[1] = m_Preferences->Get("widget2 corner annotation", "Sagittal");
  m_WidgetAnnotation[2] = m_Preferences->Get("widget3 corner annotation", "Coronal");
  m_WidgetAnnotation[3] = m_Preferences->Get("widget4 corner annotation", "3D");


  //Ui stuff
  int index = m_Ui.m_RenderWindowChooser->currentIndex();
  QColor firstBackgroundColor(m_WidgetBackgroundColor1[index]);
  QColor secondBackgroundColor(m_WidgetBackgroundColor2[index]);
  QColor widgetColor(m_WidgetDecorationColor[index]);

  this->SetStyleSheetToColorChooserButton(firstBackgroundColor, m_Ui.m_ColorButton1);
  this->SetStyleSheetToColorChooserButton(secondBackgroundColor, m_Ui.m_ColorButton2);
  this->SetStyleSheetToColorChooserButton(widgetColor, m_Ui.m_RenderWindowDecorationColor);

  m_Ui.m_RenderWindowDecorationText->setText(m_WidgetAnnotation[index]);

  m_Ui.m_EnableFlexibleZooming->setChecked(m_Preferences->GetBool("Use constrained zooming and panning", true));
  m_Ui.m_ShowLevelWindowWidget->setChecked(m_Preferences->GetBool("Show level/window widget", true));
  m_Ui.m_PACSLikeMouseMode->setChecked(m_Preferences->GetBool("PACS like mouse interaction", false));
  int mode= m_Preferences->GetInt("Rendering Mode",0);
  m_Ui.m_RenderingMode->setCurrentIndex(mode);
  m_Ui.m_CrosshairGapSize->setValue(m_Preferences->GetInt("crosshair gap size", 32));
}

void QmitkCustomMultiWidgetEditorPreferencePage::ColorChooserButtonClicked()
{
  unsigned int widgetIndex = m_Ui.m_RenderWindowChooser->currentIndex();
  if(widgetIndex > 3)
  {
    MITK_ERROR << "Selected index for unknown.";
    return;
  }
  QObject *senderObj = sender(); // This will give Sender button
  //find out last used color and set it
  QColor initialColor;
  if( senderObj->objectName() == m_Ui.m_ColorButton1->objectName())
  {
    initialColor = QColor(m_WidgetBackgroundColor1[widgetIndex]);
  }else if( senderObj->objectName() == m_Ui.m_ColorButton2->objectName())
  {
    initialColor = QColor(m_WidgetBackgroundColor2[widgetIndex]);
  }else if( senderObj->objectName() == m_Ui.m_RenderWindowDecorationColor->objectName())
  {
    initialColor = QColor(m_WidgetDecorationColor[widgetIndex]);
  }

  //get the new color
  QColor newcolor = QColorDialog::getColor(initialColor);
  if(!newcolor.isValid())
  {
    newcolor = initialColor;
  }
  this->SetStyleSheetToColorChooserButton(newcolor, static_cast<QPushButton*>(senderObj));

  //convert it to std string and apply it
  if( senderObj->objectName() == m_Ui.m_ColorButton1->objectName())
  {
    m_WidgetBackgroundColor1[widgetIndex] = newcolor.name();
  }
  else if( senderObj->objectName() == m_Ui.m_ColorButton2->objectName())
  {
    m_WidgetBackgroundColor2[widgetIndex] = newcolor.name();
  }
  else if( senderObj->objectName() == m_Ui.m_RenderWindowDecorationColor->objectName())
  {
    m_WidgetDecorationColor[widgetIndex] = newcolor.name();
  }
}

void QmitkCustomMultiWidgetEditorPreferencePage::SetStyleSheetToColorChooserButton(QColor backgroundcolor, QPushButton* button)
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

void QmitkCustomMultiWidgetEditorPreferencePage::AnnotationTextChanged(QString text)
{
  unsigned int widgetIndex = m_Ui.m_RenderWindowChooser->currentIndex();
  if( widgetIndex > 3)
  {
    MITK_INFO << "Selected index for unknown widget.";
    return;
  }
  m_WidgetAnnotation[widgetIndex] = text;
}

void QmitkCustomMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  m_Preferences->Clear();
  this->Update();
}

void QmitkCustomMultiWidgetEditorPreferencePage::OnWidgetComboBoxChanged(int i)
{
  if( i > 3)
  {
    MITK_ERROR << "Selected unknown widget.";
    return;
  }
  QColor widgetColor(m_WidgetDecorationColor[i]);
  QColor gradientBackground1(m_WidgetBackgroundColor1[i]);
  QColor gradientBackground2(m_WidgetBackgroundColor2[i]);
  SetStyleSheetToColorChooserButton(widgetColor, m_Ui.m_RenderWindowDecorationColor);
  SetStyleSheetToColorChooserButton(gradientBackground1, m_Ui.m_ColorButton1);
  SetStyleSheetToColorChooserButton(gradientBackground2, m_Ui.m_ColorButton2);
  m_Ui.m_RenderWindowDecorationText->setText(m_WidgetAnnotation[i]);
}

void QmitkCustomMultiWidgetEditorPreferencePage::ChangeRenderingMode(int i)
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
