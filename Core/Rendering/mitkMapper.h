#ifndef MAPPER_H_HEADER_INCLUDED_C1E6EA08
#define MAPPER_H_HEADER_INCLUDED_C1E6EA08

#include "mitkCommon.h"
#include "Geometry3D.h"
#include "LevelWindow.h"
#include "ImageSource.h"
#include "BaseData.h"

namespace mitk {

class DataTreeNode;

//##ModelId=3D6A0EE70237
//##Documentation
//## @brief Base class of all mappers, 2D as well as 3D
//## @ingroup Mapper
//## Base class of all Mappers, 2D as well as 3D. 
//## Subclasses of mitk::Mapper control the creation of rendering primitives
//## that interface to the graphics library (e.g., OpenGL, vtk). 
//## @todo Should Mapper be a subclass of ImageSource?
class Mapper : public ImageSource
{
public:
    mitkClassMacro(Mapper, ImageSource);

	/** Method for creation through the object factory. */
	//   itkNewMacro(Self);  

	//##ModelId=3E32E456028D
    //##Documentation
    //## @brief Set the DataTreeNode containing the data to map
	virtual void SetInput(const mitk::DataTreeNode* data);

    //##ModelId=3E860B9A0378
    //##Documentation
    //## @brief Get the data to map
    BaseData* GetData() const;

    //##ModelId=3E86B0EA00B0
    //##Documentation
    //## @brief Get the DataTreeNode containing the data to map
    mitk::DataTreeNode* GetDataTreeNode() const;
  protected:
    //##ModelId=3E3C337E0162
    Mapper();

    //##ModelId=3E3C337E019E
    virtual ~Mapper();
};

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
