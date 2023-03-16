/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkStdMultiWidgetEditorPreferencePage.h"
#include <ui_QmitkStdMultiWidgetEditorPreferencePage.h>
#include <QmitkStdMultiWidgetEditor.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QColorDialog>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(QmitkStdMultiWidgetEditor::EDITOR_ID.toStdString());
  }
}

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage()
  : m_Ui(new Ui::QmitkStdMultiWidgetEditorPreferencePage),
    m_Control(nullptr)
{
}

QmitkStdMultiWidgetEditorPreferencePage::~QmitkStdMultiWidgetEditorPreferencePage()
{
}

void QmitkStdMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);

  m_Ui->setupUi(m_Control);

  QObject::connect( m_Ui->m_ColorButton1, SIGNAL( clicked() )
                    , this, SLOT( ColorChooserButtonClicked() ) );

  QObject::connect( m_Ui->m_ColorButton2, SIGNAL( clicked() )
                    , this, SLOT( ColorChooserButtonClicked() ) );

  QObject::connect( m_Ui->m_ResetButton, SIGNAL( clicked() )
                    , this, SLOT( ResetPreferencesAndGUI() ) );

  QObject::connect( m_Ui->m_RenderWindowDecorationColor, SIGNAL( clicked() )
                    , this, SLOT( ColorChooserButtonClicked() ) );

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
  auto* prefs = GetPreferences();

  prefs->Put("stdmulti.widget0 corner annotation", m_WidgetAnnotation[0].toStdString());
  prefs->Put("stdmulti.widget1 corner annotation", m_WidgetAnnotation[1].toStdString());
  prefs->Put("stdmulti.widget2 corner annotation", m_WidgetAnnotation[2].toStdString());
  prefs->Put("stdmulti.widget3 corner annotation", m_WidgetAnnotation[3].toStdString());

  prefs->Put("stdmulti.widget0 decoration color", m_WidgetDecorationColor[0].toStdString());
  prefs->Put("stdmulti.widget1 decoration color", m_WidgetDecorationColor[1].toStdString());
  prefs->Put("stdmulti.widget2 decoration color", m_WidgetDecorationColor[2].toStdString());
  prefs->Put("stdmulti.widget3 decoration color", m_WidgetDecorationColor[3].toStdString());

  prefs->Put("stdmulti.widget0 first background color", m_WidgetBackgroundColor1[0].toStdString());
  prefs->Put("stdmulti.widget1 first background color", m_WidgetBackgroundColor1[1].toStdString());
  prefs->Put("stdmulti.widget2 first background color", m_WidgetBackgroundColor1[2].toStdString());
  prefs->Put("stdmulti.widget3 first background color", m_WidgetBackgroundColor1[3].toStdString());
  prefs->Put("stdmulti.widget0 second background color", m_WidgetBackgroundColor2[0].toStdString());
  prefs->Put("stdmulti.widget1 second background color", m_WidgetBackgroundColor2[1].toStdString());
  prefs->Put("stdmulti.widget2 second background color", m_WidgetBackgroundColor2[2].toStdString());
  prefs->Put("stdmulti.widget3 second background color", m_WidgetBackgroundColor2[3].toStdString());
  prefs->PutInt("crosshair gap size", m_Ui->m_CrosshairGapSize->value());

  prefs->PutBool("Use constrained zooming and panning"
                         , m_Ui->m_EnableFlexibleZooming->isChecked());
  prefs->PutBool("Show level/window widget", m_Ui->m_ShowLevelWindowWidget->isChecked());
  prefs->PutBool("PACS like mouse interaction", m_Ui->m_PACSLikeMouseMode->isChecked());

  return true;
}

void QmitkStdMultiWidgetEditorPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  //Note: there should be default preferences already defined in the
  //QmitkStdMultiWidgetEditor::InitializePreferences(). Therefore,
  //all default values here are not relevant.
  //gradient background colors
  m_WidgetBackgroundColor1[0] = QString::fromStdString(prefs->Get("stdmulti.widget0 first background color", "#000000"));
  m_WidgetBackgroundColor2[0] = QString::fromStdString(prefs->Get("stdmulti.widget0 second background color", "#000000"));
  m_WidgetBackgroundColor1[1] = QString::fromStdString(prefs->Get("stdmulti.widget1 first background color", "#000000"));
  m_WidgetBackgroundColor2[1] = QString::fromStdString(prefs->Get("stdmulti.widget1 second background color", "#000000"));
  m_WidgetBackgroundColor1[2] = QString::fromStdString(prefs->Get("stdmulti.widget2 first background color", "#000000"));
  m_WidgetBackgroundColor2[2] = QString::fromStdString(prefs->Get("stdmulti.widget2 second background color", "#000000"));
  m_WidgetBackgroundColor1[3] = QString::fromStdString(prefs->Get("stdmulti.widget3 first background color", "#191919"));
  m_WidgetBackgroundColor2[3] = QString::fromStdString(prefs->Get("stdmulti.widget3 second background color", "#7F7F7F"));

  //decoration colors
  m_WidgetDecorationColor[0] = QString::fromStdString(prefs->Get("stdmulti.widget0 decoration color", "#c00000"));
  m_WidgetDecorationColor[1] = QString::fromStdString(prefs->Get("stdmulti.widget1 decoration color", "#00b000"));
  m_WidgetDecorationColor[2] = QString::fromStdString(prefs->Get("stdmulti.widget2 decoration color", "#0080ff"));
  m_WidgetDecorationColor[3] = QString::fromStdString(prefs->Get("stdmulti.widget3 decoration color", "#ffff00"));

  //annotation text
  m_WidgetAnnotation[0] = QString::fromStdString(prefs->Get("stdmulti.widget0 corner annotation", "Axial"));
  m_WidgetAnnotation[1] = QString::fromStdString(prefs->Get("stdmulti.widget1 corner annotation", "Sagittal"));
  m_WidgetAnnotation[2] = QString::fromStdString(prefs->Get("stdmulti.widget2 corner annotation", "Coronal"));
  m_WidgetAnnotation[3] = QString::fromStdString(prefs->Get("stdmulti.widget3 corner annotation", "3D"));


  //Ui stuff
  int index = m_Ui->m_RenderWindowChooser->currentIndex();
  QColor firstBackgroundColor(m_WidgetBackgroundColor1[index]);
  QColor secondBackgroundColor(m_WidgetBackgroundColor2[index]);
  QColor widgetColor(m_WidgetDecorationColor[index]);

  this->SetStyleSheetToColorChooserButton(firstBackgroundColor, m_Ui->m_ColorButton1);
  this->SetStyleSheetToColorChooserButton(secondBackgroundColor, m_Ui->m_ColorButton2);
  this->SetStyleSheetToColorChooserButton(widgetColor, m_Ui->m_RenderWindowDecorationColor);

  m_Ui->m_RenderWindowDecorationText->setText(m_WidgetAnnotation[index]);

  m_Ui->m_EnableFlexibleZooming->setChecked(prefs->GetBool("Use constrained zooming and panning", true));
  m_Ui->m_ShowLevelWindowWidget->setChecked(prefs->GetBool("Show level/window widget", true));
  m_Ui->m_PACSLikeMouseMode->setChecked(prefs->GetBool("PACS like mouse interaction", false));
  m_Ui->m_CrosshairGapSize->setValue(prefs->GetInt("crosshair gap size", 32));
}

void QmitkStdMultiWidgetEditorPreferencePage::ColorChooserButtonClicked()
{
  unsigned int widgetIndex = m_Ui->m_RenderWindowChooser->currentIndex();
  if(widgetIndex > 3)
  {
    MITK_ERROR << "Selected index for unknown.";
    return;
  }
  QObject *senderObj = sender(); // This will give Sender button
  //find out last used color and set it
  QColor initialColor;
  if( senderObj->objectName() == m_Ui->m_ColorButton1->objectName())
  {
    initialColor = QColor(m_WidgetBackgroundColor1[widgetIndex]);
  }else if( senderObj->objectName() == m_Ui->m_ColorButton2->objectName())
  {
    initialColor = QColor(m_WidgetBackgroundColor2[widgetIndex]);
  }else if( senderObj->objectName() == m_Ui->m_RenderWindowDecorationColor->objectName())
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
  if( senderObj->objectName() == m_Ui->m_ColorButton1->objectName())
  {
    m_WidgetBackgroundColor1[widgetIndex] = newcolor.name();
  }
  else if( senderObj->objectName() == m_Ui->m_ColorButton2->objectName())
  {
    m_WidgetBackgroundColor2[widgetIndex] = newcolor.name();
  }
  else if( senderObj->objectName() == m_Ui->m_RenderWindowDecorationColor->objectName())
  {
    m_WidgetDecorationColor[widgetIndex] = newcolor.name();
  }
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

void QmitkStdMultiWidgetEditorPreferencePage::AnnotationTextChanged(QString text)
{
  unsigned int widgetIndex = m_Ui->m_RenderWindowChooser->currentIndex();
  if( widgetIndex > 3)
  {
    MITK_INFO << "Selected index for unknown widget.";
    return;
  }
  m_WidgetAnnotation[widgetIndex] = text;
}

void QmitkStdMultiWidgetEditorPreferencePage::ResetPreferencesAndGUI()
{
  auto* prefs = GetPreferences();

  prefs->Clear();
  this->Update();
}

void QmitkStdMultiWidgetEditorPreferencePage::OnWidgetComboBoxChanged(int i)
{
  if( i > 3)
  {
    MITK_ERROR << "Selected unknown widget.";
    return;
  }
  QColor widgetColor(m_WidgetDecorationColor[i]);
  QColor gradientBackground1(m_WidgetBackgroundColor1[i]);
  QColor gradientBackground2(m_WidgetBackgroundColor2[i]);
  this->SetStyleSheetToColorChooserButton(widgetColor, m_Ui->m_RenderWindowDecorationColor);
  this->SetStyleSheetToColorChooserButton(gradientBackground1, m_Ui->m_ColorButton1);
  this->SetStyleSheetToColorChooserButton(gradientBackground2, m_Ui->m_ColorButton2);
  m_Ui->m_RenderWindowDecorationText->setText(m_WidgetAnnotation[i]);
}
