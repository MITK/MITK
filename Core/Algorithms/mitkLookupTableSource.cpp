#include "mitkLookupTableSource.h"
#include <ipPic/ipPic.h>
#include <vtkLookupTable.h>

extern "C"
{
#include "uscfunctions/usc.h"
}


mitk::LookupTableSource::~LookupTableSource()
{}

mitk::LookupTableSource::LookupTableSource()
{
	m_Mode = HP;
	m_LookupTable = NULL;

}


/**
 *
 */
mitk::LookupTableSource::OutputTypePointer mitk::LookupTableSource::GetOutput()
{

	std::cout << "mitk::LookupTableSource::GetOutput() ... " << std::endl;

//	if (m_LookupTable !+= NULL) delete m_LookupTable;

	m_LookupTable = new mitk::LookupTable();

	if (m_Mode == HP)
	{
			std::cout << "  creating HP LookupTable ... " << std::endl;

			ipPicDescriptor *HPMap;
			HPMap=usReadMap("a.map", ".", -1000,-1000);

			vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
			ipUInt1_t *data = ((ipUInt1_t *)HPMap->data);

			int LookupTablesize =256;
			float rgba[4];
			vtkLookupTable->SetNumberOfColors(LookupTablesize);
			for (int i=0; i<LookupTablesize; i++)
			{
				rgba[0] = ((ipUInt1_t *) data)[0 + i*3*LookupTablesize] /255.0f;
				rgba[1] = ((ipUInt1_t *) data)[1 + i*3*LookupTablesize] /255.0f ;
				rgba[2] = ((ipUInt1_t *) data)[2 + i*3*LookupTablesize] /255.0f ;
				rgba[3] = 1;
				vtkLookupTable->SetTableValue (i, rgba);
			}

			m_LookupTable->SetVtkLookupTable(vtkLookupTable);

	} else if (m_Mode ==Strain)
	{
			std::cout << "  creating Strain  LookupTable ... " << std::endl;
			vtkLookupTable *vtkLookupTable = vtkLookupTable::New();

			int size=256;

//			xDim = size;
//			yDim = 3;
			float rgba[4];

			float quartal = size  / 8.0f;

			float sizeQuartal1 = 3 * quartal; //quartal-1;

			float sizeQuartal2 = 0.9375 * quartal;
			float sizeZeroStrain = 0.125* quartal;
			float sizeQuartal3 = 0.9375 * quartal;

//			float sizeQuartal2 = 0.875 * quartal;
//			float sizeZeroStrain = 0.25* quartal;
//			float sizeQuartal3 = 0.875 * quartal;

//			float sizeQuartal2 = 0.975 * quartal;
//			float sizeZeroStrain = 0.05* quartal;
//			float sizeQuartal3 = 0.975 * quartal;


			float sizeQuartal4 = 3 * quartal;

//			std::cout << "quartal = " << quartal << std::endl;
//			std::cout << "quartal 1 = " << sizeQuartal1 << std::endl;
//			std::cout << "quartal 2 = " << sizeQuartal2 << std::endl;
//			std::cout << "quartal 3 = " << sizeQuartal3 << std::endl;
//			std::cout << "quartal 4 = " << sizeQuartal4 << std::endl;
//			std::cout << "quartal zero = " << sizeZeroStrain << std::endl;


			int factor = 1;
      int i;

			// dunkelrot
			for(i=1; i<=sizeQuartal1; i++)
			{
				int index = i-1;
    	  rgba[0] = factor * (0.5 + i/(2*sizeQuartal1));      // rot
				rgba[1] = factor * 0;                           // gruen
				rgba[2] = factor * 0;                           // blau
				rgba[3] = factor * 1;
				vtkLookupTable->SetTableValue (index, rgba);
			}

			// hellrot bis gelb
			for (i=1 ; i<=sizeQuartal2 ; i++)
			{
		   	int index = (i-1) + sizeQuartal1;
      	rgba[0] = factor * 1;
      	rgba[1] = factor * (i/sizeQuartal2);
      	rgba[2] = factor * 0;
				rgba[3] = factor * 1;
				vtkLookupTable->SetTableValue (index, rgba);
			}


				//gruen
			for(i=1 ; i<=sizeZeroStrain ; i++)
			{
      		int index = (i-1) + (sizeQuartal1+sizeQuartal2);
      		rgba[0] = factor * 0; //1 - i/sizeZeroStrain;
      		rgba[1] = factor * 1;
      		rgba[2] = factor * 0; //i/sizeZeroStrain;
					rgba[3] = factor * 1;
					vtkLookupTable->SetTableValue (index, rgba);
			}


			// hellblau
			for (i=1 ; i<=sizeQuartal3; i++)
			{
      		int index = (i-1) + ( sizeQuartal1 + sizeQuartal2 + sizeZeroStrain );
      		rgba[0]= factor * 0;
      		rgba[1] = factor * (1 - i/sizeQuartal3);
      		rgba[2] = factor * 1;
					rgba[3] = factor * 1;
					vtkLookupTable->SetTableValue (index, rgba);
			}

			// blau
			for(i=1 ; i<=sizeQuartal4 ; i++)
			{
		      int index = (i-1) + (sizeQuartal1+sizeQuartal2+sizeQuartal3 + sizeZeroStrain);
    		  rgba[0] = factor * 0;
		      rgba[1] = factor * 0;
    		  rgba[2] = factor * (1- i/sizeQuartal4);
					rgba[3] = factor * 1;
					vtkLookupTable->SetTableValue (index, rgba);

			}

			// the value 0 is mapped to black
			// so we see the field-out-of-view as black
			int index=0;
			rgba[0] = 0;
			rgba[1] = 0;
			rgba[2] = 0;
			rgba[3] = factor * 1;
			vtkLookupTable->SetTableValue (index, rgba);
			vtkLookupTable->SetTableValue (index+1, rgba);

//			for (int i=0; i<size; i++)
//			{
//				vtkLookupTable->GetTableValue(i,&rgba[0]);
//				cout << "i=" <<  i << " r=" << rgba[0] << " g=" << rgba[1]<< " b=" << rgba[2] << endl;
//			}

				m_LookupTable->SetVtkLookupTable(vtkLookupTable);


	} else
		m_LookupTable = NULL;

	std::cout << "mitk::LookupTableSource::GetOutput() ... " << std::endl;
	return m_LookupTable;

}


