#include "mitkLookupTable.h"


mitk::LookupTable::LookupTable() 
{
	m_LookupTable = vtkLookupTable::New();
//	m_LookupTableforRawMode = vtkLookupTable::New();
	
}


mitk::LookupTable::~LookupTable()
{
}


void mitk::LookupTable::ChangeOpacityForAll(float opacity) 
{

	int noValues = 	m_LookupTable->GetNumberOfTableValues ();
	
	float rgba[4];
	for (int i=0;i<noValues;i++)
	{
			 m_LookupTable->GetTableValue (i, rgba);
			 rgba[3] = opacity;
			 m_LookupTable->SetTableValue (i, rgba);
	}
//	this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged
//		    							// so no widget-updat is executed
}



vtkLookupTable* mitk::LookupTable::GetVtkLookupTable() const
{
	return m_LookupTable;
};

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{

//	int noValues =m_LookupTable->GetNumberOfTableValues ();
//////	m_LookupTableforRawMode->SetNumberOfTableValues(noValues);
////
//	float rgba[4];
//////
//////
//	unsigned char *raw = new unsigned char[4*noValues];
//////
//	for (int i=0 ; i<noValues ; i++)
//	{
//		m_LookupTable->GetTableValue(i,rgba);
//////
////////		rgbaRaw[0] = rgba[0] / 255.0f;
////////		rgbaRaw[1] = rgba[1] / 255.0f;
////////		rgbaRaw[2] = rgba[2] / 255.0f;
//		raw[0 + 4*i] = (unsigned char) (rgba[0]*255);
//		raw[1 + 4*i] = (unsigned char) (rgba[1]*255);
//		raw[2 + 4*i] = (unsigned char) (rgba[2]*255);
//		raw[3 + 4*i] = (unsigned char) (rgba[3]*255);;
////////		m_LookupTableforRawMode->SetTableValue(i,rgba);
//////
//	}

//	return m_LookupTableforRawMode->GetPointer(0);
//		return raw;
	return m_LookupTable->GetPointer(0);
		
};


/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator==(const mitk::LookupTable& LookupTable) const
{

		if ( m_LookupTable == (LookupTable.GetVtkLookupTable()) ) {
				 return true;
			 }
		else {
			return false;
		}
}

/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator!=(const mitk::LookupTable& LookupTable) const
{

		if ( m_LookupTable == (LookupTable.GetVtkLookupTable()) ) {
				 return false;
			 }
		else {
				return true;
		}

}

/*!
 * \brief non equality operator inplementation
 */
mitk::LookupTable& mitk::LookupTable::operator=(const mitk::LookupTable& LookupTable)
{
	if (this == &LookupTable) {
		return *this;
	}
	else {
		m_LookupTable = LookupTable.GetVtkLookupTable();
		return *this;
	}
}
