#ifndef MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD
#define MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD

#include <set>
#include <string>
#include "mitkBaseRenderer.h"

namespace mitk {
//##ModelId=3E3ECC1201B2
//##Documentation
//## @brief Abstract window/widget class used for rendering
//## @ingroup Renderer
class RenderWindow
{
public:
  typedef std::set<RenderWindow*> RenderWindowSet;

  //##ModelId=3E3ECC13036D
  RenderWindow(const char *name, mitk::BaseRenderer* renderer = NULL)  : m_Name(name), m_Renderer(renderer)
  {
    instances.insert(this);
  };

  //##ModelId=3EF1627602DF
  //##Documentation
  //## @brief Makes the renderwindow the current widget for rendering.
  //##
  //## Needed for example for OpenGL, i.e. makes the widget's rendering 
  //## context the current OpenGL rendering context.
  virtual void MakeCurrent() = 0;

  //##Documentation
  //## @brief  Swaps the screen contents with an off-screen buffer. 
  //##
  //## This only works if the widget's format specifies double buffer mode.
  //## Normally, there is no need to explicitly call this function because it is done automatically 
  //## after each widget repaint, i.e. each time after paintGL() has been executed. 
  virtual void SwapBuffers () = 0;

  //##Documentation
  //## @brief Returns @a true if display list sharing with another window
  //##  was requested and could be provided.
  //## 
  //## For example, a OpenGL system may fail to provide display list sharing 
  //## if the two windows use different formats.
  virtual bool IsSharing () const = 0;

  //##ModelId=3EF59AD202D5
  //##Documentation
  //## @brief Updates and repaints the contents of the renderwindow, if necessary.
  //##
  //## Repainting may be scheduled as a paint event for processing o optimize for 
  //## more speed and less flicker than a call to Repaint() does.
  virtual void Update() = 0;

  //##Documentation
  //## @brief Immediately repaints the contents of the renderwindow
  //##
  //## Renderwindow will be updated before repainting. 
  virtual void Repaint() = 0;

  //##Documentation
  //## @brief Updates the contents of all renderwindows
  static void UpdateAllInstances() {
    for (RenderWindowSet::iterator iter = instances.begin();iter != instances.end();iter++) {
      (*iter)->Update();
    }
  }
  static const RenderWindowSet& GetInstances() {
    return instances;
  } 

  const char * GetName()
  {
    return m_Name.c_str();
  }

  //## @brief return the first RenderWindow created with the given name 
  static const RenderWindow* GetByName(const std::string& name) {
    for (RenderWindowSet::const_iterator iter = instances.begin();iter != instances.end();iter++) {
      if (name == (*iter)->m_Name) {
        return *iter;
      }
    }
    return NULL;
  }   
  virtual ~RenderWindow() 
  {
    instances.erase(this);
  }

  //##Documentation
  //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
  virtual RenderWindow* SharedWidget() const {return NULL;}
  //##Documentation
  //## TEMPORARY FOR IIL COMPATIBILITY - DO NOT USE!!!
  virtual bool isVisible (const float, const float,
    const float, const float) {return true;}

  virtual void InitRenderer() = 0;

  virtual BaseRenderer::Pointer GetRenderer() const {return m_Renderer;} 

protected:
  static RenderWindowSet instances;
  std::string m_Name;
  
  BaseRenderer::Pointer m_Renderer;
};

} // namespace mitk

#endif /* MITKRENDERWINDOW_H_HEADER_INCLUDED_C1EBD0AD */

