#ifndef MITKUSLookupTableSOURCE_H_HEADER_INCLUDED
#define MITKUSLookupTableSOURCE_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkLookupTableSource.h"


class mitk::LookupTable;

namespace mitk
{


class USLookupTableSource : public LookupTableSource
{
public:
    typedef enum {HP, Strain, DefaultLUT} LookupTableMode;

    mitkClassMacro( USLookupTableSource, LookupTableSource );
    itkNewMacro( Self );
    
    /** @brief Some convenient typedefs. */
    typedef mitk::LookupTable OutputType;
    typedef OutputType * OutputTypePointer;

    void SetUseHPDopplerLookupTable()
    {
        m_Mode = HP;
    };

    void SetUseStrainLookupTable()
    {
        m_Mode = Strain;
    };

    virtual OutputTypePointer GetOutput( void );

protected:
    USLookupTableSource();
    virtual ~USLookupTableSource();

private:
    LookupTableMode m_Mode;
    OutputTypePointer m_LookupTable;

};

} // namespace mitk



#endif /* MITKLookupTableSOURCE_H_HEADER_INCLUDED */
