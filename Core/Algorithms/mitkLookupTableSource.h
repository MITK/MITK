#ifndef MITKLookupTableSOURCE_H_HEADER_INCLUDED
#define MITKLookupTableSOURCE_H_HEADER_INCLUDED

#include "mitkCommon.h"

#include "mitkLookupTable.h"


class mitk::LookupTable;

namespace mitk {


class LookupTableSource : public itk::Object //: public BaseProcess
{
public:
	typedef enum {HP, Strain, DefaultLUT} LookupTableMode;

  mitkClassMacro(LookupTableSource,itk::Object);

	  /** @brief Some convenient typedefs. */
    typedef mitk::LookupTable OutputType;
    typedef OutputType * OutputTypePointer;

    void SetUseHPDopplerLookupTable(){m_Mode = HP;};

    void SetUseStrainLookupTable(){m_Mode = Strain;};

		virtual OutputTypePointer GetOutput(void);
		LookupTableSource();

		virtual ~LookupTableSource();


protected:

private:

	LookupTableMode m_Mode;
	OutputTypePointer m_LookupTable;

};

} // namespace mitk



#endif /* MITKLookupTableSOURCE_H_HEADER_INCLUDED */
