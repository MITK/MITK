#ifndef MITK_CONTOUR_MAPPER_2D_H_
#define MITK_CONTOUR_MAPPER_2D_H_

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkContour.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

//##
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
    
    mitkClassMacro(ContourMapper2D, Mapper2D);

    itkNewMacro(Self);

    virtual void Paint(mitk::BaseRenderer * renderer);

    const mitk::Contour * GetInput(void);

    virtual void Update();

protected:
    ContourMapper2D();

    virtual ~ContourMapper2D();

    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKContourMapper2D_H_HEADER_INCLUDED */
