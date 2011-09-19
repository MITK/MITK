/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkStdMultiWidgetEditorPreferencePage.h"
#include "QmitkStdMultiWidgetEditor.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QColorDialog>
#include <QLineEdit>
#include <QComboBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage()
: m_MainControl(0)
{

}

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage(const QmitkStdMultiWidgetEditorPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkStdMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkStdMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_StdMultiWidgetEditorPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkStdMultiWidgetEditor::EDITOR_ID);

  m_MainControl = new QWidget(parent);
  m_EnableFlexibleZooming = new QCheckBox;
  m_ShowLevelWindowWidget = new QCheckBox;

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("&Use constrained zooming and padding:", m_EnableFlexibleZooming);
  formLayout->addRow("&Show intensity range control:", m_ShowLevelWindowWidget);

  QLabel* intensityRangeTypeLabel = new QLabel("Intensity range defined by:");
  m_IntensityRangeType = new QComboBox;
  m_IntensityRangeType->addItem("Level / Window");
  m_IntensityRangeType->addItem("Window Bounds");
  // 0: Level / Window, 1: Window Bounds
  m_IntensityRangeType->setCurrentIndex(0);
  QString intensityRangeTypeToolTip =
		  "Affects how you can control the displayed intensity\n"
		  "range of the current image. If set to \"Level / Window\",\n"
		  "the two fields in the bottom-right corner of the display\n"
		  "will set the level and window values, respectively.\n"
		  "Otherwise, the fields set the lower and the upper\n"
		  "window bound.";
  intensityRangeTypeLabel->setToolTip(intensityRangeTypeToolTip);
  m_IntensityRangeType->setToolTip(intensityRangeTypeToolTip);
  formLayout->addRow(intensityRangeTypeLabel, m_IntensityRangeType);

  QLabel* exponentialFormatLabel = new QLabel("Display values in exponential format:");
  m_ExponentialFormat = new QCheckBox;
  QString exponentialFormatToolTip =
		  "If checked, the intensity value at the crosshair will\n"
		  "be displayed in exponential format on the status bar.\n"
		  "The intensity range control will accept and display\n"
		  "values in exponential format as well.";
  exponentialFormatLabel->setToolTip(exponentialFormatToolTip);
  m_ExponentialFormat->setToolTip(exponentialFormatToolTip);
  formLayout->addRow(exponentialFormatLabel, m_ExponentialFormat);

  QLabel* precisionLabel = new QLabel("Precision:");
  m_Precision = new QLineEdit;
  m_Precision->setMaxLength(2);
  m_Precision->setValidator(new QIntValidator);
  QString precisionToolTip =
		  "Precision of floating point numbers shown on the status\n"
		  "bar and in the intensity range control.";
  precisionLabel->setToolTip(precisionToolTip);
  m_Precision->setToolTip(precisionToolTip);
  formLayout->addRow(precisionLabel, m_Precision);

  // gradient background
  QLabel* gBName = new QLabel("Gradient background:");

  // color
  m_ColorButton1 = new QPushButton;
  m_ColorButton1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  m_ColorButton2 = new QPushButton;
  m_ColorButton2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  QPushButton* resetButton = new QPushButton;
  resetButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  resetButton->setText("Reset");

  QLabel* colorLabel1 = new QLabel("first color:");
  colorLabel1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  QLabel* colorLabel2 = new QLabel("second color:");
  colorLabel2->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

  QHBoxLayout* colorWidgetLayout = new QHBoxLayout;
  colorWidgetLayout->setContentsMargins(4,4,4,4);
  colorWidgetLayout->addWidget(colorLabel1);
  colorWidgetLayout->addWidget(m_ColorButton1);
  colorWidgetLayout->addWidget(colorLabel2);
  colorWidgetLayout->addWidget(m_ColorButton2);
  colorWidgetLayout->addWidget(resetButton);

  QWidget* colorWidget = new QWidget;
  colorWidget->setLayout(colorWidgetLayout);

  //spacer
  QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  QVBoxLayout* vBoxLayout = new QVBoxLayout;
  vBoxLayout->addLayout(formLayout);
  vBoxLayout->addWidget(gBName);
  vBoxLayout->addWidget(colorWidget);
  vBoxLayout->addSpacerItem(spacer);

  m_MainControl->setLayout(vBoxLayout);

  QObject::connect( m_ColorButton1, SIGNAL( clicked() )
    , this, SLOT( FirstColorChanged() ) );

  QObject::connect( m_ColorButton2, SIGNAL( clicked() )
    , this, SLOT( SecondColorChanged() ) );

  QObject::connect( resetButton, SIGNAL( clicked() )
    , this, SLOT( ResetColors() ) );

  this->Update();
}

QWidget* QmitkStdMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkStdMultiWidgetEditorPreferencePage::PerformOk()
{
  m_StdMultiWidgetEditorPreferencesNode->Put("first background color style sheet", m_FirstColorStyleSheet.toStdString());
  m_StdMultiWidgetEditorPreferencesNode->Put("second background color style sheet", m_SecondColorStyleSheet.toStdString());
  m_StdMultiWidgetEditorPreferencesNode->PutByteArray("first background color", m_FirstColor);
  m_StdMultiWidgetEditorPreferencesNode->PutByteArray("second background color", m_SecondColor);
  m_StdMultiWidgetEditorPreferencesNode->PutBool("Use constrained zooming and padding",
      m_EnableFlexibleZooming->isChecked());
  m_StdMultiWidgetEditorPreferencesNode->PutBool("Show level/window widget",
      m_ShowLevelWindowWidget->isChecked());
  // 0: Level / Window, 1: Window Bounds
  m_StdMultiWidgetEditorPreferencesNode->PutBool("Window bounds intensity range",
      m_IntensityRangeType->currentIndex());
  m_StdMultiWidgetEditorPreferencesNode->PutBool("Exponential format",
      m_ExponentialFormat->isChecked());
  m_StdMultiWidgetEditorPreferencesNode->PutInt("Precision",
      m_Precision->text().toInt());

  return true;
}

void QmitkStdMultiWidgetEditorPreferencePage::PerformCancel()
{

}

void QmitkStdMultiWidgetEditorPreferencePage::Update()
{
  m_EnableFlexibleZooming->setChecked(m_StdMultiWidgetEditorPreferencesNode->GetBool("Use constrained zooming and padding", true));
  m_ShowLevelWindowWidget->setChecked(m_StdMultiWidgetEditorPreferencesNode->GetBool("Show level/window widget", true));
  // 0: Level / Window, 1: Window Bounds
  m_IntensityRangeType->setCurrentIndex(m_StdMultiWidgetEditorPreferencesNode->GetBool("Window bounds intensity range", false));
  m_ExponentialFormat->setChecked(m_StdMultiWidgetEditorPreferencesNode->GetBool("Exponential format", false));
  m_Precision->setText(QString::number(m_StdMultiWidgetEditorPreferencesNode->GetInt("Precision", 2)));
  m_FirstColorStyleSheet = QString::fromStdString(m_StdMultiWidgetEditorPreferencesNode->Get("first background color style sheet", ""));
  m_SecondColorStyleSheet = QString::fromStdString(m_StdMultiWidgetEditorPreferencesNode->Get("second background color style sheet", ""));
  m_FirstColor = m_StdMultiWidgetEditorPreferencesNode->GetByteArray("first background color", "");
  m_SecondColor = m_StdMultiWidgetEditorPreferencesNode->GetByteArray("second background color", "");
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
  m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);
}

void QmitkStdMultiWidgetEditorPreferencePage::FirstColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_ColorButton1->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_ColorButton1->setStyleSheet(styleSheet);

  m_FirstColorStyleSheet = styleSheet;
  QStringList firstColor;
  firstColor << color.name();
  m_FirstColor = firstColor.replaceInStrings(";","\\;").join(";").toStdString();
 }

void QmitkStdMultiWidgetEditorPreferencePage::SecondColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_ColorButton2->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_ColorButton2->setStyleSheet(styleSheet);

  m_SecondColorStyleSheet = styleSheet;
  QStringList secondColor;
  secondColor << color.name();
  m_SecondColor = secondColor.replaceInStrings(";","\\;").join(";").toStdString();
 }

void QmitkStdMultiWidgetEditorPreferencePage::ResetColors()
{
  m_FirstColorStyleSheet = "background-color:rgb(25,25,25)";
  m_SecondColorStyleSheet = "background-color:rgb(127,127,127)";
  m_FirstColor = "#191919";
  m_SecondColor = "#7F7F7F";
  m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);
}

void QmitkStdMultiWidgetEditorPreferencePage::UseGradientBackgroundSelected()
{

}

void QmitkStdMultiWidgetEditorPreferencePage::ColorActionChanged()
{

}

