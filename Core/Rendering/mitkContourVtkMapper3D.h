#ifndef MITK_CONTOUR_VTK_MAPPER_3D_H
#define MITK_CONTOUR_VTK_MAPPER_3D_H

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkContour.h"
#include "mitkBaseRenderer.h"

#include <vtkPolyData.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk {

//##Documentation
//## @brief Vtk-based mapper for mitk::Contour
//## @ingroup Mapper
class ContourVtkMapper3D : public BaseVtkMapper3D
{
  public:

    mitkClassMacro(ContourVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);


    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    virtual const mitk::Contour* GetInput();


    //##Documentation
    //## @brief Called by BaseRenderer when an update is required
    virtual void Update(mitk::BaseRenderer* renderer);

    virtual void Update();

    virtual vtkProp* GetProp();


  protected:
   
    ContourVtkMapper3D();
   
    virtual ~ContourVtkMapper3D();

    vtkPolyDataMapper* m_PointVtkPolyDataMapper;
    vtkPolyDataMapper* m_VtkPolyDataMapper;
    vtkTubeFilter*     m_TubeFilter;

		vtkPolyData *m_Contour;
    vtkActor *m_Actor;
  
};

} // namespace mitk

#endif // MITK_CONTOUR_VTK_MAPPER_3D_H
