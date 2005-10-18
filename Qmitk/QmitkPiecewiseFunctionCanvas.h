#ifndef QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED
#define QMITKPIECEWISEFUNCTIONCANVAS_H_INCLUDED

#include "QmitkTransferFunctionCanvas.h"
#include <vtkPiecewiseFunction.h>
class QmitkPiecewiseFunctionCanvas : public QmitkTransferFunctionCanvas {
  Q_OBJECT

  public:

    QmitkPiecewiseFunctionCanvas( QWidget * parent=0, const char * name=0, WFlags f = false );
    virtual void paintEvent( QPaintEvent* e );
    int GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 6);
    void SetPiecewiseFunction(vtkPiecewiseFunction* piecewiseFunction)
    {
      this->m_PiecewiseFunction = piecewiseFunction;
      this->SetMin(m_PiecewiseFunction->GetRange()[0]);
      this->SetMax(m_PiecewiseFunction->GetRange()[1]);
      setEnabled(true);
      update();

    };
    void AddFunctionPoint(vtkFloatingPointType x,vtkFloatingPointType val) {
      m_PiecewiseFunction->AddPoint(x,val);
    };  
    void RemoveFunctionPoint(vtkFloatingPointType x) {
      int old_size = GetFunctionSize();
      m_PiecewiseFunction->RemovePoint(x); 
      if (GetFunctionSize() + 1 != old_size) {
        std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
        std::cout << "called with x=" << x << std::endl;
      }
    };
    vtkFloatingPointType GetFunctionX(int index) {
      return m_PiecewiseFunction->GetDataPointer()[index*2];
    }
    int GetFunctionSize() {
      return m_PiecewiseFunction->GetSize();
    }
    void DoubleClickOnHandle(int handle) {};
    void MoveFunctionPoint(int index, std::pair<vtkFloatingPointType,vtkFloatingPointType> pos);
  protected: 
    vtkPiecewiseFunction* m_PiecewiseFunction;

};
#endif

