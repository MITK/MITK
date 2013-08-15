/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkDicomRTReader.h"

namespace mitk
{

DicomRTReader::DicomRTReader(){}

DicomRTReader::~DicomRTReader(){}

  //############################################################################################################

  int DicomRTReader::ReadContourDataSequence0(char* filename)
  {
    std::cout << filename
              << "\n\n"
              << "###############################################################################################\n\n\n";
    gdcm::Scanner scanner;

    const gdcm::Tag tag_array[] = {
      gdcm::Tag(0x3006,0x0009),
      gdcm::Tag(0x3006,0x0010),
      gdcm::Tag(0x3006,0x0016),
      gdcm::Tag(0x3006,0x0039),
      gdcm::Tag(0x3006,0x0042),
      gdcm::Tag(0x3006,0x0046),
      gdcm::Tag(0x3006,0x0050),
      gdcm::Tag(0x2,0x2),
      gdcm::Tag(0x2,0x3),
      gdcm::Tag(0x2,0x10),
      gdcm::Tag(0x8,0x50),
      gdcm::Tag(0x8,0x60),
    };
    int tag_array_length = sizeof(tag_array) / sizeof(tag_array[0]);

    for(int x=0; x<tag_array_length; ++x)
    {
      scanner.AddTag(tag_array[x]);
    }

    gdcm::Directory::FilenamesType filenames;
    filenames.push_back( filename );
    scanner.Scan(filenames);

    //const char* outp = scanner.GetValue(filename, contourData);
    gdcm::Scanner::TagToValue const &ttv = scanner.GetMapping(filename);

    const gdcm::Tag *ptag = tag_array;
    for( ; ptag != tag_array + tag_array_length; ++ptag )
    {
      gdcm::Scanner::TagToValue::const_iterator it = ttv.find( *ptag );
      if( it != ttv.end() )
      {
        std::cout << *ptag << " was properly found in this file" << std::endl;
        const char *value = it->second;
        if( *value )
        {
          std::cout << "  It has the value: " << value << std::endl << "\n";
        }
        else
        {
          std::cout << "  It has no value (empty)" << std::endl << "\n";
        }
      }
      else
      {
        std::cout << "Sorry " << *ptag << " could not be found in this file" << std::endl << "\n";
      }
    }

    std::cout << "###############################################################################################\n\n\n";
    return 1;
  }

  //############################################################################################################

  int DicomRTReader::ReadContourDataSequence0_1(char* filename)
  {
    gdcm::Reader reader;
    reader.SetFileName( filename );

    std::cout << "FileName:" << filename << "\n\n";

    bool canRead = reader.CanRead();
    std::cout << "CanRead:" << canRead << "\n\n";
    
    std::filebuf fb;
    if(fb.open(filename,std::ios::in))
    {
      std::istream is(&fb);
      while(is)
        std::cout << char(is.get());
      fb.close();
      std::cout << "\n\n";
    }

    gdcm::Reader RTreader;
    RTreader.SetFileName( filename );
    if( !RTreader.Read() ) 
    {
      std::cout << "Problem reading file: " << filename << std::endl;
      return 0;
    }
    std::cout << "\n\nKein Problem anscheinend ?!\n\n";

    gdcm::File &file = RTreader.GetFile();

    /*
    reader.Read();

    gdcm::File &file = reader.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();
    gdcm::Tag tag(0x10,0x10);

    const gdcm::DataElement &Sequence = ds.GetDataElement( tag );
    gdcm::SequenceOfItems *sqi = Sequence.GetValueAsSQ();

    if( !sqi || sqi->GetNumberOfItems() != 1 )
    {
      std::cout << "sqi:" << sqi << "\n\n";
      return 1;
    }

    gdcm::Item &item = sqi->GetItem(1);
    gdcm::DataSet &nestedds = item.GetNestedDataSet();

    if( !nestedds.FindDataElement( gdcm::Tag( 0x3006,0x0050 ) ) ) 
    {
      std::cout << "!nestedds.FindDataElement( gdcm::Tag( 0x3006,0x0050 ) )\n\n";
      return 1;
    }

    const gdcm::DataElement &Content = nestedds.GetDataElement( gdcm::Tag( 0x3006, 0x0050 ) );
    const gdcm::ByteValue *bv = Content.GetByteValue();

    std::ofstream of( "outputFromDicomRT", std::ios::binary );
    of.write( bv->GetPointer(), bv->GetLength() );
    of.close();*/

    return 0;

  }

  //############################################################################################################

  int DicomRTReader::ReadContourDataSequence1(char* filename)
  {
    gdcm::Reader reader;
    reader.SetFileName(filename);

    gdcm::File &file = reader.GetFile();
    gdcm::DataSet &ds = file.GetDataSet();

    gdcm::FileExplicitFilter fef;
  
    fef.SetFile( reader.GetFile() );

    gdcm::Tag tag(0x3006,0x0039);

    const gdcm::DataElement &roicsq = ds.GetDataElement( tag );
    gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = roicsq.GetValueAsSQ();

    const gdcm::Item & item = sqi->GetItem(1); 
    const gdcm::DataSet& nestedds = item.GetNestedDataSet();

    gdcm::Tag tcsq(0x3006,0x0040);
    if( !nestedds.FindDataElement( tcsq ) )
      {
      return 0;
      }
    const gdcm::DataElement& csq = nestedds.GetDataElement( tcsq );
    gdcm::SmartPointer<gdcm::SequenceOfItems> sqi2 = csq.GetValueAsSQ();
    if( !sqi2 || !sqi2->GetNumberOfItems() )
      {
      return 0;
      }
    
    gdcm::Item & item2 = sqi2->GetItem(1);

    gdcm::DataSet& nestedds2 = item2.GetNestedDataSet();
    gdcm::Tag tcontourdata(0x3006,0x0050);
    const gdcm::DataElement & contourdata = nestedds2.GetDataElement( tcontourdata );

    //std::cout << contourdata << std::endl << "\n";

    gdcm::Attribute<0x3006,0x0046> ncontourpoints;
    ncontourpoints.Set( nestedds2 );

    gdcm::Attribute<0x3006,0x0050> at;
    at.SetFromDataElement( contourdata );
    const double* pts = at.GetValues();
    unsigned int npts = at.GetNumberOfValues() / 3;

    std::cout << npts << "\n";

    return 1;
  }

  //############################################################################################################

  int DicomRTReader::ReadContourDataSequence2(char* filename)
  {
    std::cout << "ReadContourDataSequence2 started\n\n";
    gdcm::Reader reader;
    reader.SetFileName( filename );
    if( !reader.Read() )
    {
      return 0;
    }
    gdcm::MediaStorage ms;
  ms.SetFromFile( reader.GetFile() );
  if( ms != gdcm::MediaStorage::RTStructureSetStorage )
    {
    return 0;
    }

    const gdcm::DataSet& ds = reader.GetFile().GetDataSet();
    gdcm::Tag troicsq(0x3006,0x0039);
    gdcm::Tag x(0x3006,0x0050);
    std::cout << ds.FindDataElement( troicsq ) << "\n\n" << ds.FindDataElement( x ) << "\n\n";
    if( !ds.FindDataElement( troicsq ) )
      {
      return 0;
      }

    const gdcm::DataElement &roicsq = ds.GetDataElement( troicsq );

    std::cout << ds.FindDataElement( x ) << "\n\n";

    const gdcm::SequenceOfItems  *sqi = roicsq.GetValueAsSQ();

    if( !sqi || !sqi->GetNumberOfItems() )
      {
        std::cout << "!sqi:" << sqi << "\n" << "!sqi->GetNumberOfItems():" << !sqi->GetNumberOfItems() << "\n";
        std::cout << sqi->GetItem(1) << "\n";
        return 0;
      }
    
    const gdcm::Item & item = sqi->GetItem(1);

    const gdcm::DataSet& nestedds = item.GetNestedDataSet();
    gdcm::Tag tcsq(0x3006,0x0040);
    if( !nestedds.FindDataElement( tcsq ) )
      {
      return 0;
     } 
    const gdcm::DataElement& csq = nestedds.GetDataElement( tcsq );
    std::cout << "Hier kommt was:" << csq << std::endl;
    std::cout << "ReadContourDataSequence2 ended\n\n";
    return 1;
  }

  //############################################################################################################

  int DicomRTReader::ReadContourDataSequence3(char* filename)
  {
    gdcm::Reader reader;
    reader.SetFileName(filename);
    gdcm::MediaStorage ms;
    ms.SetFromFile(reader.GetFile());
    const gdcm::DataSet& ds = reader.GetFile().GetDataSet();
    gdcm::Tag troicsq(0x3006,0x0039);
    const gdcm::DataElement &roicsq = ds.GetDataElement( troicsq );
    const gdcm::SequenceOfItems *sqi = roicsq.GetSequenceOfItems();
    const gdcm::Item & item = sqi->GetItem(1);
    
    std::cout << item << "\n\n";

    const gdcm::DataSet& nestedds = item.GetNestedDataSet();
    gdcm::Tag tcsq(0x3006,0x0040);
    const gdcm::DataElement& csq = nestedds.GetDataElement( tcsq );
    const gdcm::SequenceOfItems *sqi2 = csq.GetSequenceOfItems();
    unsigned int nitems = sqi2->GetNumberOfItems();
  
    std::cout << nitems << "\n\n";

    for(unsigned int i=0; i<nitems;++i)
    {
      const gdcm::Item & item2 = sqi2->GetItem(i+1);
      const gdcm::DataSet& nestedds2 = item2.GetNestedDataSet();
      gdcm::Tag tcontourdata(0x3006,0x0050);
      const gdcm::DataElement & contourdata = nestedds2.GetDataElement( tcontourdata );
      gdcm::Attribute<0x3006,0x0050> at;
      at.SetFromDataElement( contourdata );
      const double* pts = at.GetValues();
      for(unsigned int i = 0; i < (at.GetNumberOfValues() / 3) * 3; i+=3)
      {
        float x[3];
        x[0] = pts[i+0];
        x[1] = pts[i+1];
        x[2] = pts[i+2];
      }
    }

    return 1;
  }

}