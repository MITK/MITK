#ifndef MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD
#define MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD

#include <qgl.h>
#include <widget.h>
#include <set>
class iilItem;

namespace mitk {
//##ModelId=3E3ECC1201B2
//##Documentation
//## @brief Abstract window/widget class used for rendering
//## @ingroup Renderer
class RenderWindow : public RenderWindowBase
{
public:
    //##ModelId=3E3ECC130358
    RenderWindow(QGLFormat glf, QWidget *parent = 0, const char *name = 0) : RenderWindowBase(glf, parent, name)
    {
      instances.insert(this);
    };

    //##ModelId=3E3ECC13036D
    RenderWindow(QWidget *parent = 0, const char *name = 0) : RenderWindowBase(parent, name)
    {
      instances.insert(this);
    };

    //##ModelId=3EF1627602DF
    //##Documentation
    //## @brief Makes the renderwindow the current widget for 
    //## (e.g., if the renderwindow is an OpenGL-widget) 
    //## OpenGL operations, i.e. makes the widget's rendering context the current 
    //## OpenGL rendering context.
    virtual void MakeCurrent() {};

    //##ModelId=3EF59AD202D5
    //##Documentation
    //## @brief Updates the contents of the renderwindow 
    virtual void Update() {};
  
  //##Documentation
  //## @brief Updates the contents of all renderwindows
  static void UpdateAllInstances() {
    for (std::set<RenderWindow*>::iterator iter = instances.begin();iter != instances.end();iter++) {
	(*iter)->Update();
    }
  }
  
  virtual ~RenderWindow() {
    instances.erase(this);
  }
protected:
  static std::set<RenderWindow*> instances;
};

} // namespace mitk

#endif /* MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD */

