#ifndef BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4
#define BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4

#include "DataTree.h"
#include "mitkRenderWindow.h"
#include "Geometry2D.h"
#include "DisplayGeometry.h"
#include "mitkGeometry2DData.h"

namespace mitk {

//##ModelId=3C6E9AA90306
//##Documentation
//## Organizes the rendering process. Contains the DataTree and asks for the
//## Mappers of DataTreeNodes. 
class BaseRenderer : public itk::Object
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E33ECF200C6
	typedef BaseRenderer             Self;
    //##ModelId=3E33ECF200F8
    typedef itk::Object              Superclass;
    //##ModelId=3E33ECF20116
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E33ECF20134
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
//	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
    itkTypeMacro(itk::Object,BaseRenderer);

	  //##ModelId=3E330B640353
    typedef int MapperSlotId;

    //##ModelId=3D6A1791038B
    virtual void SetData(mitk::DataTreeIterator* iterator);

    //##ModelId=3E6423D20245
    virtual mitk::DataTreeIterator* GetData()  const {
		    return m_DataTreeIterator;
    };
    //##ModelId=3E6423D20264
   mitk::RenderWindow* GetRenderWindow() const {
		return m_RenderWindow;
	 }
  
    //##ModelId=3E330B930328
    virtual void Update() = 0;

    //##ModelId=3E330B9C02F9
    virtual void Render() = 0;

    //##ModelId=3E3314720003
    virtual void SetWindowId(void *id);

    //##ModelId=3E330C4D0395
    static const MapperSlotId defaultMapper;

    //##ModelId=3E33162C00D0
    virtual void Paint();

    //##ModelId=3E331632031E
    virtual void Initialize();

    //##ModelId=3E33163703D9
    virtual void Resize(int w, int h);

    //##ModelId=3E33163A0261
    virtual void InitRenderer(mitk::RenderWindow* renderwindow);
    //##ModelId=3E3799250397
    virtual void InitSize(int w, int h);

    itkGetObjectMacro(DisplayGeometry, mitk::DisplayGeometry);
    //##ModelId=3E66CC59026B
    virtual void SetDisplayGeometry(mitk::DisplayGeometry* geometry2d);

    itkGetObjectMacro(WorldGeometry, mitk::Geometry2D);
    //##ModelId=3E66CC590379
    virtual void SetWorldGeometry(mitk::Geometry2D* geometry2d);

    itkGetObjectMacro(DisplayGeometry2DData, mitk::Geometry2DData);
    itkGetObjectMacro(WorldGeometry2DData, mitk::Geometry2DData);

protected:
    //##ModelId=3E3D2F120050
    BaseRenderer();

    //##ModelId=3E3D2F12008C
    //##ModelId=3E3D2F12008C
    virtual ~BaseRenderer();

    //##ModelId=3E3D381A027D
    MapperSlotId m_MapperID;

    //##ModelId=3E330D6902E8
    mitk::DataTreeIterator* m_DataTreeIterator;

    //##ModelId=3E3D2EEB0087
    DisplayGeometry::Pointer m_DisplayGeometry;
    //##ModelId=3E66B9FA022D
    Geometry2D::Pointer m_WorldGeometry;
    //##ModelId=3E66BDF000F4
    Geometry2DData::Pointer m_DisplayGeometry2DData;
    //##ModelId=3E66CC5901C1
    Geometry2DData::Pointer m_WorldGeometry2DData;

    //##ModelId=3E3D1FCA0272
    RenderWindow* m_RenderWindow;

    //##ModelId=3E6423D20213
    unsigned long m_LastUpdateTime;
};

} // namespace mitk



#endif /* BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4 */
