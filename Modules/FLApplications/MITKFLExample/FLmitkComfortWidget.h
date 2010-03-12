#ifndef FLMITKCOMFORTWIDGET_INCLUDED
#define FLMITKCOMFORTWIDGET_INCLUDED
#include "FLmitkRenderWindow/FLmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
#include "mitkDataNode.h"
class FLmitkComfortWidget : public FLmitkRenderWindow {
  public:
    FLmitkComfortWidget(int x,int y,int w,int h,const char* label=NULL);

    void SetNode(mitk::DataNode::Pointer node) {m_Node = node;}
    int handle(int event);
    mitk::SliceNavigationController::Pointer sliceCtrl;
  int m_LastX,m_LastY;
    mitk::DataNode::Pointer m_Node;     
};
#endif
