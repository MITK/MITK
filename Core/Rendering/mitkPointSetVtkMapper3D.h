#ifndef MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273
#define MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273

#include "mitkCommon.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkPointSet.h"
#include "mitkBaseRenderer.h"

#include <vtkSphereSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkTubeFilter.h>
#include <vtkVectorText.h>
#include <vtkTextSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


class vtkActor;
class vtkAssembly;
class vtkFollower;
class vtkPolyDataMapper;

namespace mitk {

//##ModelId=3E70F60202EA
//##Documentation
//## @brief Vtk-based mapper for PointList
//## @ingroup Mapper
class PointSetVtkMapper3D : public BaseVtkMapper3D
{
  public:

    mitkClassMacro(PointSetVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Self);

    //##ModelId=3E70F60301D5
    virtual const mitk::PointSet* GetInput();

    //##ModelId=3E70F60301E9
    virtual void GenerateData();

    //##ModelId=3EF19FA803BF
    //##Documentation
    //## @brief Called by BaseRenderer when an update is required
    virtual void Update(mitk::BaseRenderer* renderer);
    //##ModelId=3EF1B44001D5
    virtual void Update();



  protected:
    //##ModelId=3E70F60301F2
    virtual void GenerateOutputInformation();
    //##ModelId=3E70F60301F4
    PointSetVtkMapper3D();

    //##ModelId=3E70F60301F5
    virtual ~PointSetVtkMapper3D();

    //##ModelId=3E70F60301C9

     vtkActor *m_Actor;
 //   vtkAssembly *m_Actor;
//    vtkActor* m_PointActor;
//	  vtkFollower* m_TextActor;
// 	  vtkActor* m_TextActor;


    
    //##ModelId=3E70F60301CA
    vtkPolyDataMapper* m_PointVtkPolyDataMapper;
    vtkPolyDataMapper* m_TextVtkPolyDataMapper;
    vtkPolyDataMapper* m_VtkPolyDataMapper;    


		vtkAppendPolyData *m_vtkPointList;
		vtkAppendPolyData *m_vtkTextList;
		vtkPolyData *m_contour;
	   vtkTubeFilter * m_tubefilter;
};

} // namespace mitk

#endif /* MITKPointSetVtkMAPPER3D_H_HEADER_INCLUDED_C1907273 */
