#ifndef _MITK_SPLINE_VTK_MAPPER_3D__H
#define _MITK_SPLINE_VTK_MAPPER_3D__H

#include "mitkPointSetVtkMapper3D.h"

class vtkActor;
class vtkAssembly;

namespace mitk
{

class SplineVtkMapper3D : public PointSetVtkMapper3D
{
public:

    mitkClassMacro( SplineVtkMapper3D, PointSetVtkMapper3D );

    itkNewMacro( Self );

    virtual vtkProp* GetProp();

    virtual void Update( mitk::BaseRenderer* renderer );

protected:

    SplineVtkMapper3D();

    virtual ~SplineVtkMapper3D();

    virtual void GenerateData();

    virtual void GenerateOutputInformation();
    
    vtkActor* m_SplinesActor;
    
    vtkAssembly* m_Assembly;
    
    bool m_SplinesAvailable;
    
    bool m_SplinesAddedToAssembly;
    
};


} //namespace mitk


#endif

