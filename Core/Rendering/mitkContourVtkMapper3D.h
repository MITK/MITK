#ifndef MITK_CONTOUR_VTK_MAPPER_3D_H
#define MITK_CONTOUR_VTK_MAPPER_3D_H

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkContour.h"
#include "mitkBaseRenderer.h"


//#include <vtkSphereSource.h>
//#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>

class vtkPolyDataMapper;
class vtkAppendPolyData;
class vtkActor;
class vtkTubeFilter;

namespace mitk {

//##ModelId=3E70F60202EA
//##Documentation
//## @brief Vtk-based mapper for PointList
//## @ingroup Mapper
class ContourVtkMapper3D : public BaseVtkMapper3D
{
  public:

    mitkClassMacro(ContourVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);


    //##ModelId=3E70F60301E9
    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    //##ModelId=3E70F60301D5
    virtual const mitk::Contour* GetInput();


    //##ModelId=3EF19FA803BF
    //##Documentation
    //## @brief Called by BaseRenderer when an update is required
    virtual void Update(mitk::BaseRenderer* renderer);
    //##ModelId=3EF1B44001D5
    virtual void Update();

    virtual vtkProp* GetProp();


  protected:
   
   // virtual void GenerateOutputInformation();
   
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
