#ifndef BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4
#define BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4

#include "DataTree.h"
#include "Geometry2D.h"
#include "DisplayGeometry.h"
#include "mitkGeometry2DData.h"
#include "CameraController.h"
#include "mitkEventTypedefs.h"

namespace mitk {

class RenderWindow;

//##ModelId=3C6E9AA90306
//##Documentation
//## @brief Organizes the rendering process
//## @ingroup Renderer
//## Organizes the rendering process. A Renderer contains a reference to a
//## (sub-) data 
//## tree and asks the mappers of the data objects to render 
//## the data into the renderwindow it is associated to.
class BaseRenderer : public itk::Object
{
public:
  /** Standard class typedefs. */
  //##ModelId=3E691E0901DB
  mitkClassMacro(BaseRenderer, itk::Object);

  //##ModelId=3E330B640353
  //##Documentation
  //## @brief MapperSlotId defines which kind of mapper (e.g., 2D or 3D) shoud be used.
  typedef int MapperSlotId;

  //##ModelId=3D6A1791038B
  //##Documentation
  //## @brief @a iterator defines which part of the data tree is traversed for renderering.
  virtual void SetData(mitk::DataTreeIterator* iterator);

  //##ModelId=3E6423D20245
  //##Documentation
  //## @brief Get the DataTreeIterator defining which part of the data tree is traversed for renderering.
  virtual mitk::DataTreeIterator* GetData()  const 
  {
    return m_DataTreeIterator;
  };
  //##ModelId=3E6423D20264
  //##Documentation
  //## @brief Access the RenderWindow into which this renderer renders.
  mitk::RenderWindow* GetRenderWindow() const 
  {
    return m_RenderWindow;
  }

  //##ModelId=3E330B930328
  //##Documentation
  //## @brief Call update of all mappers.
  virtual void Update() = 0;

  //##ModelId=3E330B9C02F9
  //##Documentation
  //## @brief Do the rendering. 
  virtual void Render() = 0;

  //##ModelId=3E3314720003
  //##Documentation
  //## @brief Set the window id of the window to draw into.
  virtual void SetWindowId(void *id);

  //##ModelId=3EF1627503C4
  //##Documentation
  //## @brief Makes the widget, this renderer is drawing into, the current widget for 
  //## (e.g., if the renderer is an OpenGL-based renderer as mitk::OpenGLRenderer) 
  //## OpenGL operations, i.e. makes the widget's rendering context the current 
  //## OpenGL rendering context.
  virtual void MakeCurrent();

  //##ModelId=3E330C4D0395
  //##Documentation
  //## @brief Default mapper id to use.
  static const MapperSlotId defaultMapper;

  //##ModelId=3E33162C00D0
  //##Documentation
  //## @brief Do the rendering and flush the result.
  virtual void Paint();

  //##ModelId=3E331632031E
  //##Documentation
  //## @brief Initialize the RenderWindow. Should only be called from RenderWindow.
  virtual void Initialize();

  //##ModelId=3E33163703D9
  //##Documentation
  //## @brief Called to inform the renderer that the RenderWindow has been resized.
  virtual void Resize(int w, int h);

  //##ModelId=3E33163A0261
  //##Documentation
  //## @brief Initialize the renderer with a RenderWindow (@a renderwindow).
  virtual void InitRenderer(mitk::RenderWindow* renderwindow);

  //##ModelId=3E3799250397
  //##Documentation
  //## @brief Set the initial size. Called by RenderWindow after it has become 
  //## visible for the first time.
  virtual void InitSize(int w, int h);

  //##Documentation
  //## @brief Get the DisplayGeometry (for 2D rendering)
  //## 
  //## The DisplayGeometry describes which part of the Geometry2D m_WorldGeometry
  //## is displayed.
  itkGetObjectMacro(DisplayGeometry, mitk::DisplayGeometry);
  //##ModelId=3E66CC59026B
  //##Documentation
  //## @brief Set the DisplayGeometry (for 2D rendering)
  //## 
  //## The DisplayGeometry describes which part of the Geometry2D m_WorldGeometry
  //## is displayed.
  virtual void SetDisplayGeometry(mitk::DisplayGeometry* geometry2d);

  //##Documentation
  //## @brief Get the WorldGeometry (for 2D rendering)
  //## 
  //## The WorldGeometry describes the 2D manifold to be displayed. More precisely,
  //## a subpart of this according to m_DisplayGeometry is displayed.
  itkGetConstObjectMacro(WorldGeometry, mitk::Geometry2D);
  //##ModelId=3E66CC590379
  //##Documentation
  //## @brief Set the WorldGeometry (for 2D rendering)
  //## 
  //## The WorldGeometry describes the 2D manifold to be displayed. More precisely,
  //## a subpart of this according to m_DisplayGeometry is displayed.
  virtual void SetWorldGeometry(const mitk::Geometry2D* geometry2d);

  //##Documentation
  //## @brief Get a data object containing the DisplayGeometry (for 2D rendering)
  itkGetObjectMacro(DisplayGeometryData, mitk::Geometry2DData);
  //##Documentation
  //## @brief Get a data object containing the WorldGeometry (for 2D rendering)
  itkGetObjectMacro(WorldGeometryData, mitk::Geometry2DData);

  //##Documentation
  //## @brief Get a data tree node pointing to a data object containing the DisplayGeometry (for 2D rendering)
  itkGetObjectMacro(DisplayGeometryNode, mitk::DataTreeNode);
  //##Documentation
  //## @brief Get a data tree node pointing to a data object containing the WorldGeometry (for 2D rendering)
  itkGetObjectMacro(WorldGeometryNode, mitk::DataTreeNode);

  //##Documentation
  //## @brief Get the MapperSlotId to use.
  itkGetMacro(MapperID, MapperSlotId);
  //##Documentation
  //## @brief Set the MapperSlotId to use.
  itkSetMacro(MapperID, MapperSlotId);

  //##Documentation
  //## @brief Has the renderer the focus?
  itkGetMacro(Focused, bool);
  //##Documentation
  //## @brief Tell the renderer that it is focused. The caller is responsible for focus management,
  //## not the renderer itself.
  itkSetMacro(Focused, bool);

  itkGetMacro(Size, int*);

  //##ModelId=3E6D5DD30322
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MousePressEvent(mitk::MouseEvent*);
  //##ModelId=3E6D5DD30372
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MouseReleaseEvent(mitk::MouseEvent*);
  //##ModelId=3E6D5DD303C2
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MouseMoveEvent(mitk::MouseEvent*);
  //##ModelId=3E6D5DD4002A
  //##Documentation
  //## @brief Key event dispatcher
  //## @note for internal use only. preliminary.
  virtual void KeyPressEvent(mitk::KeyEvent*);

protected:
  //##ModelId=3E3D2F120050
  BaseRenderer();

  //##ModelId=3E3D2F12008C
  virtual ~BaseRenderer();

  //##ModelId=3E3D381A027D
  //##Documentation
  //## @brief MapperSlotId to use. Defines which kind of mapper (e.g., 2D or 3D) shoud be used.
  MapperSlotId m_MapperID;

  //##ModelId=3E330D6902E8
  //##Documentation
  //## @brief The DataTreeIterator defining which part of the data tree is traversed for renderering.
  mitk::DataTreeIterator* m_DataTreeIterator;

  //##ModelId=3E3D1FCA0272
  //##Documentation
  //## @brief The RenderWindow to render into.
  RenderWindow* m_RenderWindow;

  //##ModelId=3E6423D20213
  //##Documentation
  //## @brief Timestamp of last call of Update().
  unsigned long m_LastUpdateTime;

  //##ModelId=3E6D5DD30232
  //##Documentation
  //## @brief CameraController for 3D rendering
  //## @note preliminary.
  mitk::CameraController::Pointer m_CameraController;

  //##ModelId=3E6D5DD302E6
  //##Documentation
  //## @brief Size of the RenderWindow.
  int m_Size[2];

  //##ModelId=3ED91D0400D3
  //##Documentation
  //## @brief Contains whether the renderer that it is focused. The caller of 
  //## SetFocused is responsible for focus management, not the renderer itself.
  //## is doubled because of mitk::FocusManager in GlobalInteraction!!! (ingmar)
  bool m_Focused;

private:
  //##ModelId=3EDD039F002C
  //##Documentation
  //## ConstPointer to the displaygeometry. The displaygeometry describes the
  //## geometry of the visible area in the window controlled by the renderer 
  //## in case we are doing 2D-rendering. 
  //## It is const, since we are not allowed to change it.
  DisplayGeometry::Pointer m_DisplayGeometry;
  //##ModelId=3EDD039F00A9
  //##Documentation
  //## ConstPointer to the worldgeometry. The worldgeometry describes the
  //## maximal area to be rendered in case we are doing 2D-rendering. 
  //## It is const, since we are not allowed to change it (it may be taken
  //## directly from the geometry of an image-slice and thus it would be 
  //## very strange when suddenly the image-slice changes its geometry).
  Geometry2D::ConstPointer m_WorldGeometry;

protected:
  //##ModelId=3E66BDF000F4
  //##Documentation
  //## Data object containing the m_DisplayGeometry defined above. 
  Geometry2DData::Pointer m_DisplayGeometryData;
  //##ModelId=3E66CC5901C1
  //##Documentation
  //## Data object containing the m_WorldGeometry defined above. 
  Geometry2DData::Pointer m_WorldGeometryData;

  //##ModelId=3ED91D04020B
  //##Documentation
  //## DataTreeNode objects containing the m_DisplayGeometryData defined above. 
  DataTreeNode::Pointer m_DisplayGeometryNode;
  //##ModelId=3ED91D040288
  //##Documentation
  //## DataTreeNode objects containing the m_WorldGeometryData defined above. 
  DataTreeNode::Pointer m_WorldGeometryNode;

  //##ModelId=3ED91D040305
  //##Documentation
  //## @brief test only
  unsigned long m_DisplayGeometryTransformTime;
  //##ModelId=3ED91D040382
  //##Documentation
  //## @brief test only
  unsigned long m_WorldGeometryTransformTime;
};

} // namespace mitk

#endif /* BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4 */
