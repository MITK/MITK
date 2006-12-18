#ifndef FLMITKCOMFORTWIDGET_INCLUDED
#define FLMITKCOMFORTWIDGET_INCLUDED
#include "FLmitkRenderWindow/FLmitkRenderWindow.h"
#include "mitkSliceNavigationController.h"
class FLmitkComfortWidget : public FLmitkRenderWindow {
  public:
    FLmitkComfortWidget(int x,int y,int w,int h,const char* label=NULL) : FLmitkRenderWindow(x,y,w,h,label) {

  }

    mitk::SliceNavigationController::Pointer sliceCtrl;
}
#endif
