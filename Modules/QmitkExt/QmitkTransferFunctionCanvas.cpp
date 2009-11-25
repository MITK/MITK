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

void mitk::SimpleHistogram::ComputeFromImage( Image::Pointer source )
{
  valid = false;

  // dummy histogram
  {
    min=0;
    max=1;
    first=0;
    last=1;
  }
  
  //LOG_INFO << "SimpleHistogram: ComputeFromImage started";
  
  if(source->GetDimension() != 3)
  {
    LOG_WARN << "SimpleHistogram only works on 3D-Images";
    return;
  }

  // signed short histogram
  {
    first=-32768;  last=32767;

    if(histogram)
      delete histogram;
      
    histogram = new CountType[last-first+1];
    memset(histogram,0,sizeof(CountType)*(last-first+1));
    highest = 0;
    max = first - 1;
    min = last + 1;
    
    // converting mitk image -> itk image
    CTImage::Pointer work = CTImage::New();
    CastToItkImage( source, work );
    CTIteratorIndexType workIt( work, work->GetRequestedRegion() );
    
    workIt.GoToBegin();

    while ( ! workIt.IsAtEnd() )
    {
      int value = workIt.Get();

      if(value >= first && value <= last)
      {
        if(value < min) min = value;
        if(value > max) max = value;
        CountType tmp = ++histogram[value-first];
        if(tmp > highest) highest = tmp;
      }
      
      ++workIt;
    }
  }
  
  invLogHighest = 1.0/log(double(highest));
  valid = true;
}


float mitk::SimpleHistogram::GetRelativeBin( float left, float right )
{
  if( !valid )
    return 0.0f;
    
  int iLeft = floorf(left);
  int iRight = ceilf(right);

 /*
  double sum = 0;

  for( int r = 0 ; r < 256 ; r++)
  {
    int pos = left + (right-left) * r/255.0;
    int posInArray = floorf(pos+0.5f) - first;
    sum += float(log(double(histogram[posInArray])));
  }

  sum /= 256.0;
  return float(sum*invLogHighest); 
 */

  CountType maximum = 0;
  
  for( int i = iLeft; i <= iRight ; i++)   
  {
    int posInArray = i - first;
    if( histogram[posInArray] > maximum ) maximum = histogram[posInArray];
  }
  
  return float(log(double(maximum))*invLogHighest);  


   

}

  
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
  if(m_Histogram)
  {
    p.save();
    
    p.setPen(Qt::gray);
        
    int displayWidth = contentsRect().width();
    int displayHeight = contentsRect().height();
  
    int windowLeft = m_Lower;
    int windowRight = m_Upper;

    double step = (windowRight-windowLeft)/double(displayWidth);
  
    double pos = windowLeft;
  
    for (int x = 0; x < displayWidth; x++)
    {
      float left = pos;
      float right = pos + step;
      
      float height = m_Histogram->GetRelativeBin( left , right );

      if (height >= 0)
        p.drawLine(x, displayHeight*(1-height), x, displayHeight);
      
      pos += step;
    }
    
    p.restore();
  }
}




void QmitkTransferFunctionCanvas::keyPressEvent(QKeyEvent * e)
{
  if( m_GrabbedHandle == -1)
    return;

  switch(e->key())
  {
    case Qt::Key_Delete:
      if(this->GetFunctionSize() > 1)
      {
        this->RemoveFunctionPoint(GetFunctionX(m_GrabbedHandle));
        m_GrabbedHandle = -1;
      }
      break;

    case Qt::Key_Left:
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle)-1 , GetFunctionY(m_GrabbedHandle))));
      break;
  
    case Qt::Key_Right:
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle)+1 , GetFunctionY(m_GrabbedHandle))));
      break;

    case Qt::Key_Up:
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle) , GetFunctionY(m_GrabbedHandle)+0.001)));
      break;
  
    case Qt::Key_Down:
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair( GetFunctionX(m_GrabbedHandle) , GetFunctionY(m_GrabbedHandle)-0.001)));
      break;
  }

  update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

// Update immediatly while changing the transfer function
void QmitkTransferFunctionCanvas::SetImmediateUpdate(bool state)
{
  m_ImmediateUpdate = state;
}
