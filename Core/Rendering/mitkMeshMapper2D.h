#ifndef MITKMESHMAPPER2D_H_HEADER_INCLUDED
#define MITKMESHMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkMesh.h"
#include "mitkGLMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

//##Documentation
//## @brief OpenGL-based mapper to display a mesh in a 2D window
//## @ingroup Mapper
//## @todo implement for VtkAbstractTransformGeometry.
class MeshMapper2D : public GLMapper2D
{
public:
    mitkClassMacro(MeshMapper2D, Mapper2D);

    itkNewMacro(Self);

    //##Documentation
    //## @brief Get the Mesh to map
    const mitk::Mesh * GetInput(void);

    virtual void Paint(mitk::BaseRenderer * renderer);

    virtual void Update();

protected:
    MeshMapper2D();

    virtual ~MeshMapper2D();

    virtual void GenerateOutputInformation();
};

} // namespace mitk



#endif /* MITKMESHMapper2D_H_HEADER_INCLUDED */
