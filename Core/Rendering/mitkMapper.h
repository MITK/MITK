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
    //##ModelId=3E0B115800B3
    virtual void paint(mitk::Geometry3D geometry, int s, int t, mitk::LevelWindow lw);

};

} // namespace mitk



#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
