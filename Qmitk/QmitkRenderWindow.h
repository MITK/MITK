#ifndef QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66
#define QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66

#include <qgl.h>
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"

namespace mitk {

//##ModelId=3DD521FB01A9
//##Documentation
//## @brief Qt implementation of mitk::RenderWindow
//## @ingroup Renderer
class QmitkRenderWindow : public QGLWidget, public mitk::RenderWindow
{
public:
  //##ModelId=3E1EB4410304
  QmitkRenderWindow(mitk::BaseRenderer* renderer, QGLFormat glf, QWidget *parent = 0, const char *name = 0);

  //##ModelId=3E1EB4410318
  QmitkRenderWindow(mitk::BaseRenderer* renderer, QWidget *parent = 0, const char *name = 0);

  //##ModelId=3E1EB441032C
  virtual ~QmitkRenderWindow();

  //##ModelId=3E3D1D4A00A5
  virtual void MakeCurrent();

  virtual void SwapBuffers ();

  virtual bool IsSharing () const;

  //##ModelId=3EF59AD202B7
  virtual void Update();

  virtual void Repaint();

protected:
  //##ModelId=3E3314590396
  virtual void paintGL();

  //##ModelId=3E33145903C8
  virtual void initializeGL();

  //##ModelId=3E33145A001C
  virtual void resizeGL(int w, int h);

  //##ModelId=3E33145A00F8
  virtual void showEvent(QShowEvent *);

  //##ModelId=3E33145A0171
  virtual void InitRenderer();

  //##ModelId=3E3314590288
  bool m_InitNeeded;

  //##ModelId=3E33145902C4
  bool m_ResizeNeeded;

public:
  //iil compatibility

  //##ModelId=3E6D5DD40306
  virtual void mousePressEvent(QMouseEvent*);
  //##ModelId=3E6D5DD4032E
  virtual void mouseReleaseEvent(QMouseEvent*);
  //##ModelId=3E6D5DD40356
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void wheelEvent(QWheelEvent*);
  //##ModelId=3E6D5DD40388
  virtual void keyPressEvent(QKeyEvent*);
  //##ModelId=3E6D5DD403B0
  virtual void focusInEvent(QFocusEvent*);
  //##ModelId=3E6D5DD403E2
  virtual void focusOutEvent(QFocusEvent*); 
};

} // namespace mitk



#endif /* QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66 */
