#ifndef MAPPER_H_HEADER_INCLUDED_C1E6EA08
#define MAPPER_H_HEADER_INCLUDED_C1E6EA08

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "LevelWindow.h"
#include "ImageSource.h"

namespace mitk {

//##ModelId=3D6A0EE70237
class Mapper : public ImageSource
{
public:
	/** Standard class typedefs. */
	//##ModelId=3E32C49C031E
	typedef Mapper              Self;
	//##ModelId=3E32C49C0346
	typedef ImageSource         Superclass;
	//##ModelId=3E32C49C0364
	typedef itk::SmartPointer<Self>  Pointer;
	//##ModelId=3E32C49C0378
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Method for creation through the object factory. */
	//   itkNewMacro(Self);  

	/** Run-time type information (and related methods). */
	itkTypeMacro(ImageSource,BaseProcess);

	//##ModelId=3E32E456028D
	virtual void SetInput(const mitk::BaseData* data) = 0;
 //   //##ModelId=3E3C320001E4
 //   virtual void SetDisplaySizeInDisplayUnits(unsigned int widthInDisplayUnits, unsigned int heightInDisplayUnits);

  protected:
    //##ModelId=3E3C337E0162
    Mapper();

    //##ModelId=3E3C337E019E
    //##ModelId=3E3C337E019E
    virtual ~Mapper();

    ////##ModelId=3E3C322800FC
    //Vector2D m_DisplaySizeInDisplayUnits;
};


} // namespace mitk



#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
