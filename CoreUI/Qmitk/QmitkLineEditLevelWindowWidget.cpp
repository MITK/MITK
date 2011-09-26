/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2011-01-18 13:22:38 +0100 (Di, 18 Jan 2011) $
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkLineEditLevelWindowWidget.h"
#include "QmitkLevelWindowWidgetContextMenu.h"

#include <mitkRenderingManager.h>

#include <itkCommand.h>
#include <QValidator>
#include <QLayout>
#include <QLineEdit>
#include <limits>

/**
* Constructor
*/
QmitkLineEditLevelWindowWidget::QmitkLineEditLevelWindowWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f),
m_Format('f'),
m_Precision(2),
m_IntensityRangeType(LevelWindow)
{
  m_Manager = mitk::LevelWindowManager::New();
  
  itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  m_IsObserverTagSet = true;

  m_Contextmenu = new QmitkLevelWindowWidgetContextMenu(this); // true);
  
  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_LevelInput = new QLineEdit( this );
  m_LevelInput->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred));
  //m_LevelInput->setFrameShape( QLineEdit::LineEditPanel );
  //m_LevelInput->setFrameShadow( QLineEdit::Sunken );

  m_WindowInput = new QLineEdit( this );
  m_WindowInput->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred));
  //m_WindowInput->setFrameShape( QLineEdit::LineEditPanel );
  //m_WindowInput->setFrameShadow( QLineEdit::Sunken );

  layout->addWidget(m_LevelInput);
  layout->addWidget(m_WindowInput);

  // signals and slots connections
  connect( m_LevelInput, SIGNAL(editingFinished()), this, SLOT( SetLevelValue() ) );
  connect( m_WindowInput, SIGNAL(editingFinished()), this, SLOT( SetWindowValue() ) );

  m_Validator1 = new QDoubleValidator(0, std::numeric_limits<double>::max(), m_Precision, this);
  m_WindowInput->setValidator(m_Validator1);

  m_Validator2 = new QDoubleValidator(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), m_Precision, this);
  m_LevelInput->setValidator(m_Validator2);
  
  this->hide();
}

QmitkLineEditLevelWindowWidget::~QmitkLineEditLevelWindowWidget()
{
  if( m_IsObserverTagSet )
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
}

void QmitkLineEditLevelWindowWidget::OnPropertyModified(const itk::EventObject& )
{
  try
  {
    updateInputs();
    m_LevelInput->setEnabled(!m_LevelWindow.IsFixed());
    m_WindowInput->setEnabled(!m_LevelWindow.IsFixed());  
    this->show();
  }
  catch(...)
  {
    try
    {
      this->hide();
    }
    catch(...)
    {
    }
  }
}

void QmitkLineEditLevelWindowWidget::updateInputs()
{
  m_LevelWindow = m_Manager->GetLevelWindow();
  double number1, number2;
  QString toolTip1, toolTip2;
  if (m_IntensityRangeType == LevelWindow)
  {
    number1 = m_LevelWindow.GetLevel();
    number2 = m_LevelWindow.GetWindow();
    toolTip1 = QString("Level: %1").arg(number1);
    toolTip2 = QString("Window: %1").arg(number2);
  }
  else
  {
    number1 = m_LevelWindow.GetLowerWindowBound();
    number2 = m_LevelWindow.GetUpperWindowBound();
    toolTip1 = QString("Lower bound: %1").arg(number1);
    toolTip2 = QString("Upper bound: %1").arg(number2);
  }

  //setValidator();
  QString qNumber1;
  qNumber1.setNum(number1, m_Format, m_Precision);
  m_LevelInput->setText(qNumber1);
  m_LevelInput->setCursorPosition(0);
  m_LevelInput->setToolTip(toolTip1);
  QString qNumber2;
  qNumber2.setNum(number2, m_Format, m_Precision);
  m_WindowInput->setText(qNumber2);
  m_WindowInput->setCursorPosition(0);
  m_WindowInput->setToolTip(toolTip2);
}

void QmitkLineEditLevelWindowWidget::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
  if( m_IsObserverTagSet )
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  m_Manager = levelWindowManager;
  if ( m_Manager.IsNotNull() )
  {
    itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkLineEditLevelWindowWidget>::New();
    command->SetCallbackFunction(this, &QmitkLineEditLevelWindowWidget::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;
  }
}

void QmitkLineEditLevelWindowWidget::SetDataStorage( mitk::DataStorage* ds )
{
  m_Manager->SetDataStorage(ds);
}

//read the levelInput and change level and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLineEditLevelWindowWidget::SetLevelValue()
{
  if (m_IntensityRangeType == LevelWindow)
  {
    double oldLevel = m_LevelWindow.GetLevel();
    double level = m_LevelInput->text().toDouble();
    correctLevel(level);
    if (level != oldLevel)
    {
      QString qLevel;
      qLevel.setNum(level, m_Format, m_Precision);
      m_LevelInput->setText(qLevel);
      m_LevelInput->setCursorPosition(0);
      m_LevelInput->setToolTip(QString("Level: %1").arg(qLevel));
      m_LevelWindow.SetLevelWindow(level, m_LevelWindow.GetWindow());
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    double oldLowerBound = m_LevelWindow.GetLowerWindowBound();
    double lowerBound = m_LevelInput->text().toDouble();
    correctLowerBound(lowerBound);
    if (lowerBound != oldLowerBound)
    {
      QString qLowerBound;
      qLowerBound.setNum(lowerBound, m_Format, m_Precision);
      m_LevelInput->setText(qLowerBound);
      m_LevelInput->setCursorPosition(0);
      m_LevelInput->setToolTip(QString("Lower bound: %1").arg(qLowerBound));
      m_LevelWindow.SetWindowBounds(lowerBound, m_LevelWindow.GetUpperWindowBound());
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

//read the windowInput and change window and slider when the button "ENTER" was pressed in the windowInput-LineEdit
void QmitkLineEditLevelWindowWidget::SetWindowValue()
{
  if (m_IntensityRangeType == LevelWindow)
  {
    double oldWindow = m_LevelWindow.GetWindow();
    double window = m_WindowInput->text().toDouble();
    correctWindow(window);
    if (window != oldWindow)
    {
      QString qWindow;
      qWindow.setNum(window, m_Format, m_Precision);
      m_WindowInput->setText(qWindow);
      m_WindowInput->setCursorPosition(0);
      m_WindowInput->setToolTip(QString("Window: %1").arg(qWindow));
      m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), window);
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    double oldUpperBound = m_LevelWindow.GetUpperWindowBound();
    double upperBound = m_WindowInput->text().toDouble();
    correctUpperBound(upperBound);
    if (upperBound != oldUpperBound)
    {
      QString qUpperBound;
      qUpperBound.setNum(upperBound, m_Format, m_Precision);
      m_WindowInput->setText(qUpperBound);
      m_WindowInput->setCursorPosition(0);
      m_WindowInput->setToolTip(QString("Upper bound: %1").arg(qUpperBound));
      m_LevelWindow.SetWindowBounds(m_LevelWindow.GetLowerWindowBound(), upperBound);
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkLineEditLevelWindowWidget::contextMenuEvent( QContextMenuEvent * )
{ 
  m_Contextmenu->setLevelWindowManager(m_Manager.GetPointer());
  m_Contextmenu->getContextMenu();
}

bool QmitkLineEditLevelWindowWidget::correctLevel(double& level)
{
  double window = m_LevelWindow.GetWindow();
  double halfWindow = window / 2;
  double upperLimit = m_LevelWindow.GetRangeMax() - halfWindow;
  if (level > upperLimit)
  {
    level = upperLimit;
    return true;
  }
  else
  {
    double lowerLimit = m_LevelWindow.GetRangeMin() + halfWindow;
    if (level < lowerLimit)
    {
      level = lowerLimit;
      return true;
    }
  }
  return false;
}

bool QmitkLineEditLevelWindowWidget::correctWindow(double& window)
{
  double level = m_LevelWindow.GetLevel();
  double upperLimit = (m_LevelWindow.GetRangeMax() - level) * 2;
  if (window > upperLimit)
  {
    window = upperLimit;
    return true;
  }
  else
  {
    double lowerLimit = (level - m_LevelWindow.GetRangeMin()) * 2;
    if (window < lowerLimit)
    {
      window = lowerLimit;
      return true;
    }
  }
  return false;
}

bool QmitkLineEditLevelWindowWidget::correctLowerBound(double& lowerBound)
{
  double rangeMin = m_LevelWindow.GetRangeMin();
  if(lowerBound < rangeMin)
  {
    lowerBound = rangeMin;
    return true;
  }
  else
  {
    double rangeMax = m_LevelWindow.GetRangeMax();
    if(lowerBound > rangeMax)
    {
      lowerBound = rangeMax;
      return true;
    }
  }
  return false;
}

bool QmitkLineEditLevelWindowWidget::correctUpperBound(double& upperBound)
{
  double rangeMin = m_LevelWindow.GetRangeMin();
  if(upperBound < rangeMin)
  {
    upperBound = rangeMin;
    return true;
  }
  else {
    double rangeMax = m_LevelWindow.GetRangeMax();
    if(upperBound > rangeMax)
    {
      upperBound = rangeMax;
      return true;
    }
  }
  return false;
}

mitk::LevelWindowManager* QmitkLineEditLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}

void QmitkLineEditLevelWindowWidget::SetExponentialFormat(bool value)
{
  m_Format = value ? 'e' : 'f';
}

void QmitkLineEditLevelWindowWidget::SetPrecision(int precision)
{
  m_Precision = precision;
  m_Validator1->setDecimals(precision);
  m_Validator2->setDecimals(precision);
  if (!this->isHidden())
  {
    updateInputs();
  }
}

void QmitkLineEditLevelWindowWidget::SetIntensityRangeType(QmitkLineEditLevelWindowWidget::IntensityRangeType intensityRangeType)
{
  m_IntensityRangeType = intensityRangeType;
  /// The other bounds are set in the constructor and cannot change
  m_Validator2->setBottom(intensityRangeType == LevelWindow ? 0.0 : std::numeric_limits<double>::min());
  if (!this->isHidden())
  {
    updateInputs();
  }
}
