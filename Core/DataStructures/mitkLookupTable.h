#ifndef MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D
#define MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D

#include "mitkCommon.h"
#include "vtkLookupTable.h"
//#include "itk.h"

namespace mitk {

//## 
//##Documentation
//## @brief 
//## @ingroup Data
//## 
//## 
//## 
class LookupTable //: public itk::Object //: public BaseData
{
protected:
public:
//	 	mitkClassMacro(LookupTable, itk::Object);
//	 	mitkClassMacro(LookupTable, LookupTable);
	  
	  /** @brief Some convenient typedefs. */
    typedef unsigned char RawLookupTableType;

		vtkLookupTable* GetVtkLookupTable() const;

		RawLookupTableType * GetRawLookupTable() const;

		
		void SetVtkLookupTable(vtkLookupTable* vtkLookupTable){m_LookupTable = vtkLookupTable;};

		void ChangeOpacityForAll(float opacity);
		
    LookupTable();

    virtual ~LookupTable();


    /*!
    * \brief equality operator implementation
    */
    virtual bool operator==(const mitk::LookupTable& LookupTable) const;

    /*!
    * \brief non equality operator implementation
    */
    //##ModelId=3EA969CD007C
    virtual bool operator!=(const LookupTable& LookupTable) const;

    /*!
    * \brief implementation necessary because operator made
    *	private in itk::Object
    */
    virtual LookupTable& operator=(const LookupTable& LookupTable);

        
						
protected:


    vtkLookupTable* m_LookupTable;
//    vtkLookupTable* m_LookupTableforRawMode;
    
private:

};

} // namespace mitk



#endif /* LookupTable_H_HEADER_INCLUDED_C1EBD53D */
