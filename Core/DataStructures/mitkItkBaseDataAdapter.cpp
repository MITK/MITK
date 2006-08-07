#include "mitkItkBaseDataAdapter.h"

namespace mitk {

void ItkBaseDataAdapter::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "ItkBaseDataAdapter:" << std::endl;
  if(m_ItkDataObject.IsNull())
  {
    os << indent << "No DataObject!" << std::endl;
  }
  else
  {
    m_ItkDataObject->Print(os, indent);
  }
}

}
