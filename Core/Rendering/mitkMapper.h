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

  /** Standard class typedefs. */

  typedef Mapper         Self;
  typedef ImageSource         Superclass;

  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;
  
  /** Method for creation through the object factory. */
  //   itkNewMacro(Self);  
 
  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageSource,BaseProcess);

  public:
    //##ModelId=3E0B115800B3
    virtual void Paint(mitk::Geometry3D &geometry, int s, int t, mitk::LevelWindow &lw);
    virtual void SetInput(BaseData* data) = 0; 
};


} // namespace mitk



#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
