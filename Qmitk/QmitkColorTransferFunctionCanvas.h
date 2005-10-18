#ifndef QMITKCOLORTRANSFERFUNCTIONCANVAS_H_INCLUDED
#define QMITKCOLORTRANSFERFUNCTIONCANVAS_H_INCLUDED

#include "QmitkTransferFunctionCanvas.h"
#include <vtkColorTransferFunction.h>
class QmitkColorTransferFunctionCanvas : public QmitkTransferFunctionCanvas {
  Q_OBJECT

  public:

    QmitkColorTransferFunctionCanvas( QWidget * parent=0, const char * name=0, WFlags f = false );
    virtual void paintEvent( QPaintEvent* e );
    int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 6);
    void SetColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
    {
      this->m_ColorTransferFunction = colorTransferFunction;
      this->SetMin(colorTransferFunction->GetRange()[0]);
      this->SetMax(colorTransferFunction->GetRange()[1]);
      setEnabled(true);
      update();
    };
    void AddFunctionPoint(vtkFloatingPointType x,vtkFloatingPointType val) {
      m_ColorTransferFunction->AddRGBPoint(x,m_ColorTransferFunction->GetRedValue(x),m_ColorTransferFunction->GetGreenValue(x),m_ColorTransferFunction->GetBlueValue(x));  
    };  
    void RemoveFunctionPoint(vtkFloatingPointType x) {
      int old_size = GetFunctionSize();
      m_ColorTransferFunction->RemovePoint(x); 
      if (GetFunctionSize() + 1 != old_size) {
        std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
        std::cout << "called with x=" << x << std::endl;
      }
    };
    vtkFloatingPointType GetFunctionX(int index) {
      return m_ColorTransferFunction->GetDataPointer()[index*4];
    }
    int GetFunctionSize() {
      return m_ColorTransferFunction->GetSize();
    }
    void DoubleClickOnHandle(int handle);
    void MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos);
  protected: 
    vtkColorTransferFunction* m_ColorTransferFunction;

};
#endif
