#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkCommon.h"
#include "Geometry2D.h"
#include <itkImageRegion.h>
#include <itkBoundingBox.h>
#include <ipPic.h>

namespace mitk {

#if _MSC_VER < 1300
#define GImageDimension 4
#endif

//##ModelId=3E8600D800C5
//##Documentation
//## @brief Standard 3D-BoundingBox typedef (float)
//## Standard 3D-BoundingBox typedef to get rid of template arguments (3D,
//## float).
typedef itk::BoundingBox<> BoundingBox;

//##ModelId=3DCBF389032B
//##Documentation
//## @brief Describes the geometry of a data object
//## @ingroup Geometry
//## Describes the geometry of a data object, e.g., an Image. At least, it can
//## return the bounding box of the data object. If the data object consists of
//## slices, a Geometry2D can be requested for each slice and conversions
//## between world coordinates (in mm) and unit coordinates (e.g., pixels in
//## the case of an Image) can be performed.
//## 
//## Geometry3D and the associated Geometry2Ds (if applicable) have to be
//## initialized in the method GenerateOutputInformation() of BaseProcess (or
//## CopyInformation/ UpdateOutputInformation of BaseData, if possible, e.g.,
//## by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//## 
//## @warning The Geometry2Ds are not necessarily up-to-date and not even
//## initialized. As described in the previous paragraph, one of the
//## Generate-/Copy-/UpdateOutputInformation methods have to initialize it.
//## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
//## up-to-date before returning it (by setting the update extent appropriately
//## and calling UpdateOutputInformation).
//## 
//## Rule: everything is in mm (ms) if not stated otherwise.
class Geometry3D : public itk::Object
{
public:
    /** Standard class typedefs. */
    //##ModelId=3E1018B502F0
    typedef Geometry3D         Self;
    //##ModelId=3E1018B5032C
    typedef itk::Object        Superclass;
    //##ModelId=3E1018B50368
    typedef itk::SmartPointer<Self> Pointer;
    //##ModelId=3E1018B5039A
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    //	itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(Geometry3D, itk::Object);

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
    //##Documentation
    //## Return the Geometry2D of the slice (@a s, @a t).
    //## If (a) we don't have a Geometry2D stored for the requested slice, 
    //## (b) m_EvenlySpaced is activated and (c) the first slice (s=0,t=0) 
    //## is a PlaneGeometry instance, then we calculate geometry of the
    //## requested
    //## as the plane of the first slice shifted by m_Spacing*s.
    //## 
    //## @warning The Geometry2Ds are not necessarily up-to-date and not even
    //## initialized. Geometry2Ds (if applicable) have to be initialized in the
    //## method GenerateOutputInformation() of BaseProcess (or CopyInformation/
    //## UpdateOutputInformation of BaseData, if possible, e.g., by analyzing
    //## pic tags in Image) subclasses. See also
    //## itk::ProcessObject::GenerateOutputInformation(),
    //## itk::DataObject::CopyInformation() and
    //## itk::DataObject::UpdateOutputInformation().
    //## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
    //## up-to-date before returning it (by setting the update extent
    //## appropriately and calling UpdateOutputInformation).
    mitk::Geometry2D::ConstPointer GetGeometry2D(int s, int t) const;

    //##ModelId=3DCBF5D40253
    virtual mitk::BoundingBox::ConstPointer GetBoundingBox(int t = 0) const;

    //##ModelId=3DCBF5E9037F
    double GetTime(int t) const;

    //##ModelId=3DE763C500C4
    virtual TransformPointer GetTransfrom() const;

    //##ModelId=3DDE65D1028A
    void MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units, float t = 0) const;

    //##ModelId=3DDE65DC0151
    void UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm, float t = 0) const;

    //##ModelId=3DCBC65C017C
    const float* GetSpacing() const;

    //##ModelId=3E144966003D
    const RegionType& GetLargestPossibleRegion() const 
    {
        return m_LargestPossibleRegion;
    }
    //##ModelId=3E15572E0269
    Geometry3D(unsigned int dimension, const unsigned int* dimensions);
    //##ModelId=3E3452F10253
    Geometry3D(const mitk::Geometry3D& right);

    //##ModelId=3E15578402BD
    virtual bool SetGeometry2D(const mitk::Geometry2D* geometry2D, int s, int t = 0);
    //##ModelId=3E155839024F
    //##Documentation
    //## set according to tags in @a pic. @a pic can be 2D, 3D or 4D.  For 3D,
    //## the parameter @a s is ignored, for 4D @a s and @a t are ignored.
    //## @return @a false: geometry not changed, either because of inconsistent
    //## data (e.g., dimensions do not match the dimensions of a slice) or
    //## read-only geometry.
    //## @return @a true: geometry successfully updated.
    virtual bool SetGeometry2D(ipPicDescriptor* pic, int s = 0, int t = 0);

    //##ModelId=3E3B986602CF
    void MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units, float t = 0) const;
    //##ModelId=3E3B987503A3
    void UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm, float t = 0) const;

    //##ModelId=3E3BE1F10106
    virtual bool IsValidSlice(int s = 0, int t = 0) const;
    //##ModelId=3E3BE1F8000C
    virtual bool IsValidTime(int t = 0) const;
    //##ModelId=3E3BE8CF010E
    virtual void SetSpacing(mitk::Vector3D aSpacing);
    //##ModelId=3E3C13F802A6
    virtual void SetEvenlySpaced(bool on = true);
    //##ModelId=3E3C2C37031B
    //##Documentation
    //## Set the spacing according to the tags in @ pic.
    virtual void SetSpacing(ipPicDescriptor* pic);

    //##ModelId=3ED91D050299
    virtual void SetBoundingBox(const mitk::BoundingBox* boundingBox,  int t=0);

    //##ModelId=3ED91D050305
	virtual void SetBoundingBox(const float bounds[6],  int t=0);
protected:
    //##ModelId=3E3453C703AF
    virtual void Initialize(unsigned int dimension, const unsigned int* dimensions);
    //##ModelId=3E3452F10393
    //    mitk::Geometry3D& operator=(const mitk::Geometry3D& right);
    //##ModelId=3E3456C50067
    virtual ~Geometry3D();

    //##ModelId=3E14493100B0
    RegionType m_LargestPossibleRegion;

    //##ModelId=3E1896DB01DA
    unsigned int m_Dimension;
    //##ModelId=3E1896DB0249
    unsigned int *m_Dimensions;

    //##ModelId=3E3968CC000E
    mutable std::vector<Geometry2D::ConstPointer> m_Geometry2Ds;
    //##ModelId=3E3BE8BF0288
    mitk::Vector3D m_Spacing;
	
    //##ModelId=3ED91D050269
	mutable std::vector<mitk::BoundingBox::ConstPointer> m_BoundingBoxes;

    //##ModelId=3E3BE8BF02BA
    mitk::Matrix4D m_TransformUnitsToMM;
    //##ModelId=3E3BEC5D0257
    mitk::Matrix4D m_TransformMMToUnits;
    //##ModelId=3E3BE8BF02EC
    mitk::Matrix4D m_TransformOfOrigin;
    //##ModelId=3E3C13B70180
    bool m_EvenlySpaced;
};

} // namespace mitk



#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
