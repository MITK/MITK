#ifndef BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC
#define BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC

#include "mitkCommon.h"

namespace mitk {

class BaseData;

//##ModelId=3DCBF9BC011A
//##Documentation
//## Superclass of all classes generating some kind of mitk::BaseData. 
//## 
//## In itk and vtk the generated result of a ProcessObject is only guaranteed
//## to be up-to-date, when Update() of the ProcessObject or the generated
//## DataObject is called immediately before access of the data stored in the
//## DataObject. This is also true for subclasses of mitk::BaseProcess. But
//## many of the subclasses of mitk::BaseProcess define additional access
//## functions to the generated output that guarantee an up-to-date result, see
//## for example mitk::ImageSource. 
class BaseProcess : public itk::ProcessObject
{
public:
    mitkClassMacro(BaseProcess, itk::ProcessObject);

    //##ModelId=3E8600DC03E2
    virtual void UnRegister() const;

    //##ModelId=3E8600DD000E
    virtual int GetExternalReferenceCount() const;
protected:
    //##ModelId=3E8600DD0036
    BaseProcess();
    //##ModelId=3E8600DD004A
    virtual ~BaseProcess();

    /** Protected methods for setting outputs.
    * Subclasses make use of them for getting output. 
    * These are only overwritten because of itk::DataObject::connectSource being 
    * private and non-virtual:
    * the important stuff is done in mitk::BaseData::connectSource.*/
    //##ModelId=3E8600DD0072
    virtual void SetNthOutput(unsigned int num, itk::DataObject *output);
    //##ModelId=3E8600DD00F4
    virtual void AddOutput(itk::DataObject *output);
private:
    //##ModelId=3E8600DC0356
    mutable bool m_Unregistering;
    //##ModelId=3E8600DC037E
    mutable bool m_CalculatingExternalReferenceCount;
    //##ModelId=3E8600DC03B0
    mutable int m_ExternalReferenceCount;
};

} // namespace mitk



#endif /* BASEPROCESS_H_HEADER_INCLUDED_C19BE6FC */
