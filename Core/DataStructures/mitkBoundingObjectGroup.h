#ifndef BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED
#define BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED

#include <mitkBoundingObject.h>

namespace mitk {

//##Documentation
//## @brief group object, that contains several mitk::BoundingObjects
//## calculates a bounding box that contains all sub-bounding boxes
//## @ingroup Data  
class BoundingObjectGroup : public mitk::BoundingObject
{
public:
  typedef itk::VectorContainer<unsigned int ,mitk::BoundingObject::Pointer> BoundingObjectContainer;

  enum CSGMode
   {
      Union,
      Intersection,
      Difference
   };


  mitkClassMacro(BoundingObjectGroup, mitk::BoundingObject);// itk::VectorContainer<unsigned int ,mitk::BoundingObject::Pointer>);  
  itkNewMacro(Self);

  virtual void UpdateOutputInformation();
  void SetRequestedRegionToLargestPossibleRegion();
  bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  void SetRequestedRegion(itk::DataObject *data);
  virtual bool IsInside(mitk::ITKPoint3D p);

  itkSetObjectMacro(BoundingObjects, mitk::BoundingObjectGroup::BoundingObjectContainer);
  itkGetObjectMacro(BoundingObjects, mitk::BoundingObjectGroup::BoundingObjectContainer);

  itkSetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);
  itkGetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);

  void AddBoundingObject(mitk::BoundingObject::Pointer boundingObject);
  unsigned int GetCount() const;

protected:
  BoundingObjectGroup();
  virtual ~BoundingObjectGroup();

  mitk::BoundingObjectGroup::BoundingObjectContainer::Pointer m_BoundingObjects;
  unsigned int m_Counter;
  CSGMode m_CSGMode;
};

}
#endif /* BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED */
