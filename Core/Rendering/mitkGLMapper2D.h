#ifndef MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872
#define MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872

#include "mitkCommon.h"
#include "Mapper2D.h"
#include "BaseRenderer.h"

namespace mitk {

//##ModelId=3E6814230160
//##Documentation
//## @brief Base class of all OpenGL-based 2D-Mappers
//## Base class of all OpenGL-based 2D-Mappers. Those must implement the
//## abstract method Paint(BaseRenderer).
class GLMapper2D : public Mapper2D
{
  public:


    //##ModelId=3E67E9390346
    virtual void Paint(mitk::BaseRenderer *renderer) = 0;
  protected:
    //##ModelId=3E681470037E
    GLMapper2D();
    //##ModelId=3E681470039C
    virtual ~GLMapper2D();

};

} // namespace mitk



#endif /* MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872 */
