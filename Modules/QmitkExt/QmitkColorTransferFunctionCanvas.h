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

#ifndef QMITKCOLORTRANSFERFUNCTIONCANVAS_H_INCLUDED
#define QMITKCOLORTRANSFERFUNCTIONCANVAS_H_INCLUDED

#include "QmitkTransferFunctionCanvas.h"

#include <vtkColorTransferFunction.h>

class QMITKEXT_EXPORT QmitkColorTransferFunctionCanvas: public QmitkTransferFunctionCanvas
{
Q_OBJECT

public:

  QmitkColorTransferFunctionCanvas( QWidget* parent = 0, Qt::WindowFlags f = 0 )  ;
  virtual void paintEvent( QPaintEvent* e );
  int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 32);
  void SetTitle(std::string title);

  void SetColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
  {
    this->m_ColorTransferFunction = colorTransferFunction;
    this->SetMin(colorTransferFunction->GetRange()[0]);
    this->SetMax(colorTransferFunction->GetRange()[1]);
    setEnabled(true);
    update();
  };

  void AddFunctionPoint(vtkFloatingPointType x,vtkFloatingPointType /*val*/)
  {
    m_ColorTransferFunction->AddRGBPoint(x,m_ColorTransferFunction->GetRedValue(x),m_ColorTransferFunction->GetGreenValue(x),m_ColorTransferFunction->GetBlueValue(x));
  };

  void RemoveFunctionPoint(vtkFloatingPointType x)
  {
    int old_size = GetFunctionSize();
    m_ColorTransferFunction->RemovePoint(x);
    if (GetFunctionSize() + 1 != old_size)
    {
      std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
      std::cout << "called with x=" << x << std::endl;
    }
  };

  vtkFloatingPointType GetFunctionX(int index)
  {
    return m_ColorTransferFunction->GetDataPointer()[index*4];
  }

  int GetFunctionSize()
  {
    return m_ColorTransferFunction->GetSize();
  }

  void DoubleClickOnHandle(int handle);
  void MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos);

  void AddRGB(double x, double r, double g, double b);

  double GetFunctionMax()
  {
    return m_ColorTransferFunction->GetRange()[1];
  }

  double GetFunctionMin()
  {
    return m_ColorTransferFunction->GetRange()[0];
  }

  double GetFunctionRange()
  {
    double range;
    if((m_ColorTransferFunction->GetRange()[0])==0)
    {
      range = m_ColorTransferFunction->GetRange()[1];
      return range;
    }
    else
    {
      range = (m_ColorTransferFunction->GetRange()[1])-(m_ColorTransferFunction->GetRange()[0]);
      return range;
    }
  }

  void RemoveAllFunctionPoints()
  {
    m_ColorTransferFunction->AddRGBSegment(this->GetFunctionMin(),1,0,0,this->GetFunctionMax(),1,1,0);
  }

  float GetFunctionY(int)
  {
    return 0.0;
  }

protected:
  
  vtkColorTransferFunction* m_ColorTransferFunction;
	std::string m_Title;

};
#endif
