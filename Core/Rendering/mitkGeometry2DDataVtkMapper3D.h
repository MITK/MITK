#ifndef MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F
#define MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F

#include "mitkCommon.h"
#include "BaseVtkMapper3D.h"
#include "mitkGeometry2DData.h"

class vtkActor;
class vtkTexture;
class vtkPlaneSource;
class vtkImageMapToWindowLevelColors;
class vtkPolyData;
class vtkPolyDataMapper;

namespace mitk {

//##ModelId=3E32C8E9030D
//##Documentation
//## Vtk-based mapper to display a Geometry2D in a 3D window.
class Geometry2DDataVtkMapper3D : public BaseVtkMapper3D
{
  public:
    mitkClassMacro(Geometry2DDataVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Geometry2DDataVtkMapper3D);

    //##ModelId=3E32C9E5007A
    virtual vtkProp* GetProp();
    //##ModelId=3E690E68028C
    virtual void SetInput(const mitk::Geometry2DData *input);

    //##ModelId=3E690E6A0163
    const mitk::Geometry2DData *GetInput(void);
  
    //##ModelId=3E690E6B0331
    virtual void SetInput(const mitk::BaseData* data);
  
    //##ModelId=3E690E6D03A2
    virtual void Update();
  
    //##ModelId=3E690E7D021E
    virtual void GenerateData();

  protected:
    //##ModelId=3E32CA6C01A1
    Geometry2DDataVtkMapper3D();

    //##ModelId=3E32CA6C01C9
    virtual ~Geometry2DDataVtkMapper3D();
    //##ModelId=3E690E6F00D4
    virtual void GenerateOutputInformation();


    //##ModelId=3E690C6303DD
	vtkPolyDataMapper* m_VtkPolyDataMapper;

    //##ModelId=3E690C6400A0
    vtkActor* m_Actor;

    //##ModelId=3E690C640140
    vtkTexture* m_VtkTexture;

    //##ModelId=3E690E3B03B4
    vtkPolyData* m_VtkPolyData;

    //##ModelId=3E690C640190
    vtkPlaneSource* m_VtkPlaneSource;

    //##ModelId=3E690C6401E0
    vtkImageMapToWindowLevelColors* m_LevelWindowFilter;
};

} // namespace mitk



#endif /* MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F */
