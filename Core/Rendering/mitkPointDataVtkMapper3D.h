#ifndef _MITK_POINT_DATA_VTK_MAPPER_3D__H_
#define _MITK_POINT_DATA_VTK_MAPPER_3D__H_

#include <mitkBaseVtkMapper3D.h>
#include "mitkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"


class vtkActor;
class vtkPolyDataMapper;
class vtkSphereSource;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;

namespace mitk
{

class PointDataVtkMapper3D : public BaseVtkMapper3D
{
public:

    mitkClassMacro( PointDataVtkMapper3D, BaseVtkMapper3D );

    itkNewMacro( Self );

    itkSetMacro( Radius, float);

    itkGetMacro( Radius, float);

protected:

    PointDataVtkMapper3D();

    virtual ~PointDataVtkMapper3D();

    virtual void GenerateData();

    vtkActor* m_PointActor;

    vtkPolyDataMapper* m_PointMapper;

    vtkPolyData* m_PolyData;
    vtkPoints* m_Points;
    vtkCellArray* m_CellArray;

    float m_Radius;
};


} //namespace mitk


#endif

