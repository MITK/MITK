#ifndef MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB
#define MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB

#include "mitkCommon.h"
#include "Mapper2D.h"
#include "mitkGeometry2DData.h"

namespace mitk {

class BaseRenderer;

//##ModelId=3E639D57030F
class Geometry2DDataMapper2D : public Mapper2D
{
public:
    //##ModelId=3E6423D2032C
    mitkClassMacro(Geometry2DDataMapper2D, Mapper2D);

    itkNewMacro(Self);

    /** Set/Get the image input of this process object.  */
    //##ModelId=3E6423D20337
    virtual void SetInput( const mitk::Geometry2DData *input );
    //##ModelId=3E6423D20341
    const mitk::Geometry2DData * GetInput(void);
    //##ModelId=3E6423D2034B
    virtual void SetInput( const mitk::BaseData* data );

    virtual void Paint(mitk::BaseRenderer * renderer);
protected:
    //##ModelId=3E639E100243
    Geometry2DDataMapper2D();

    //##ModelId=3E639E100257
    virtual ~Geometry2DDataMapper2D();

};

} // namespace mitk



#endif /* MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB */
