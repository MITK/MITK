#ifndef BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A
#define BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A

#include "mitkCommon.h"
#include "Mapper.h"
#include "Mapper3D.h"

class vtkProp;
class vtkProp3D;

namespace mitk {

//##ModelId=3E32C62B00BE
//##Documentation
//## @brief Base class of all vtk-based 3D-Mappers
//## @ingroup Mapper
//## Those must implement the abstract
//## method vtkProp* GetProp().
class BaseVtkMapper3D : public Mapper3D
{
public:
    /** Standard class typedefs. */
    //##ModelId=3E32DC990067
    typedef BaseVtkMapper3D          Self;
    //##ModelId=3E32DC9900A3
    typedef Mapper                   Superclass;
    //##ModelId=3E32DC9900C2
    typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E32DC9900EA
    typedef itk::SmartPointer<const Self>  ConstPointer;

    /** Run-time type information (and related methods). */
    itkTypeMacro(Mapper,BaseVtkMapper3D);

    //##ModelId=3E32C93D0000
    virtual vtkProp* GetProp();
  protected:
    //##ModelId=3E3424950213
    BaseVtkMapper3D();
  
    //##ModelId=3E3424950231
    virtual ~BaseVtkMapper3D();

  public:

    itkGetObjectMacro(Geometry,Geometry3D);
    itkSetObjectMacro(Geometry,Geometry3D);

    itkGetConstMacro(SliceNr,int);
    itkSetMacro(SliceNr,int);

    itkGetConstMacro(TimeNr,int);
    itkSetMacro(TimeNr,int);

    itkGetConstMacro(ChannelNr,int);
    itkSetMacro(ChannelNr,int);

protected:
    //##ModelId=3E33ECF2030B
    Geometry3D::Pointer m_Geometry;
    //##ModelId=3ED91D050038
    LevelWindow m_LevelWindow;

    //##ModelId=3E3AE0DC023D
    int m_SliceNr;

    //##ModelId=3E33ECF203A1
    int m_TimeNr;

    //##ModelId=3E33ECF203B5
    int m_ChannelNr;

    //##ModelId=3ED91D050046
	vtkProp3D* m_Prop3D;
};

} // namespace mitk



#endif /* BASEVTKMAPPER3D_H_HEADER_INCLUDED_C1CD0F0A */
