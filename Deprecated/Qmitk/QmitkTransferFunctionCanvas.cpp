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

#include "mitkRenderingManager.h"
#include "QmitkTransferFunctionCanvas.h"

#include <itkObject.h>

#include <qcolordialog.h>
#include <qlineedit.h>
#include <qpainter.h>

#include <sstream>
#include <iostream>
#include <assert.h>
#include <cmath>


QmitkTransferFunctionCanvas::QmitkTransferFunctionCanvas(QWidget * parent, const char * name, WFlags f) : QWidget(parent, name, f), m_GrabbedHandle(-1), m_Lower(0.0f), m_Upper(1.0f), m_Min(0.0f), m_Max(1.0f)
{
  setEnabled(false);
  setFocusPolicy(QWidget::ClickFocus);
  m_LineEditAvailable = false;
}

void QmitkTransferFunctionCanvas::paintEvent(QPaintEvent* ev)
{
  QWidget::paintEvent(ev);
}
std::pair<int,int> QmitkTransferFunctionCanvas::FunctionToCanvas(std::pair<vtkFloatingPointType,vtkFloatingPointType> functionPoint)
{
  //std::cout<<"F2C.first: "<<(int)((functionPoint.first - m_Lower) /  (m_Upper - m_Lower) * width())<<" F2C.second: "<<(int)(height() * (1 - functionPoint.second))<<std::endl;
  return std::make_pair((int)((functionPoint.first - m_Lower) /  (m_Upper - m_Lower) * width()),(int)(
        height() * (1 - functionPoint.second)));
}
std::pair<vtkFloatingPointType,vtkFloatingPointType> QmitkTransferFunctionCanvas::CanvasToFunction(std::pair<int,int> canvasPoint)
{
  //std::cout<<"C2F.first: "<<(canvasPoint.first * (m_Upper - m_Lower) / width() + m_Lower)<<" C2F.second: "<<(1.0 - (vtkFloatingPointType)canvasPoint.second / height())<<std::endl;
  return std::make_pair(canvasPoint.first * (m_Upper - m_Lower) / width() + m_Lower,  1.0 - (vtkFloatingPointType)canvasPoint.second / height());
}

void QmitkTransferFunctionCanvas::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
  int nearHandle = GetNearHandle(mouseEvent->x(),mouseEvent->y());
  if (nearHandle != -1) {
    this->DoubleClickOnHandle(nearHandle);
  }
}
/** returns index of a near handle or -1 if none is near
*/
int QmitkTransferFunctionCanvas::GetNearHandle(int /*x*/,int /*y*/,unsigned int /*maxSquaredDistance*/)
{
  return -1;
}

void QmitkTransferFunctionCanvas::mousePressEvent( QMouseEvent* mouseEvent )
{   

  if(m_LineEditAvailable)
  {
    m_XEdit->clear();
    m_YEdit->clear();
  }
  
  m_GrabbedHandle = GetNearHandle(mouseEvent->x(),mouseEvent->y());
  
  if (( mouseEvent->state() & Qt::ShiftButton ) &&
      (mouseEvent->button() & Qt::LeftButton) && m_GrabbedHandle == -1)
  {
    // mode = add
    this->AddFunctionPoint(this->CanvasToFunction(std::make_pair(mouseEvent->x(),mouseEvent->y())).first,this->CanvasToFunction(std::make_pair(mouseEvent->x(),mouseEvent->y())).second);
    m_GrabbedHandle = GetNearHandle(mouseEvent->x(),mouseEvent->y());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if ((mouseEvent->button() & Qt::LeftButton) || (mouseEvent->button() & Qt::MidButton))
  {
    if (m_GrabbedHandle != -1)
    {
      m_MoveStart = std::make_pair(mouseEvent->x(),mouseEvent->y());
    }
    /* if ((mouseEvent->button() & Qt::MidButton)  &&
       (mouseEvent->state() & Qt::ShiftButton) &&
       m_GrabbedHandle != -1 && this->GetFunctionSize() >2 )
       {
    // mode = delete
    this->RemoveFunctionPoint(this->GetFunctionX(nearHandle));
    m_GrabbedHandle = -1;

    } */
  }
  update();

  // m_TransferFunction->UpdateVtkFunctions();
}

void QmitkTransferFunctionCanvas::mouseMoveEvent( QMouseEvent* mouseEvent )
{
  if (m_GrabbedHandle != -1)
  {
    std::pair<vtkFloatingPointType,vtkFloatingPointType> newPos = this->CanvasToFunction(std::make_pair(mouseEvent->x(),mouseEvent->y()));
    if (GetFunctionX(m_GrabbedHandle)<=m_Min)
    {
      // left-most point -> x is fixed
      newPos.first = m_Min;
    }
    else if (GetFunctionX(m_GrabbedHandle) >= m_Max)
    {
      // right-most point -> x is fixed
      newPos.first = m_Max;
    }
    else
    {
      // todo: add checks for lower/upper bounds in zoom mode
      //std::cout << "before:" << newPos.first << "/" << newPos.second;
      if (newPos.first <= this->GetFunctionX(m_GrabbedHandle-1)) newPos.first = this->GetFunctionX(m_GrabbedHandle);
      if (newPos.first >= this->GetFunctionX(m_GrabbedHandle+1)) newPos.first = this->GetFunctionX(m_GrabbedHandle);
    }
    if (newPos.second < 0.0) newPos.second = 0.0;
    if (newPos.second > 1.0) newPos.second = 1.0;
    //std::cout << " after:" << newPos.first << "/" << newPos.second << std::endl;
    //this->RemoveFunctionPoint(this->GetFunctionX(m_GrabbedHandle));
    this->MoveFunctionPoint(m_GrabbedHandle,newPos);
    m_GrabbedHandle = -1;
    //this->AddFunctionPoint(newPos.first,newPos.second);
    for (int i=0 ; i<this->GetFunctionSize() ; i++)
    {
      if (this->GetFunctionX(i) == newPos.first)
      {
        m_GrabbedHandle = i;
        break;
      }
    }
    assert(m_GrabbedHandle != -1);
    update();
    if(m_ImmediateUpdate)
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
void QmitkTransferFunctionCanvas::mouseReleaseEvent( QMouseEvent*  )
{
  // m_GrabbedHandle = -1;
  update();
  mitk::RenderingManager::GetInstance()->SetNextLOD(2);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
};

void QmitkTransferFunctionCanvas::PaintHistogram(QPainter &p)
{
  if (m_Histogram.IsNotNull())
  {
    p.save();
    p.setPen(Qt::blue);
    float scaleFactor = (float)(this->GetHistogram()->GetSize()[0]) / width();
    float maxFreqLog = vcl_log(mitk::HistogramGenerator::CalculateMaximumFrequency(this->GetHistogram()));
    for (int x = 0; x<width()/* /0.5 */; x++)
    {
      //int tfIndex = static_cast<int> (x  * scaleFactor *0.5);
      int tfIndex = static_cast<int> ( x * scaleFactor );
      float freq = this->GetHistogram()->GetFrequency(tfIndex);
      //std::cout<<"x: "<<x<<" tfIndex: "<<tfIndex<<" freq: "<<freq;
      if (freq>0)
      {
        int y = static_cast<int>( ( 1 - vcl_log(freq) / maxFreqLog ) * height() );
        //std::cout<<" y: "<<y<<std::endl;
        //p.drawLine(x-width()*0.5,height(),x-width()*0.5,y);
        
        p.setPen(Qt::blue);        
        p.drawLine(x,height(),x,y);
        
      }
    }
    p.restore();
  }
}

void QmitkTransferFunctionCanvas::PaintHistogramGO(QPainter&)
{ 
//nothing yet...
}

void QmitkTransferFunctionCanvas::keyPressEvent ( QKeyEvent * e ) {
  
  if(m_Min<0)
  {
    m_Range = (m_Min*-1)+m_Max;
  }
  else
  {
    m_Range = m_Min+m_Max;
  }
  
  if (e->key() == Qt::Key_Delete && 
      m_GrabbedHandle  != -1 && 
      m_GrabbedHandle != 0 && 
      m_GrabbedHandle != this->GetFunctionSize()-1 ) {
    this->RemoveFunctionPoint(GetFunctionX(m_GrabbedHandle));
    m_GrabbedHandle = -1;
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  
  /**** Move Functionpoints with the arrow keys ****/
  if (e->key() == Qt::Key_Left && 
      m_GrabbedHandle  != -1 && 
      m_GrabbedHandle != 0 && 
      m_GrabbedHandle != this->GetFunctionSize()-1 )
  {
    if (GetFunctionX(m_GrabbedHandle) == m_Max)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Max,GetFunctionY(m_GrabbedHandle)));
    } 
    
    else if (GetFunctionX(m_GrabbedHandle)-(m_Range*0.01)>m_Min)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle)-(m_Range*0.01),GetFunctionY(m_GrabbedHandle)));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Min,GetFunctionY(m_GrabbedHandle)));
    }  
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();   
  }
  
  if (e->key() == Qt::Key_Right && 
      m_GrabbedHandle  != -1 && 
      m_GrabbedHandle != 0 && 
      m_GrabbedHandle != this->GetFunctionSize()-1 )
  {   
    if (GetFunctionX(m_GrabbedHandle) == m_Min)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Min,GetFunctionY(m_GrabbedHandle)));
    }
    
    else if (GetFunctionX(m_GrabbedHandle)+(m_Range*0.01)<m_Max)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle)+(m_Range*0.01),GetFunctionY(m_GrabbedHandle)));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(m_Max,GetFunctionY(m_GrabbedHandle)));
    }
    update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 
  }
  
  if (e->key() == Qt::Key_Up && 
      m_GrabbedHandle  != -1 && 
      m_GrabbedHandle != 0 && 
      m_GrabbedHandle != this->GetFunctionSize()-1 &&
      GetFunctionY(m_GrabbedHandle)<1)
  {
    if(GetFunctionY(m_GrabbedHandle)+0.01<=1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle),GetFunctionY(m_GrabbedHandle)+0.01));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle),1));
    }    
    update();
    //std::cout<<"X: "<<GetFunctionX(m_GrabbedHandle)<<"Y: "<<GetFunctionY(m_GrabbedHandle)<<std::endl;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();   
  }
  
  if (e->key() == Qt::Key_Down && 
      m_GrabbedHandle  != -1 && 
      m_GrabbedHandle != 0 && 
      m_GrabbedHandle != this->GetFunctionSize()-1 &&
      GetFunctionY(m_GrabbedHandle) >0)
  {
    if(GetFunctionY(m_GrabbedHandle)-0.01>=0)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle),GetFunctionY(m_GrabbedHandle)-0.01));
    }
    else
    {
      this->MoveFunctionPoint(m_GrabbedHandle, std::make_pair(GetFunctionX(m_GrabbedHandle),0));
    }
    update();
    //std::cout<<"X: "<<GetFunctionX(m_GrabbedHandle)<<"Y: "<<GetFunctionY(m_GrabbedHandle)<<std::endl;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();   
  }  
}

// Update immediatly while changing the transfer function
void QmitkTransferFunctionCanvas::SetImmediateUpdate(bool state)
{
  m_ImmediateUpdate=state;
}
