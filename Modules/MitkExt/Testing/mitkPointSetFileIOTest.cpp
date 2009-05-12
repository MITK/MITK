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

#include "mitkPointSet.h"
#include "mitkPointSetWriter.h"
#include "mitkPointSetReader.h"

#include <itksys/SystemTools.hxx>

unsigned int numberOfTestPointSets = 1;

// create one test PointSet
mitk::PointSet::Pointer CreateTestPointSet(unsigned int which)
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  switch (which)
  {
    case 0:
    {
      unsigned int position(0);
      mitk::Point3D point;
      mitk::FillVector3D(point, 1.0, 2.0, 3.0);             pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
      mitk::FillVector3D(point, 2.0, 3.0, 4.0); ++position; pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
      mitk::FillVector3D(point, 3.0, 4.0, 5.0); ++position; pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
    }
    break;


    default:
    {
      unsigned int position(0);
      mitk::Point3D point;
      mitk::FillVector3D(point, 1.0, 2.0, 3.0);             pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
      mitk::FillVector3D(point, 2.0, 3.0, 4.0); ++position; pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
      mitk::FillVector3D(point, 3.0, 4.0, 5.0); ++position; pointSet->GetPointSet()->GetPoints()->InsertElement(position, point);
    }
  }

  return pointSet;

}

void PointSetCompare( mitk::PointSet* pointSet2, mitk::PointSet* pointSet1, bool& identical )
{
  if (pointSet1->GetSize() != pointSet2->GetSize())
  {
    std::cerr << "point sets differ in size" << std::endl;
    identical = false;
    return;
  }

  for (unsigned int i = 0; i < (unsigned int)pointSet1->GetSize(); ++i)
  {
    mitk::Point3D p1 = pointSet1->GetPoint(i);
    mitk::Point3D p2 = pointSet2->GetPoint(i);

    double difference = (   (p1[0] - p2[0])
                          + (p1[1] - p2[1])
                          + (p1[2] - p2[2])
                        );

    if (difference > 0.0001)
    {
      std::cerr << "points not at the same position" << std::endl;
      identical = false;
      break;
    }
  }

}

int mitkPointSetFileIOTest(int, char*[])
{
  unsigned int numberFailed(0);

  for (unsigned int i = 0; i < numberOfTestPointSets; ++i)
  {
    mitk::PointSet::Pointer originalPointSet = CreateTestPointSet(i);
    mitk::PointSet::Pointer secondPointSet;

    // write
      try
      {
        mitk::PointSetWriter::Pointer pointSetWriter = mitk::PointSetWriter::New();
        pointSetWriter->SetInput(originalPointSet);
        pointSetWriter->SetFileName("test_pointset.mps");
        pointSetWriter->Write();
      }
      catch ( std::exception& e )
      {
        std::cerr << "Error during attempt to write 'test_pointset.mps' Exception says:" << std::endl;
        std::cerr << e.what() << std::endl;
        ++numberFailed;
        continue;
      }

    // load
      try
      {
        mitk::PointSetReader::Pointer pointSetReader = mitk::PointSetReader::New();
        
        pointSetReader->SetFileName("test_pointset.mps");
        pointSetReader->Update();

        secondPointSet = pointSetReader->GetOutput();
      }
      catch ( std::exception& e )
      {
        std::cerr << "Error during attempt to read 'test_pointset.mps' Exception says:" << std::endl;
        std::cerr << e.what() << std::endl;
        ++numberFailed;
        continue;
      }

      if (secondPointSet.IsNull())
      {
        std::cerr << "Error reading 'test_pointset.mps'. No PointSet created." << std::endl;
        ++numberFailed;
        continue;
      }
    
    std::remove( "test_pointset.mps" );
  
    // compare
    bool identical(true);
    PointSetCompare( secondPointSet.GetPointer(), originalPointSet.GetPointer(), identical );

    if (!identical)
    {
      std::cerr << "PointSets differ for testPointSet " << i << std::endl;
      ++numberFailed;
      continue;
    }
  }

  // if one fails, whole test fails
  if (numberFailed > 0)
  {
    std::cout << "[FAILED]: " << numberFailed << " of " << numberOfTestPointSets << " sub-tests failed." << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
    return EXIT_SUCCESS;
  }

}

