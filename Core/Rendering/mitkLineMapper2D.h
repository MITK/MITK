#ifndef MITKLINEMAPPER2D_H_HEADER_INCLUDED
#define MITKLINEMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkPointSet.h"
#include "mitkPointSetMapper2D.h"
#include "mitkDataTree.h"

namespace mitk {

class BaseRenderer;

//##Documentation
//## @brief OpenGL-based mapper to display Lines
//## @ingroup Mapper
//## uses the information from the PointSet to build up the lines. 
//## Only lines in 2D in one Slice are drawn, not between slices!
class LineMapper2D : public PointSetMapper2D
{
public:
  mitkClassMacro(LineMapper2D, PointSetMapper2D);
  itkNewMacro(Self);

  //##Documentation
  //## @brief Get the PointDataList to map
  const mitk::PointSet * GetInput(void);

  virtual void Paint(mitk::BaseRenderer * renderer);

  virtual void Update();

protected:
  LineMapper2D();

  virtual ~LineMapper2D();

  virtual void GenerateOutputInformation();
};

} // namespace mitk

#endif /* MITKLINEMAPPER2D_H_HEADER_INCLUDED */
