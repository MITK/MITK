#ifndef _MITK_BASE_DATA_SOURCE_H
#define _MITK_BASE_DATA_SOURCE_H

#include "mitkBaseProcess.h"
#include "mitkBaseData.h"

namespace mitk
{

class BaseDataSource : public BaseProcess
{
public:
    mitkClassMacro( BaseDataSource, BaseProcess );

    itkNewMacro( Self );

    typedef mitk::BaseData OutputType;

    typedef OutputType::Pointer OutputTypePointer;
    
    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx );

    /**
     * Allows to set the output of the base data source. 
     * @param output the intended output of the base data source
     */
    virtual void SetOutput( OutputType* output );
    
    /**
     * Allows to set the n-th output of the base data source. 
     * @param output the intended output of the base data source
     */
    virtual void SetOutput( unsigned int idx, OutputType* output );
    
    /**
     * Returns the output with index 0 of the base data source
     * @returns the output
     */
    virtual OutputType* GetOutput();
    
    /**
     * Returns the n'th output of the base data source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    virtual OutputType* GetOutput ( unsigned int idx );
    
protected:

    BaseDataSource();
    
    virtual ~BaseDataSource();

};

}
#endif // #define _MITK_BASE_DATA_SOURCE_H
