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

#ifndef QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED
#define QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED

#include "QmitkTransferFunctionCanvas.h"

#include <vtkPiecewiseFunction.h>

class QMITK_EXPORT QmitkPiecewiseFunctionCanvas : public QmitkTransferFunctionCanvas 
{
  Q_OBJECT

  public:
    
    QmitkPiecewiseFunctionCanvas( QWidget * parent=0, const char * name=0, WFlags f = false );
    virtual void paintEvent( QPaintEvent* e );
    int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32);

    void SetPiecewiseFunction(vtkPiecewiseFunction* piecewiseFunction)
    {
      this->m_PiecewiseFunction = piecewiseFunction;
      this->SetMin(m_PiecewiseFunction->GetRange()[0]);
      this->SetMax(m_PiecewiseFunction->GetRange()[1]);
      setEnabled(true);
      
      m_IsGradientOpacityFunction = false;

      update();

    };

     void SetPiecewiseFunctionGO(vtkPiecewiseFunction* piecewiseFunction)
     {
        this->m_PiecewiseFunction = piecewiseFunction;
        this->SetMin(m_PiecewiseFunction->GetRange()[0]);
        this->SetMax(m_PiecewiseFunction->GetRange()[1]);
        //this->SetMin(0);
        //this->SetMax((m_PiecewiseFunction->GetRange()[1])*0.25);
        setEnabled(true);

        m_IsGradientOpacityFunction = true;
       
        update();

    };

    void AddFunctionPoint(vtkFloatingPointType x,vtkFloatingPointType val) 
    {
      m_PiecewiseFunction->AddPoint(x,val);
    };  
    void RemoveFunctionPoint(vtkFloatingPointType x) 
    {
      int old_size = GetFunctionSize();
      m_PiecewiseFunction->RemovePoint(x); 
      if (GetFunctionSize() + 1 != old_size) 
      {
        std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
        std::cout << "called with x=" << x << std::endl;
      }
    };
    vtkFloatingPointType GetFunctionX(int index) 
    {
      return m_PiecewiseFunction->GetDataPointer()[index*2];
    }
    
    float GetFunctionY(int index) 
    {
      return m_PiecewiseFunction->GetValue(m_PiecewiseFunction->GetDataPointer()[index*2]);
    }
    
    int GetFunctionSize() 
    {
      return m_PiecewiseFunction->GetSize();
    }
    void DoubleClickOnHandle(int /*handle*/) {};
    void MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos);


    double GetFunctionMax()
    {
      return m_PiecewiseFunction->GetRange()[1];
    }

    double GetFunctionMin()
    {
      return m_PiecewiseFunction->GetRange()[0];
    }

    double GetFunctionRange()
    {
      double range;
      if((m_PiecewiseFunction->GetRange()[0])<0)
      {
        range = (m_PiecewiseFunction->GetRange()[1])-(m_PiecewiseFunction->GetRange()[0]);
        return range;
      }
      else
      {
        range = m_PiecewiseFunction->GetRange()[1];
        return range;
      }
    }

    void RemoveAllFunctionPoints()
    {
      m_PiecewiseFunction->AddSegment(this->GetFunctionMin(),0,this->GetFunctionMax(),1);
      m_PiecewiseFunction->AddPoint(0.0,0.0);
    }

    void ResetGO()
    { //Gradient Opacity
     m_PiecewiseFunction->AddSegment(this->GetFunctionMin(),0,0,1);
     m_PiecewiseFunction->AddSegment(0,1,((this->GetFunctionRange())*0.125),1);
     m_PiecewiseFunction->AddSegment(((this->GetFunctionRange())*0.125),1,((this->GetFunctionRange())*0.2),1);
     m_PiecewiseFunction->AddSegment(((this->GetFunctionRange())*0.2),1,((this->GetFunctionRange())*0.25),1);
    }
    
  protected: 
    vtkPiecewiseFunction* m_PiecewiseFunction;
    bool m_IsGradientOpacityFunction;


};
#endif

