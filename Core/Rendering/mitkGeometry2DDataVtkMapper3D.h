#ifndef MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F
#define MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F

#include "mitkCommon.h"
#include "BaseVtkMapper3D.h"
#include "mitkGeometry2DData.h"
#include "DataTree.h"
#include "ImageSliceSelector.h"

class vtkActor;
class vtkTexture;
class vtkPlaneSource;
class vtkImageMapToWindowLevelColors;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkLookupTable;

namespace mitk {

//##ModelId=3E691E09014F
//##Documentation
//## @brief Vtk-based mapper to display a Geometry2D in a 3D window
//## @ingroup Mapper
class Geometry2DDataVtkMapper3D : public BaseVtkMapper3D
{
  public:
    //##ModelId=3E691E09036C
    mitkClassMacro(Geometry2DDataVtkMapper3D, BaseVtkMapper3D);

    itkNewMacro(Geometry2DDataVtkMapper3D);

    //##ModelId=3E691E090380
    virtual const mitk::Geometry2DData *GetInput();
  
  
    //##ModelId=3E691E09038A
    virtual void Update();
  
    //##ModelId=3E691E09038C
    virtual void GenerateData();
    //##ModelId=3E6E874F0007
    virtual void SetDataIteratorForTexture(mitk::DataTreeIterator* iterator);

  protected:
    //##ModelId=3E691E09038E
    Geometry2DDataVtkMapper3D();

    //##ModelId=3E691E090394
    virtual ~Geometry2DDataVtkMapper3D();
    //##ModelId=3E691E090396
    virtual void GenerateOutputInformation();


    //##ModelId=3E691E090331
	vtkPolyDataMapper* m_VtkPolyDataMapper;

    //##ModelId=3E691E090344
    vtkActor* m_Actor;

    //##ModelId=3E691E09034E
    vtkTexture* m_VtkTexture;

    //##ModelId=3E691E090358
    vtkPolyData* m_VtkPolyData;

    //##ModelId=3E691E090362
    vtkPlaneSource* m_VtkPlaneSource;

    //##ModelId=3E6E8AA4034C
    vtkLookupTable *m_VtkLookupTable;

    //##ModelId=3E691E090363
    vtkImageMapToWindowLevelColors* m_LevelWindowFilter;
    
    //##ModelId=3E6E8AA40375
    mitk::DataTreeIterator* m_DataTreeIterator;

    //##ModelId=3E6E912903B1
    mitk::ImageSliceSelector::Pointer m_SliceSelector;

	unsigned long int m_LastTextureUpdateTime;
};

} // namespace mitk



#endif /* MITKGEOMETRY2DDATAVTKMAPPER3D_H_HEADER_INCLUDED_C196C71F */
