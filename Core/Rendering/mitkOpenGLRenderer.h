#ifndef MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D
#define MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "DataTree.h"
#include "ImageSliceSelector.h"

#include "picimage.h"

#include <itkCommand.h>

class vtkRenderer;
class vtkRenderWindow;
class vtkLight;
class vtkLightKit;

namespace mitk {

class VtkRenderWindow;

//##ModelId=3C6E9AD90215
//##Documentation
//## @brief OpenGL-based implementation of BaseRenderer
//## @ingroup Renderer
class OpenGLRenderer : public BaseRenderer
{
  public:
    /** Standard class typedefs. */
    //##ModelId=3E691CC70250
    mitkClassMacro(OpenGLRenderer,BaseRenderer);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    
    //##ModelId=3E3D28AB0018
    void SetData(mitk::DataTreeIterator* iterator);

    //##ModelId=3E330D260255
    virtual void Update();

    //##ModelId=3E330D2903CC
    //##Documentation
    //## @brief Do the rendering. 
    //## If necessary, Update() and UpdateVtkActors() is called first.
    virtual void Render();

    //##ModelId=3E3314B0005C
    virtual void SetWindowId(void *id);

    //##ModelId=3EF59AD20235
    virtual void SetMapperID(const MapperSlotId mapperId);

    //##ModelId=3EF162760271
    virtual void MakeCurrent();

    //##ModelId=3E3799420227
    virtual void InitSize(int w, int h);

    

    //##ModelId=3ED91D060305
    //##Documentation
    //## @brief Update the vtk-based-mappers by adding the produced actors to the m_VtkRenderer.
    //## 
    //## @warning Always call Update() before, which checks, whether there are vtk-based-mappers.
	virtual void UpdateVtkActors();

    VtkRenderWindow*  GetVtkRenderWindow(){ return  m_MitkVtkRenderWindow;}
    
    //##Documentation
    //## @brief returns the vtk-renderer associated with the OpenGL renderer.
    vtkRenderer * GetVtkRenderer(){return m_VtkRenderer;};

protected:
    //##ModelId=3E33145B005A
    virtual void Paint();

    //##ModelId=3E33145B0096
    virtual void Initialize();

    //##ModelId=3E33145B00D2
    virtual void Resize(int w, int h);

    //##ModelId=3E33145B0226
    virtual void InitRenderer(mitk::RenderWindow* renderwindow);

    //##ModelId=3E33ECF301AD
    OpenGLRenderer();

    //##ModelId=3E33ECF301B7
    virtual ~OpenGLRenderer();

    virtual void DataChangedEvent(const itk::Object *caller, const itk::EventObject &event);

    itk::MemberCommand<mitk::OpenGLRenderer>::Pointer m_DataChangedCommand;

  private:
    //##ModelId=3E33145A0315
    bool m_InitNeeded;

    //##ModelId=3E33145A0347
    bool m_ResizeNeeded;

    //##ModelId=3EF1627601DB
    bool m_VtkMapperPresent;

    //##ModelId=3E33145A0383
    vtkRenderer* m_VtkRenderer;

    //##ModelId=3E33145A03BF
    VtkRenderWindow* m_MitkVtkRenderWindow;

    //##ModelId=3E33145B001E
    vtkLight* m_Light;

    //##ModelId=3ED91D0602E6
    vtkLightKit* m_LightKit;
};

} // namespace mitk



#endif /* MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D */
