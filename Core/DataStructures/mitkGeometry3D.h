#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkCommon.h"
#include "mitkVector.h"
#include <itkImageRegion.h>
#include <itkBoundingBox.h>

class vtkTransform;

namespace mitk {

#if _MSC_VER < 1300
#define GImageDimension 4
#endif

//##ModelId=3E8600D800C5
//##Documentation
//## @brief Standard 3D-BoundingBox typedef (float)
//## Standard 3D-BoundingBox typedef to get rid of template arguments (3D,
//## float).
typedef itk::BoundingBox<unsigned long, 3, mitk::ScalarType>   BoundingBox;

//##ModelId=3DCBF389032B
//##Documentation
//## @brief Describes the geometry of a data object
//## @ingroup Geometry
//## Describes the geometry of a data object. At least, it can
//## return the bounding box of the data object. An important sub-class is
//## SlicedGeometry3D, which descibes data objects consisting of
//## slices, e.g., objects of type Image.
//## Conversions between world coordinates (in mm) and unit coordinates 
//## (e.g., pixels in the case of an Image) can be performed.
//## 
//## Geometry3D have to be initialized in the method GenerateOutputInformation() 
//## of BaseProcess (or CopyInformation/ UpdateOutputInformation of BaseData, 
//## if possible, e.g., by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//## 
//## Rule: everything is in mm (ms) if not stated otherwise.
class Geometry3D : public itk::Object
{
public:
  mitkClassMacro(Geometry3D, itk::Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

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

  //##ModelId=3DCBF3AD0110
  const unsigned int *GetDimensions() const;

  //##ModelId=3DDE649E00A3
  unsigned int GetDataDimension() const;

  //##ModelId=3DCBF5D40253
  virtual mitk::BoundingBox::ConstPointer GetBoundingBox(int t = 0) const;

  //##ModelId=3DCBF5E9037F
  double GetTime(int t) const;

  //##ModelId=3DDE65D1028A
  void MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units, float t = 0) const;

  //##ModelId=3DDE65DC0151
  void UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm, float t = 0) const;

  //##ModelId=3E144966003D
  const RegionType& GetLargestPossibleRegion() const 
  {
    return m_LargestPossibleRegion;
  }

  //##ModelId=3E3B986602CF
  void MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units, float t = 0) const;
  //##ModelId=3E3B987503A3
  void UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm, float t = 0) const;

  //##ModelId=3E3BE1F8000C
  virtual bool IsValidTime(int t = 0) const;

  //##ModelId=3ED91D050299
  virtual void SetBoundingBox(const mitk::BoundingBox* boundingBox,  int t=0);

  //##ModelId=3ED91D050305
  virtual void SetBoundingBox(const float bounds[6],  int t=0);
  
  vtkTransform* GetTransform();  
  
  //##ModelId=3E3453C703AF
  virtual void Initialize(unsigned int dimension, const unsigned int* dimensions);

  void SetBaseGeometry(mitk::Geometry3D* base);  
  
  //##Documentation
  //## @brief Transform the geometry with the transformation description
  //## provided as @a transform (type vtkTransform).
  //## @todo not yet implemented
  virtual void SetMasterTransform(const vtkTransform * transform);

  virtual Pointer Clone();

protected:
  Geometry3D();

  //##ModelId=3E3456C50067
  virtual ~Geometry3D();

  //##ModelId=3E14493100B0
  RegionType m_LargestPossibleRegion;

  //##ModelId=3E1896DB01DA
  unsigned int m_Dimension;
  //##ModelId=3E1896DB0249
  unsigned int *m_Dimensions;

  //##ModelId=3ED91D050269
  mutable std::vector<mitk::BoundingBox::ConstPointer> m_BoundingBoxes;

  //##ModelId=3E3BE8BF02BA
  mitk::Matrix4D m_TransformUnitsToMM;
  //##ModelId=3E3BEC5D0257
  mitk::Matrix4D m_TransformMMToUnits;
  //##ModelId=3E3BE8BF02EC
  mitk::Matrix4D m_TransformOfOrigin;

  //New:
  vtkTransform* m_Transform;
  //vtkTransform* m_RelativeTransform;
  Geometry3D::Pointer m_BaseGeometry;  
  //Geometry3D::ConstPointer m_BaseGeometry;  
  //unsigned long m_TransformTimeStamp;
  //unsigned long m_BaseTransformTimeStamp;

};

} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
