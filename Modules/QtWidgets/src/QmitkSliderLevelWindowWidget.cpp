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

#include <QmitkSliderLevelWindowWidget.h>

// mitk core
#include <mitkRenderingManager.h>

// mitk qt widgets
#include <QmitkLevelWindowWidgetContextMenu.h>

// qt
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

// itk
#include <itkCommand.h>

// c++
#include <cmath>

QmitkSliderLevelWindowWidget::QmitkSliderLevelWindowWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Manager = mitk::LevelWindowManager::New();

  itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::Pointer command =
    itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::New();
  command->SetCallbackFunction(this, &QmitkSliderLevelWindowWidget::OnPropertyModified);
  m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
  m_IsObserverTagSet = true;

  setMouseTracking(true);
  m_Resize = false;
  m_Bottom = false;
  m_CtrlPressed = false;
  m_MouseDown = false;

  m_Font.setPointSize(6);

  m_MoveHeight = height() - 25;
  m_ScaleVisible = true;
  m_Contextmenu = new QmitkLevelWindowWidgetContextMenu(this);

  this->hide();
  Update();
}

QmitkSliderLevelWindowWidget::~QmitkSliderLevelWindowWidget()
{
  if (m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
}

void QmitkSliderLevelWindowWidget::SetLevelWindowManager(mitk::LevelWindowManager *levelWindowManager)
{
  if (m_IsObserverTagSet)
  {
    m_Manager->RemoveObserver(m_ObserverTag);
    m_IsObserverTagSet = false;
  }
  m_Manager = levelWindowManager;
  if (m_Manager.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::Pointer command =
      itk::ReceptorMemberCommand<QmitkSliderLevelWindowWidget>::New();
    command->SetCallbackFunction(this, &QmitkSliderLevelWindowWidget::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;
  }
}

void QmitkSliderLevelWindowWidget::OnPropertyModified(const itk::EventObject &)
{
  try
  {
    m_LevelWindow = m_Manager->GetLevelWindow();
    this->show();
    Update();
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

void QmitkSliderLevelWindowWidget::paintEvent(QPaintEvent *itkNotUsed(e))
{
  QPixmap pm(width(), height());
  pm.fill(this->palette().color(this->backgroundRole()));
  QPainter painter(&pm);

  painter.setFont(m_Font);
  painter.setPen(this->palette().color(this->foregroundRole()));

  QColor c(51, 153, 204);
  QColor cl = c.light();
  QColor cd = c.dark();

  painter.setBrush(c);
  painter.drawRect(m_Rect);

  float mr = m_LevelWindow.GetRange();
  float smallestLevelableValue = 1e-9;

  //This check is needed as safe guard. LevelWindow is refactored to only deduce finite ranges
  //from images, but old scene serialization may contain infinite ranges that overwrite the new
  //business logic. This roots in two many "jobs" LevelWindow" is used for; see also T24962.
  //Until LevelWindow and this widget is refactored the check was the minimal invasive fix.
  if (!std::isfinite(mr))
  {
    mr = m_LevelWindow.GetWindow();
  }


  // avoiding a division by 0 while still enabling small level windows
  if (mr < smallestLevelableValue)
    mr = smallestLevelableValue;

  float fact = (float)m_MoveHeight / mr;

  // begin draw scale
  if (m_ScaleVisible)
  {
    double minRange = (double)m_LevelWindow.GetRangeMin();
    double maxRange = (double)m_LevelWindow.GetRangeMax();

    //This check is needed as safe guard. LevelWindow is refactored to only deduce finite ranges
    //from images, but old scene serialization may contain infinite ranges that overwrite the new
    //business logic. This roots in two many "jobs" LevelWindow" is used for; see also T24962.
    //Until LevelWindow and this widget is refactored the check was the minimal invasive fix.
    if (!std::isfinite(minRange))
    {
      minRange = m_LevelWindow.GetLowerWindowBound();
    }
    //This check is needed as safe guard. LevelWindow is refactored to only deduce finite ranges
    //from images, but old scene serialization may contain infinite ranges that overwrite the new
    //business logic. This roots in two many "jobs" LevelWindow" is used for; see also T24962.
    //Until LevelWindow and this widget is refactored the check was the minimal invasive fix.
    if (!std::isfinite(maxRange))
    {
      maxRange = m_LevelWindow.GetUpperWindowBound();
    }

    int yValue = m_MoveHeight + (int)(minRange * fact);
    QString s = " 0";
    if (minRange < 0 && maxRange > 0)
    {
      painter.drawLine(5, yValue, 15, yValue);
      painter.drawText(21, yValue + 3, s);
    }

    int count = 1;
    int k = 5;
    bool enoughSpace = false;
    bool enoughSpace2 = false;

    double dStepSize = pow(10, floor(log10(mr / 100)) + 1);

    for (int i = m_MoveHeight + (int)(minRange * fact); i < m_MoveHeight;) // negative
    {

      if (-count * dStepSize < minRange)
      {
        break;
      }

      yValue = m_MoveHeight + (int)((minRange + count * dStepSize) * fact);

      s = QString::number(-count * dStepSize);
      if (count % k && ((dStepSize * fact) > 2.5))
      {
        painter.drawLine(8, yValue, 12, yValue);
        enoughSpace = true;
      }
      else if (!(count % k))
      {
        if ((k * dStepSize * fact) > 7)
        {
          painter.drawLine(5, yValue, 15, yValue);
          painter.drawText(21, yValue + 3, s);
          enoughSpace2 = true;
        }
        else
        {
          k += 5;
        }
      }
      if (enoughSpace)
      {
        i = yValue;
        count++;
      }
      else if (enoughSpace2)
      {
        i = yValue;
        count += k;
      }
      else
      {
        i = yValue;
        count = k;
      }
    }

    count = 1;
    k = 5;
    enoughSpace = false;
    enoughSpace2 = false;
    for (int i = m_MoveHeight + (int)(minRange * fact); i >= 0;)
    {
      if (count * dStepSize > maxRange)
      {
        break;
      }
      yValue = m_MoveHeight + (int)((minRange - count * dStepSize) * fact);
      s = QString::number(count * dStepSize);
      if (count % k && ((dStepSize * fact) > 2.5))
      {
        if (!(minRange > 0 && (count * dStepSize) < minRange))
          painter.drawLine(8, yValue, 12, yValue);
        enoughSpace = true;
      }
      else if (!(count % k))
      {
        if ((k * dStepSize * fact) > 7)
        {
          if (!(minRange > 0 && (count * dStepSize) < minRange))
          {
            painter.drawLine(5, yValue, 15, yValue);
            painter.drawText(21, yValue + 3, s);
          }
          enoughSpace2 = true;
        }
        else
        {
          k += 5;
        }
      }
      if (enoughSpace)
      {
        i = yValue;
        count++;
      }
      else if (enoughSpace2)
      {
        i = yValue;
        count += k;
      }
      else
      {
        i = yValue;
        count = k;
      }
    }
  }
  // end draw scale
  painter.setPen(cl);
  painter.drawLine(m_Rect.topLeft(), m_Rect.topRight());
  painter.drawLine(m_Rect.topLeft(), m_Rect.bottomLeft());

  painter.setPen(cd);
  painter.drawLine(m_Rect.topRight(), m_Rect.bottomRight());
  painter.drawLine(m_Rect.bottomRight(), m_Rect.bottomLeft());
  painter.end();

  QPainter p(this);
  p.drawPixmap(0, 0, pm);
}

void QmitkSliderLevelWindowWidget::mouseMoveEvent(QMouseEvent *mouseEvent)
{
  if (!mouseEvent)
    return;
  if (m_LevelWindow.IsFixed())
    return;
  if (!m_MouseDown)
  {
    if (mouseEvent->pos().y() >= 0 && mouseEvent->pos().y() <= (m_Rect.topLeft().y() + 3))
    {
      setCursor(Qt::SizeVerCursor);
      m_UpperBound.setRect(m_Rect.topLeft().x(), m_Rect.topLeft().y() - 3, 17, 7);
      this->setToolTip("Ctrl + left click to change only upper bound");
      m_Resize = true;
    }
    else if (mouseEvent->pos().y() >= (m_Rect.bottomLeft().y() - 3))
    {
      setCursor(Qt::SizeVerCursor);
      m_LowerBound.setRect(m_Rect.bottomLeft().x(), m_Rect.bottomLeft().y() - 3, 17, 7);
      this->setToolTip("Ctrl + left click to change only lower bound");
      m_Resize = true;
      m_Bottom = true;
    }
    else
    {
      setCursor(Qt::ArrowCursor);
      this->setToolTip("Left click and mouse move to adjust the slider");
      m_Resize = false;
      m_Bottom = false;
    }
  }

  else
  {
    float fact = (float)m_MoveHeight / m_LevelWindow.GetRange();

    if (m_Leftbutton)
    {
      if (m_Resize && !m_CtrlPressed)
      {
        double diff = (mouseEvent->pos().y()) / fact;
        diff -= (m_StartPos.y()) / fact;
        m_StartPos = mouseEvent->pos();

        if (diff == 0)
          return;
        float value;
        if (m_Bottom)
          value = m_LevelWindow.GetWindow() + ((2 * diff));
        else
          value = m_LevelWindow.GetWindow() - ((2 * diff));

        if (value < 0)
          value = 0;

        m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), value);
      }
      else if (m_Resize && m_CtrlPressed)
      {
        if (!m_Bottom)
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (m_StartPos.y()) / fact;
          m_StartPos = mouseEvent->pos();

          if (diff == 0)
            return;
          float value;

          value = m_LevelWindow.GetWindow() - ((diff));

          if (value < 0)
            value = 0;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = m_LevelWindow.GetWindow();
          oldLevel = m_LevelWindow.GetLevel();
          newLevel = oldLevel + (value - oldWindow) / 2;
          if (!((newLevel + value / 2) > m_LevelWindow.GetRangeMax()))
            m_LevelWindow.SetLevelWindow(newLevel, value);
        }
        else
        {
          double diff = (mouseEvent->pos().y()) / fact;
          diff -= (m_StartPos.y()) / fact;
          m_StartPos = mouseEvent->pos();

          if (diff == 0)
            return;
          float value;

          value = m_LevelWindow.GetWindow() + ((diff));

          if (value < 0)
            value = 0;
          float oldWindow;
          float oldLevel;
          float newLevel;
          oldWindow = m_LevelWindow.GetWindow();
          oldLevel = m_LevelWindow.GetLevel();
          newLevel = oldLevel - (value - oldWindow) / 2;
          if (!((newLevel - value / 2) < m_LevelWindow.GetRangeMin()))
            m_LevelWindow.SetLevelWindow(newLevel, value);
        }
      }
      else
      {
        const float minv = m_LevelWindow.GetRangeMin();

        const float level = (m_MoveHeight - mouseEvent->pos().y()) / fact + minv;

        double diff = (mouseEvent->pos().x()) / fact;
        diff -= (m_StartPos.x()) / fact;
        m_StartPos = mouseEvent->pos();

        float window;
        if (m_Bottom)
          window = m_LevelWindow.GetWindow() + ((2 * diff));
        else
          window = m_LevelWindow.GetWindow() - ((2 * diff));

        if (window < 0)
          window = 0;

        m_LevelWindow.SetLevelWindow(level, window);
      }
      m_Manager->SetLevelWindow(m_LevelWindow);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkSliderLevelWindowWidget::enterEvent(QEvent * /*event*/)
{
  QPoint p = QCursor::pos();
  p = this->mapFromGlobal(p);
  QMouseEvent ev(QEvent::MouseMove, p, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
  this->mouseMoveEvent(&ev);
}

void QmitkSliderLevelWindowWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
  if (m_LevelWindow.IsFixed())
    return;
  m_MouseDown = true;
  m_StartPos = mouseEvent->pos();

  if (mouseEvent->button() == Qt::LeftButton)
  {
    if (mouseEvent->modifiers() == Qt::ControlModifier || mouseEvent->modifiers() == Qt::ShiftModifier)
    {
      m_CtrlPressed = true;
    }
    else
    {
      m_CtrlPressed = false;
    }
    m_Leftbutton = true;
  }
  else
    m_Leftbutton = false;

  mouseMoveEvent(mouseEvent);
}

void QmitkSliderLevelWindowWidget::resizeEvent(QResizeEvent *event)
{
  m_MoveHeight = event->size().height() - 25;
  Update();
}

void QmitkSliderLevelWindowWidget::mouseReleaseEvent(QMouseEvent *)
{
  if (m_LevelWindow.IsFixed())
    return;
  m_MouseDown = false;
}

void QmitkSliderLevelWindowWidget::Update()
{
  int rectWidth;
  if (m_ScaleVisible)
  {
    rectWidth = 17;
    setMinimumSize(QSize(50, 50));
    setMaximumSize(QSize(50, 2000));
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
  }
  else
  {
    rectWidth = 26;
    setMinimumSize(QSize(40, 50));
    setMaximumSize(QSize(50, 2000));
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
  }
  float mr = m_LevelWindow.GetRange();

  if (mr < 1e-9)
    mr = 1e-9;

  float fact = (float)m_MoveHeight / mr;

  float rectHeight = m_LevelWindow.GetWindow() * fact;

  if (rectHeight < 15)
    rectHeight = 15;

  if (m_LevelWindow.GetLowerWindowBound() < 0)
    m_Rect.setRect(2,
                   (int)(m_MoveHeight - (m_LevelWindow.GetUpperWindowBound() - m_LevelWindow.GetRangeMin()) * fact),
                   rectWidth,
                   (int)rectHeight);
  else
    m_Rect.setRect(2,
                   (int)(m_MoveHeight - (m_LevelWindow.GetUpperWindowBound() - m_LevelWindow.GetRangeMin()) * fact),
                   rectWidth,
                   (int)rectHeight);

  QWidget::repaint();
}

void QmitkSliderLevelWindowWidget::contextMenuEvent(QContextMenuEvent *)
{
  m_Contextmenu->SetLevelWindowManager(m_Manager.GetPointer());
  auto contextMenu = new QMenu(this);
  Q_CHECK_PTR(contextMenu);
  if (m_ScaleVisible)
    contextMenu->addAction(tr("Hide Scale"), this, SLOT(HideScale()));
  else
    contextMenu->addAction(tr("Show Scale"), this, SLOT(ShowScale()));
  contextMenu->addSeparator();
  m_Contextmenu->GetContextMenu(contextMenu);

  // Fix: Bug #13327 we need to reset the m_MouseDown value
  // otherwise the cursor is not correctly restored afterwards
  m_MouseDown = false;
}

void QmitkSliderLevelWindowWidget::HideScale()
{
  m_ScaleVisible = false;
  Update();
}

void QmitkSliderLevelWindowWidget::ShowScale()
{
  m_ScaleVisible = true;
  Update();
}

void QmitkSliderLevelWindowWidget::SetDataStorage(mitk::DataStorage *ds)
{
  m_Manager->SetDataStorage(ds);
}

mitk::LevelWindowManager *QmitkSliderLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}
