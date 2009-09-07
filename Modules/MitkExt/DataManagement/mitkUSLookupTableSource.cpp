/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkUSLookupTableSource.h"
#include <mitkIpPic.h>
#include <vtkLookupTable.h>
#include <fstream>


extern "C"
{
#include "uscfunctions/usc.h"
}


mitk::USLookupTableSource::~USLookupTableSource()
{}


mitk::USLookupTableSource::USLookupTableSource() : mitk::LookupTableSource()
{

    std::cout << "creating USLookupTableSource ... " << std::endl;
    m_Mode = DefaultLUT;
    m_LookupTable = NULL;

    this->Modified();    
    std::cout << "creating USLookupTableSource OK! " << std::endl;
}


void mitk::USLookupTableSource::GenerateData()
{
    std::cout << "USLookupTableSource::generate data!" << std::endl;
    OutputType::Pointer output = this->GetOutput();
    output->SetVtkLookupTable( this->BuildVtkLookupTable( ) );
}

vtkLookupTable* mitk::USLookupTableSource::BuildDSRDopplerLookupTable()
{

     std::cout << "  creating HP LookupTable ... " << std::endl;

     mitkIpPicDescriptor *HPMap;

        char MapFilename[ 20 ] = "a.map";
        int failed ;

        ifstream infile ( MapFilename, ios::in );
        failed = infile.fail() ;
        infile.close() ;

        if ( !failed )   // do we have a HP LUT ?
        {
            std::cout << "  reading a.map ... " << std::endl;
            HPMap = usReadMap( "a.map", ".", -1000, -1000 );

            vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
            ipUInt1_t *data = ( ( ipUInt1_t * ) HPMap->data );

            vtkLookupTable->SetTableRange(0,255);

            int LookupTablesize = 256;
            vtkFloatingPointType rgba[ 4 ];

            vtkLookupTable->SetNumberOfColors( LookupTablesize );
            for ( int i = 0; i < LookupTablesize; i++ )
            {
                rgba[ 0 ] = ( ( ipUInt1_t * ) data ) [ 0 + i * 3 * LookupTablesize ] / 255.0f;
                rgba[ 1 ] = ( ( ipUInt1_t * ) data ) [ 1 + i * 3 * LookupTablesize ] / 255.0f ;
                rgba[ 2 ] = ( ( ipUInt1_t * ) data ) [ 2 + i * 3 * LookupTablesize ] / 255.0f ;
                rgba[ 3 ] = 1;
                vtkLookupTable->SetTableValue ( i, rgba );
            }

            return ( vtkLookupTable );

        }  // do we have a HP LUT ?


        else  // no HP lut -> create custom LUT
        {     
            std::cout << "  no a.map available! creating custom Doppler LookUpTable ... " << std::endl;

            vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
            vtkLookupTable->SetTableRange(0,255);
            // size is the no of different colors in the lut
            int size = 256;
            int lutSize = 256;
            int repeats = lutSize / size;

            int factor = 1;
            int i, n;

            vtkFloatingPointType rgba[ 4 ];

            //int xDim = lutSize;
            //int yDim = 3;

            int quartalSize = size / 4;

            int index;

            for ( i = 1; i <= quartalSize ; i++ )
            {
                for ( n = 0; n < repeats ; n++ )
                {

                    index = ( i - 1 ) * repeats + n;
                    rgba[ 0 ] = 0;                      // rot
                    rgba[ 1 ] = 1 - i / 2.0 / quartalSize;    // gruen
                    rgba[ 2 ] = 1;                      // blau
                    rgba[ 3 ] = factor * 1;
                    vtkLookupTable->SetTableValue ( index, rgba );
                }
            }

            // dunkelblau
            for ( i = 1; i <= quartalSize ; i++ )
            {
                for ( n = 0; n < repeats ; n++ )
                {

                    index = ( i - 1 ) * repeats + repeats * quartalSize + n;
                    rgba[ 0 ] = 0;
                    rgba[ 1 ] = 0.5 - i / 2.0 / quartalSize;
                    rgba[ 2 ] = 1 - i / ( float ) ( 2 * quartalSize );
                    rgba[ 3 ] = factor * 1;
                    vtkLookupTable->SetTableValue ( index, rgba );
                }
            }



            // dunkelrot
            for ( i = 1; i <= quartalSize ; i++ )
            {
                for ( n = 0; n < repeats ; n++ )
                {

                    index = ( i - 1 ) * repeats + ( 2 * repeats * quartalSize ) + n;
                    rgba[ 0 ] = 0.5 + i / ( float ) ( 2 * quartalSize );
                    rgba[ 1 ] = i / 2.0 / quartalSize;
                    rgba[ 2 ] = 0;
                    rgba[ 3 ] = factor * 1;
                    vtkLookupTable->SetTableValue ( index, rgba );
                }
            }


            // hellrot
            for ( i = 1; i <= quartalSize ; i++ )
            {
                for ( n = 0; n < repeats ; n++ )
                {

                    index = ( i - 1 ) * repeats + ( 3 * repeats * quartalSize ) + n;
                    rgba[ 0 ] = 1;
                    rgba[ 1 ] = 0.5 + i / 2.0 / quartalSize;
                    rgba[ 2 ] = 0;
                    rgba[ 3 ] = factor * 1;
                    vtkLookupTable->SetTableValue ( index, rgba );
                }
            }

            // the value 0 is mapped to black
            // so we see the field-out-of-view as black
            index = 0;
            rgba[ 0 ] = 0;
            rgba[ 1 ] = 0;
            rgba[ 2 ] = 0;
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue( index, rgba );

            int mapZeroVelocityToBlack = 1;
            if ( mapZeroVelocityToBlack == 1 )
            {
                // map the middle value to black, so no velocity (v=0)
                // is displayed as black, otherwise darkred
                rgba[ 0 ] = 0;
                rgba[ 1 ] = 0;
                rgba[ 2 ] = 0;
                rgba[ 3 ] = factor * 1;
                index = lutSize / 2 ;
                vtkLookupTable->SetTableValue( index, rgba );
//                index = lutSize / 2 - 1;
//                vtkLookupTable->SetTableValue( index, rgba );
//                index = lutSize / 2 + 1;
                vtkLookupTable->SetTableValue( index, rgba );
            }

            //   for (int i=0; i<size; i++)
            //   {
            //    vtkLookupTable->GetTableValue(i,&rgba[0]);
            //    cout << "i=" <<  i << " r=" << rgba[0] << " g=" << rgba[1]<< " b=" << rgba[2] << endl;
            //   }

            return( vtkLookupTable );

        }  // no HP lut -> create custom LUT
}


vtkLookupTable* mitk::USLookupTableSource::BuildStrainRateLookupTable()
{
        std::cout << "  creating StrainRate  LookupTable ... " << std::endl;
        vtkLookupTable *vtkLookupTable = vtkLookupTable::New();

        vtkLookupTable->SetTableRange(0,255);

        int lutSize = 256;
        vtkLookupTable->SetNumberOfTableValues(lutSize);

        //   xDim = size;
        //   yDim = 3;
        vtkFloatingPointType rgba[ 4 ];

        float quartal = lutSize / 8.0f;

        float sizeQuartal1 = 3 * quartal; //quartal-1;

        float sizeQuartal2 = 0.9375 * quartal;
        float sizeZeroStrain = 0.125 * quartal;
        float sizeQuartal3 = 0.9375 * quartal;

        //   float sizeQuartal2 = 0.875 * quartal;
        //   float sizeZeroStrain = 0.25* quartal;
        //   float sizeQuartal3 = 0.875 * quartal;

        //   float sizeQuartal2 = 0.975 * quartal;
        //   float sizeZeroStrain = 0.05* quartal;
        //   float sizeQuartal3 = 0.975 * quartal;


        float sizeQuartal4 = 3 * quartal;

        //   std::cout << "quartal = " << quartal << std::endl;
        //   std::cout << "quartal 1 = " << sizeQuartal1 << std::endl;
        //   std::cout << "quartal 2 = " << sizeQuartal2 << std::endl;
        //   std::cout << "quartal 3 = " << sizeQuartal3 << std::endl;
        //   std::cout << "quartal 4 = " << sizeQuartal4 << std::endl;
        //   std::cout << "quartal zero = " << sizeZeroStrain << std::endl;


        int factor = 1;
        int i;

        // dunkelrot
        for ( i = 1; i <= sizeQuartal1; i++ )
        {
            int index = i - 1;
            rgba[ 0 ] = factor * ( 0.5 + i / ( 2 * sizeQuartal1 ) );      // rot
            rgba[ 1 ] = factor * 0;                           // gruen
            rgba[ 2 ] = factor * 0;                           // blau
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue ( index, rgba );
        }

        // hellrot bis gelb
        for ( i = 1 ; i <= sizeQuartal2 ; i++ )
        {
            int index = ( i - 1 ) + ( int ) sizeQuartal1;
            rgba[ 0 ] = factor * 1;
            rgba[ 1 ] = factor * ( i / sizeQuartal2 );
            rgba[ 2 ] = factor * 0;
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue ( index, rgba );
        }


        //gruen
        for ( i = 1 ; i <= sizeZeroStrain ; i++ )
        {
            int index = ( i - 1 ) + ( int ) ( sizeQuartal1 + sizeQuartal2 );
            rgba[ 0 ] = factor * 0; //1 - i/sizeZeroStrain;
            rgba[ 1 ] = factor * 1;
            rgba[ 2 ] = factor * 0; //i/sizeZeroStrain;
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue ( index, rgba );
        }


        // hellblau
        for ( i = 1 ; i <= sizeQuartal3; i++ )
        {
            int index = ( i - 1 ) + ( int ) ( sizeQuartal1 + sizeQuartal2 + sizeZeroStrain );
            rgba[ 0 ] = factor * 0;
            rgba[ 1 ] = factor * ( 1 - i / sizeQuartal3 );
            rgba[ 2 ] = factor * 1;
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue ( index, rgba );
        }

        // blau
        for ( i = 1 ; i <= sizeQuartal4 ; i++ )
        {
            int index = ( i - 1 ) + ( int ) ( sizeQuartal1 + sizeQuartal2 + sizeQuartal3 + sizeZeroStrain );
            rgba[ 0 ] = factor * 0;
            rgba[ 1 ] = factor * 0;
            rgba[ 2 ] = factor * ( 1 - i / sizeQuartal4 );
            rgba[ 3 ] = factor * 1;
            vtkLookupTable->SetTableValue ( index, rgba );

        }

        // the value 0 is mapped to black
        // so we see the field-out-of-view as black
        int index = 0;
        rgba[ 0 ] = 0;
        rgba[ 1 ] = 0;
        rgba[ 2 ] = 0;
        rgba[ 3 ] = factor * 1;
        vtkLookupTable->SetTableValue ( index, rgba );
        vtkLookupTable->SetTableValue ( index + 1, rgba );

        bool mapZeroStrainRateToBlack = true;
        if ( mapZeroStrainRateToBlack )
            {
                // map the middle value to black, so no velocity (v=0)
                // is displayed as black
                rgba[ 0 ] = 0;
                rgba[ 1 ] = 0;
                rgba[ 2 ] = 0;
                rgba[ 3 ] = factor * 1;
                
//                index = lutSize / 2 -1;  // FIXME: this should be lutSize/2 !!!!????
                index = 128 ;  // FIXME: this should be lutSize/2 !!!!????
                std::cout  << "  setting table value " << index << " to zero " << std::endl;
                
                vtkLookupTable->SetTableValue( index, rgba );
            }
        

//           for (int i=0; i<lutSize; i++)
//           {
//            vtkLookupTable->GetTableValue(i,&rgba[0]);
//            cout << "i=" <<  i << " r=" << rgba[0] << " g=" << rgba[1]<< " b=" << rgba[2] << endl;
//           }

//        std::cout << "  created with " << vtkLookupTable->GetNumberOfColors() << " colors .. " << std::endl;
//        std::cout << "  created with " << vtkLookupTable->GetNumberOfTableValues() << " colors .. " << std::endl;
//          float data[2];
//          vtkLookupTable->GetTableRange (data);
//          std::cout << "  data0=" << data[0] << "  data1=" << data[1] << std::endl;
//          vtkLookupTable->GetTableValue (200, rgba);
//          std::cout << "  r=" << rgba[0] << "  g=" << rgba[1] << "  b=" << rgba[2]<< "  a=" << rgba[3]<< std::endl;
//          float rgb[3];
//          vtkLookupTable->GetColor (200, rgb);
//          std::cout << "  r=" << rgb[0] << "  g=" << rgb[1] << "  b=" << rgb[2]<< std::endl;
//          unsigned char *p =   vtkLookupTable->GetPointer(200);
//          std::cout << "  p0=" << (int)p[0] << "  p1=" << (int)p[1] << "  p2=" << (int)p[2]<< std::endl;
//          p =   vtkLookupTable->GetPointer(128);
//          std::cout << "  p0=" << (unsigned int)p[0] << "  p1=" <<(unsigned int) p[1] << "  p2=" << (unsigned int)p[2]<< std::endl;
          
        return( vtkLookupTable );

}

vtkLookupTable* mitk::USLookupTableSource::BuildDefaultLookupTable(){

  std::cout << "  creating default LookupTable... " << std::endl;
  vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
  int size = 256;
  vtkLookupTable->SetTableRange(0,255);
  vtkLookupTable->SetNumberOfColors( size );
  vtkLookupTable->Build();
  return( vtkLookupTable );

}


vtkLookupTable* mitk::USLookupTableSource::BuildVtkLookupTable()
{

    std::cout << "mitk::USLookupTableSource::BuildVtkLookupTable() ... " << std::endl;
    
    if ( m_Mode == DSRDoppler )
    {
        return BuildDSRDopplerLookupTable();
     }
    else if ( m_Mode == StrainRate )
    {
      return BuildStrainRateLookupTable();
    }
    else
    {
      return BuildDefaultLookupTable();
    }


}



