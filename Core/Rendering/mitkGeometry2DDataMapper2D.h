#ifndef MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB
#define MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB

#include "mitkCommon.h"
#include "Mapper2D.h"
#include "mitkGeometry2DData.h"
#include "mitkGLMapper2D.h"
#include "DataTree.h"

namespace mitk {

class BaseRenderer;

//##ModelId=3E639D57030F
//##Documentation
//## @brief OpenGL-based mapper to display a Geometry2D in a 2D window
//## @ingroup Mapper
//## Currently implemented for mapping on PlaneGeometry. 
//## The result is normally a line. An important usage of this class is to show
//## the orientation of the slices displayed in other 2D windows.
//## @todo implement for AbstractTransformGeometry.
class Geometry2DDataMapper2D : public GLMapper2D
{
public:
    //##ModelId=3E6423D2032C
    mitkClassMacro(Geometry2DDataMapper2D, Mapper2D);

    itkNewMacro(Self);

    //##ModelId=3E6423D20341
    //##Documentation
    //## @brief Get the Geometry2DData to map
    const mitk::Geometry2DData * GetInput(void);

    //##ModelId=3E67D77A0109
    virtual void Paint(mitk::BaseRenderer * renderer);

    //##ModelId=3E67E1B90237
    virtual void Update();

protected:
    //##ModelId=3E639E100243
    Geometry2DDataMapper2D();

    //##ModelId=3E639E100257
    virtual ~Geometry2DDataMapper2D();

    //##ModelId=3E67E285024E
    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB */
