#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkCommon.h"
#include "Geometry2D.h"
#include <itkImageRegion.h>
#include "SpaceGeometry.h"

namespace mitk {

#if _MSC_VER < 1300
#define GImageDimension 4
#endif

//##ModelId=3DCBF389032B
class Geometry3D : public itk::LightObject
{
public:
    /** Standard class typedefs. */
    //##ModelId=3E1018B502F0
    typedef Geometry3D         Self;
    //##ModelId=3E1018B5032C
    typedef itk::LightObject   Superclass;
    //##ModelId=3E1018B50368
    typedef itk::SmartPointer<Self> Pointer;

    //##ModelId=3E1018B5039A
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    //	itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(Geometry3D, itk::LightObject);

#if !(_MSC_VER < 1300)
    //##ModelId=3E1552A603AC
    static const int GImageDimension=4;
#endif

    //##ModelId=3E14493003CF
    typedef itk::ImageRegion<GImageDimension> RegionType;

    /** Index typedef support. An index is used to access pixel values. */
    //##ModelId=3E15551A0246
    typedef itk::Index<GImageDimension>  IndexType;
    //##ModelId=3E15551A0278
    typedef IndexType::IndexValueType  IndexValueType;

    /** Offset typedef support. An offset represent relative position
    * between indices. */
    //##ModelId=3E15551A02AA
    typedef itk::Offset<GImageDimension>  OffsetType;
    //##ModelId=3E15551A02DC
    typedef OffsetType::OffsetValueType OffsetValueType;

    /** Size typedef support. A size is used to define region bounds. */
    //##ModelId=3E15551A030E
    typedef itk::Size<GImageDimension>  SizeType;
    //##ModelId=3E15551A0340
    typedef SizeType::SizeValueType SizeValueType;

    //##ModelId=3E15F8E800A4
    typedef itk::Transform<float,GImageDimension-1,GImageDimension-1>::Pointer TransformPointer;

    //##ModelId=3DCBF3AD0110
    const unsigned int *GetDimensions() const;

    //##ModelId=3DDE649E00A3
    unsigned int GetDataDimension() const;

    //##ModelId=3DCBF50C0377
    mitk::Geometry2D::Pointer GetGeometry2D(int s, int t) const;

    //##ModelId=3DCBF5D40253
    void GetBoundingBox() const;

    //##ModelId=3DCBF5E9037F
    double GetTime(int t) const;

    //##ModelId=3DE763C500C4
    virtual TransformPointer GetTransfrom() const;

    //##ModelId=3DDE65D1028A
    void PointMMToUnits(const Point3f &pt_mm, Point3f &pt_units, float t = 0) const;

    //##ModelId=3DDE65DC0151
    void PointUnitsToMM(const Point3f &pt_units, Point3f &pt_mm, float t = 0) const;

    //##ModelId=3DCBC65C017C
    const double *GetSpacing() const;

    //##ModelId=3E144966003D
    const RegionType& GetLargestPossibleRegion() const 
    {
        return m_LargestPossibleRegion;
    }
    //##ModelId=3E15572E0269
    Geometry3D(unsigned int dimension, const unsigned int* dimensions);

    //##ModelId=3E15578402BD
    virtual bool SetGeometry2D(mitk::Geometry2D* geometry2D, int s, int t = 0);

    //##ModelId=3E155839024F
    //##Documentation
    //## set according to tags in @a pic. @a pic can be 2D, 3D or 4D.  For 3D,
    //## the parameter @a s is ignored, for 4D @a s and @a t are ignored.
    //## @return false geometry not changed, either because of inconsistent
    //## data (e.g., dimensions do not match the dimensions of a slice) or
    //## read-only geometry.
    //## @return true geometry successfully updated.
    virtual bool SetGeometry2D(ipPicDescriptor* pic, int s = 0, int t = 0);
    //##ModelId=3E3452F10253
    Geometry3D(const mitk::Geometry3D& right);

    //##ModelId=3E3452F10393
    //    mitk::Geometry3D& operator=(const mitk::Geometry3D& right);
    //##ModelId=3E3456C50067
    virtual ~Geometry3D();


protected:
    //##ModelId=3E3453C703AF
    virtual void Initialize(unsigned int dimension, const unsigned int* dimensions);

    //##ModelId=3E14493100B0
    RegionType m_LargestPossibleRegion;

    //##ModelId=3E1896DB01DA
    unsigned int m_Dimension;
    //##ModelId=3E1896DB0249
    unsigned int *m_Dimensions;

    //##ModelId=3E3968CC000E
    std::vector<Geometry2D::Pointer> m_Geometry2Ds;
    //##ModelId=3E396B86022E
    Vector3f m_Resolution;

};

} // namespace mitk



#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
