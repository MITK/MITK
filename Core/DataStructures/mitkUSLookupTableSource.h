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
    typedef enum {DSRDoppler, StrainRate, DefaultLUT} LookupTableMode;

    mitkClassMacro( USLookupTableSource, LookupTableSource );
    itkNewMacro( Self );
    
    /** @brief Some convenient typedefs. */
    typedef mitk::LookupTable OutputType;

    void SetUseDSRDopplerLookupTable() {m_Mode = DSRDoppler; this->Modified();};

    void SetUseStrainRateLookupTable() {m_Mode = StrainRate; this->Modified();};

protected:
    USLookupTableSource();
    virtual ~USLookupTableSource();

     /**
     * Generates a LookupTable depended on the given mode and stores
     * it in an output object
     */
    virtual void GenerateData();

    /**
     * Generates a lookup table, dependend on the given mode
     * @returns a vtkLookupTable
     */
    vtkLookupTable* BuildVtkLookupTable( );

    /**
     * Generates a Philips Doppler lookup table
     * if an a.map file is given, then a original LookupTable is genereated
     * else an LookupTable similar to the original one is generated
     * @returns a vtkLookupTable
     */
    vtkLookupTable* BuildDSRDopplerLookupTable();

    /**
     * Generates a Strain Rate lookup table than those that are
     * used e.g. by GE
     * 
     * @returns a vtkLookupTable
     */
		vtkLookupTable* BuildStrainRateLookupTable();

    /**
     * Generates a default lookup table
     * @returns a vtkLookupTable
     */
		vtkLookupTable* BuildDefaultLookupTable();
    
    
	
private:
    LookupTableMode m_Mode;
    OutputTypePointer m_LookupTable;

};

} // namespace mitk



#endif /* MITKLookupTableSOURCE_H_HEADER_INCLUDED */
