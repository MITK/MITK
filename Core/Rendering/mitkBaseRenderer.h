#ifndef BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4
#define BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4

#include "DataTree.h"

namespace mitk {

//##ModelId=3C6E9AA90306
class BaseRenderer : public itk::Object
{
  public:
	/** Standard class typedefs. */
	typedef BaseRenderer             Self;
    typedef itk::Object              Superclass;
	typedef itk::SmartPointer<Self>  Pointer;
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
//	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
    itkTypeMacro(itk::Object,BaseRenderer);

    //##ModelId=3E330B640353
    typedef int MapperSlotId;

    //##ModelId=3D6A1791038B
    virtual void SetData(mitk::DataTreeIterator* iterator);

    //##ModelId=3E330B930328
    virtual Update() = 0;

    //##ModelId=3E330B9C02F9
    virtual Render() = 0;

    //##ModelId=3E3314720003
    virtual SetWindowId(void *id);

    //##ModelId=3E330C4D0395
    static const MapperSlotId defaultMapper;

    //##ModelId=3E33162C00D0
    virtual void PaintGL();

    //##ModelId=3E331632031E
    virtual void InitializeGL();

    //##ModelId=3E33163703D9
    virtual void ResizeGL(int w, int h);

    //##ModelId=3E33163A0261
    virtual void InitRenderer();

  protected:
    //##ModelId=3D6A17780230
    MapperSlotId m_MapperID;

    //##ModelId=3E330D6902E8
    mitk::DataTreeIterator* m_DataTreeIterator;

};

} // namespace mitk



#endif /* BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4 */
