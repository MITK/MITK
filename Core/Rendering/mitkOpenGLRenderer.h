#ifndef MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D
#define MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include "DataTree.h"
#include "ImageSliceSelector.h"

#include "picimage.h"

class vtkRenderer;
class vtkRenderWindow;
class vtkLight;

namespace mitk {

//##ModelId=3C6E9AD90215
class OpenGLRenderer : public BaseRenderer
{
  public:
    /** Standard class typedefs. */
    //##ModelId=3E33ECF2026A
    typedef OpenGLRenderer              Self;
    //##ModelId=3E33ECF20275
    typedef BaseRenderer             Superclass;
    //##ModelId=3E33ECF20288
    typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E33ECF20292
    typedef itk::SmartPointer<const Self>  ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(BaseRenderer,OpenGLRenderer);


    
    //##ModelId=3E3D28AB0018
    void SetData(mitk::DataTreeIterator* iterator);

    //##ModelId=3E330D260255
    virtual void Update();

    //##ModelId=3E330D2903CC
    virtual void Render();

    //##ModelId=3E3314B0005C
    virtual void SetWindowId(void *id);

    //##ModelId=3E3799420227
    virtual void InitSize(int w, int h);

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

  private:
    //##ModelId=3E33145A0315
    bool m_InitNeeded;

    //##ModelId=3E33145A0347
    bool m_ResizeNeeded;

    //##ModelId=3E33145A0383
    vtkRenderer* m_VtkRenderer;

    //##ModelId=3E33145A03BF
    vtkRenderWindow* m_VtkRenderWindow;

    //##ModelId=3E33145B001E
    vtkLight* m_Light;
  public: //FIXME
		iilPicImage* m_IilImage;
};

} // namespace mitk



#endif /* MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D */
