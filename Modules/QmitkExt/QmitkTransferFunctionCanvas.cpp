/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkTransferFunctionCanvas.h"

#include <itkObject.h>

#include <QColorDialog>
#include <QPainter>
#include <QMouseEvent>

QmitkTransferFunctionCanvas::QmitkTransferFunctionCanvas(QWidget * parent,
    Qt::WindowFlags f) :
  QWidget(parent, f), m_GrabbedHandle(-1), m_Lower(0.0f), m_Upper(1.0f), m_Min(
      0.0f), m_Max(1.0f)
{
  setEnabled(false);
  setFocusPolicy(Qt::ClickFocus);
  m_LineEditAvailable = false;
}

void QmitkTransferFunctionCanvas::paintEvent(QPaintEvent* ev)
{
  QWidget::paintEvent(ev);
}

std::pair<int,int> QmitkTransferFunctionCanvas::FunctionToCanvas(
    std::pair<vtkFloatingPointType,vtkFloatingPointType> functionPoint)
{
  //std::cout<<"F2C.first: "<<(int)((functionPoint.first - m_Lower) /  (m_Upper - m_Lower) * width())<<" F2C.second: "<<(int)(height() * (1 - functionPoint.second))<<std::endl;
  return std::make_pair((int) ((functionPoint.first - m_Lower) / (m_Upper
      - m_Lower) * contentsRect().width()) + contentsRect().x(), (int) (contentsRect().height() * (1 - functionPoint.second)) + contentsRect().y());
}

std::pair<vtkFloatingPointType,vtkFloatingPointType> QmitkTransferFunctionCanvas::CanvasToFunction(
    std::pair<int,int> canvasPoint)
{
  //std::cout<<"C2F.first: "<<(canvasPoint.first * (m_Upper - m_Lower) / width() + m_Lower)<<" C2F.second: "<<(1.0 - (vtkFloatingPointType)canvasPoint.second / height())<<std::endl;
  return std::make_pair((canvasPoint.first - contentsRect().x()) * (m_Upper - m_Lower) / contentsRect().width()
      + m_Lower, 1.0 - (vtkFloatingPointType) (canvasPoint.second - contentsRect().y()) / contentsRect().height());
}

void QmitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
  int nearHandle = GetNearHandle(mouseEvent->pos().x(), mouseEvent->pos().y());
  if (nearHandle != -1)
  {
    this->DoubleClickOnHandle(nearHandle);
  }
}

/** returns index of a near handle or -1 if none is near
 */
int QmitkTransferFunctionCanvas::GetNearHandle(int /*x*/, int /*y*/,
    unsigned int /*maxSquaredDistance*/)
{
  return -1;
}


void QmitkTransferFunctionCanvas::mousePressEvent(QMouseEvent* mouseEvent)
{

  if (m_LineEditAvailable)
  {
    m_XEdit->clear();
    if(m_YEdit)
      m_YEdit->clear();
  }

  m_GrabbedHandle = GetNearHandle(mouseEvent->pos().x(), mouseEvent->pos().y());

  if ( (mouseEvent->button() & Qt::LeftButton) && m_GrabbedHandle == -1)
  {
    this->AddFunctionPoint(
        this->CanvasToFunction(std::make_pair(mouseEvent->pos().x(),
            mouseEvent->pos().y())).first,
        this->CanvasToFunction(std::make_pair(mouseEvent->x(), mouseEvent->y())).second);
    m_GrabbedHandle = GetNearHandle(mouseEvent->pos().x(),
        mouseEvent->pos().y());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if ((mouseEvent->button() & Qt::RightButton) && m_GrabbedHandle != -1 && this->GetFunctionSize() > 1)
  {
     this->RemoveFunctionPoint(this->GetFunctionX(m_GrabbedHandle));
     m_GrabbedHandle = -1;
     mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  update();

  // m_TransferFunction->UpdateVtkFunctions();
}

void QmitkTransferFunctionCanvas::mouseMoveEvent(QMouseEvent* mouseEvent)
{
  if (m_GrabbedHandle != -1)
  {
    std::pair<vtkFloatingPointType,vtkFloatingPointType>
        newPos = this->CanvasToFunction(std::make_pair(mouseEvent->x(),
            mouseEvent->y()));
    
    // X Clamping
    {
      // Check with predecessor
      if( m_GrabbedHandle > 0 )
        if (newPos.first <= this->GetFunctionX(m_GrabbedHandle - 1))
          newPos.first = this->GetFunctionX(m_GrabbedHandle);

      // Check with sucessor      
      if( m_GrabbedHandle < this->GetFunctionSize()-1 )
        if (newPos.first >= this->GetFunctionX(m_GrabbedHandle + 1))
          newPos.first = this->GetFunctionX(m_GrabbedHandle);
      
      // Clamping to histogramm
           if (newPos.first < m_Min) newPos.first = m_Min;
      else if (newPos.first > m_Max) newPos.first = m_Max;
    }
        
    // Y Clamping
    {
           if (newPos.second < 0.0) newPos.second = 0.0;
      else if (newPos.second > 1.0) newPos.second = 1.0;
    }
    
    // Move selected point    
    this->MoveFunctionPoint(m_GrabbedHandle, newPos);
    
    /*
    // Search again selected point ??????? should not be required, seems like a legacy workaround/bugfix
    // and no longer required
    m_GrabbedHandle = -1;
    for (int i = 0; i < this->GetFunctionSize(); i++)
    {
      if (this->GetFunctionX(i) == newPos.first)
      {
        m_GrabbedHandle = i;
        break;
      }
    }
    */
    
    update();
    
    //if (m_ImmediateUpdate)
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  }
}

void QmitkTransferFunctionCanvas::mouseReleaseEvent(QMouseEvent*)
{
  // m_GrabbedHandle = -1;
  update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionCanvas::PaintHistogram(QPainter &p)
{
  if (m_Histogram.IsNotNull())
  {
    p.save();
    p.setPen(Qt::gray);
    float scaleFactor = (float) (this->GetHistogram()->GetSize()[0]) / contentsRect().width();
    float maxFreqLog = vcl_log(
        mitk::HistogramGenerator::CalculateMaximumFrequency(
            this->GetHistogram()));
    for (int x = contentsRect().x(); x < contentsRect().x() + contentsRect().width()/* /0.5 */; x++)
    {
      //int tfIndex = static_cast<int> (x  * scaleFactor *0.5);
      int tfIndex = static_cast<int> (x * scaleFactor);
      float freq = this->GetHistogram()->GetFrequency(tfIndex);
      //std::cout<<"x: "<<x<<" tfIndex: "<<tfIndex<<" freq: "<<freq;
      if (freq > 0)
      {
        int y = static_cast<int> ((1 - vcl_log(freq) / maxFreqLog) * contentsRect().height());
        //std::cout<<" y: "<<y<<std::endl;
        //p.drawLine(x-width()*0.5,height(),x-width()*0.5,y);

        p.setPen(Qt::gray);
        p.drawLine(x, contentsRect().height(), x, y);

      }
    }
    p.restore();
  }
}

void QmitkTransferFunctionCanvas::PaintHistogramGO(QPainter&)
{
  //nothing yet...
}

void QmitkTransferFunctionCanvas::keyPressEvent(QKeyEvent * e)
{

  if (m_Min < 0)
  {
    m_Range = (m_Min * -1) + m_Max;
  }
  else
  {
    m_Range = m_Min + m_Max;
  }

  if (e->key() == Qt::Key_Delete && m_GrabbedHandle != -1 && this->GetFunctionSize() > 1 )
  {
    this->RemoveFunctionPoint(GetFunctionX(m_GrabbedHandle));
    m_GrabbedHandle = -1;
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  /**** Move Functionpoints with the arrow keys ****/
  if (e->key() == Qt::Key_Left && m_GrabbedHandle != -1 && m_GrabbedHandle != 0
      && m_GrabbedHandle != this->GetFunctionSize() - 1)
  {
    if (GetFunctionX(m_GrabbedHandle) == m_Max)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Max,
          GetFunctionY(m_GrabbedHandle)));
    }

    else if (GetFunctionX(m_GrabbedHandle) - (m_Range * 0.01) > m_Min)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle) - (m_Range * 0.01), GetFunctionY(m_GrabbedHandle)));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Min,
          GetFunctionY(m_GrabbedHandle)));
    }
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if (e->key() == Qt::Key_Right && m_GrabbedHandle != -1 && m_GrabbedHandle
      != 0 && m_GrabbedHandle != this->GetFunctionSize() - 1)
  {
    if (GetFunctionX(m_GrabbedHandle) == m_Min)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Min,
          GetFunctionY(m_GrabbedHandle)));
    }

    else if (GetFunctionX(m_GrabbedHandle) + (m_Range * 0.01) < m_Max)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle) + (m_Range * 0.01), GetFunctionY(m_GrabbedHandle)));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Max,
          GetFunctionY(m_GrabbedHandle)));
    }
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if (e->key() == Qt::Key_Up && m_GrabbedHandle != -1 && m_GrabbedHandle != 0
      && m_GrabbedHandle != this->GetFunctionSize() - 1 && GetFunctionY(
      m_GrabbedHandle) < 1)
  {
    if (GetFunctionY(m_GrabbedHandle) + 0.01 <= 1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle), GetFunctionY(m_GrabbedHandle) + 0.01));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle), 1));
    }
    update();
    //std::cout<<"X: "<<GetFunctionX(m_GrabbedHandle)<<"Y: "<<GetFunctionY(m_GrabbedHandle)<<std::endl;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  if (e->key() == Qt::Key_Down && m_GrabbedHandle != -1 && m_GrabbedHandle != 0
      && m_GrabbedHandle != this->GetFunctionSize() - 1 && GetFunctionY(
      m_GrabbedHandle) > 0)
  {
    if (GetFunctionY(m_GrabbedHandle) - 0.01 >= 0)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle), GetFunctionY(m_GrabbedHandle) - 0.01));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(
          m_GrabbedHandle), 0));
    }
    update();
    //std::cout<<"X: "<<GetFunctionX(m_GrabbedHandle)<<"Y: "<<GetFunctionY(m_GrabbedHandle)<<std::endl;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

// Update immediatly while changing the transfer function
void QmitkTransferFunctionCanvas::SetImmediateUpdate(bool state)
{
  m_ImmediateUpdate = state;
}
