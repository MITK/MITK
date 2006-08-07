#include "mitkBaseData.h"

#include <itkDataObject.h>

namespace mitk {

/**
 * \brief Small class to use itk::DataObjects in the mitk::DataTree.
 **/
class ItkBaseDataAdapter : public BaseData
{
public:
  mitkClassMacro(ItkBaseDataAdapter, BaseData);
  itkNewMacro(Self);

  typedef itk::DataObject DataType;

  itkGetObjectMacro(ItkDataObject, DataType);
  itkSetObjectMacro(ItkDataObject, DataType);

  virtual void SetRequestedRegionToLargestPossibleRegion() {}
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() { return false; }
  virtual bool VerifyRequestedRegion() { return true; }
  virtual void SetRequestedRegion(itk::DataObject*) {}

protected:
  DataType::Pointer m_ItkDataObject;

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;
};

}
