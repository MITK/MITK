#ifndef MITK_CONTOUR_MAPPER_2D_H_
#define MITK_CONTOUR_MAPPER_2D_H_

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkContour.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

//##ModelId=3F0189F0014F
//##Documentation
//## @brief OpenGL-based mapper to display a Geometry2D in a 2D window
//## @ingroup Mapper
//## Currently implemented for mapping on PlaneGeometry. 
//## The result is normally a line. An important usage of this class is to show
//## the orientation of the slices displayed in other 2D windows.
//## @todo implement for AbstractTransformGeometry.
class ContourMapper2D : public GLMapper2D
{
public:
    //##ModelId=3F0189F00363
    mitkClassMacro(ContourMapper2D, Mapper2D);

    //##ModelId=3F0189F00354
    itkNewMacro(Self);

    //##ModelId=3F0189F00373
    virtual void Paint(mitk::BaseRenderer * renderer);


    const mitk::Contour * GetInput(void);

    //##ModelId=3F0189F00376
    virtual void Update();

protected:
    //##ModelId=3F0189F00378
    ContourMapper2D();

    //##ModelId=3F0189F00382
    virtual ~ContourMapper2D();

    //##ModelId=3F0189F00384
    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKContourMapper2D_H_HEADER_INCLUDED */
