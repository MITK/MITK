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


#include "mitkImage.h"
#include "mitkPicFileReader.h"
#include "mitkPicHelper.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include <itksys/SystemTools.hxx>

#include <fstream>
int mitkPicFileReaderTest(int argc, char* argv[])
{
  //independently read header of pic file
  mitkIpPicDescriptor *picheader=NULL;
  if(argc>=1)
  {
    if(itksys::SystemTools::LowerCase(itksys::SystemTools::GetFilenameExtension(argv[1])).find(".pic")!=std::string::npos)
      picheader = mitkIpPicGetHeader(argv[1], NULL);
  }
  if(picheader==NULL)
  {
    std::cout<<"file not found/not a pic-file - test not applied [PASSED]"<<std::endl;
    std::cout<<"[TEST DONE]"<<std::endl;
    return EXIT_SUCCESS;
  }
  mitkIpPicGetTags(argv[1], picheader);

  //Read pic-Image from file
	mitk::PicFileReader::Pointer reader = mitk::PicFileReader::New();
	  reader->SetFileName(argv[1]);
    reader->Update();

  std::cout << "Testing IsInitialized(): ";
  if(reader->GetOutput()->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsSliceSet(): ";
  if(reader->GetOutput()->IsSliceSet(0)==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability of geometry: ";
  if(reader->GetOutput()->GetGeometry()==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing type of geometry (TimeSlicedGeometry expected): ";
  mitk::TimeSlicedGeometry* timegeometry;
  timegeometry = reader->GetOutput()->GetTimeSlicedGeometry();
  if(timegeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability of first geometry contained in the TimeSlicedGeometry: ";
  if(timegeometry->GetGeometry3D(0)==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing type of first geometry contained in the TimeSlicedGeometry (SlicedGeometry3D expected): ";
  mitk::SlicedGeometry3D* slicedgeometry;
  slicedgeometry = dynamic_cast<mitk::SlicedGeometry3D*>(timegeometry->GetGeometry3D(0));
  if(slicedgeometry==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing availability of first geometry contained in the SlicedGeometry3D: ";
  mitk::Geometry2D* geometry2d = slicedgeometry->GetGeometry2D(0);
  if(geometry2d==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing extent in units of first geometry contained in the SlicedGeometry3D: ";
  if((fabs(geometry2d->GetExtent(0)-picheader->n[0])>mitk::eps) || (fabs(geometry2d->GetExtent(1)-picheader->n[1])>mitk::eps))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing extent in units of image of SlicedGeometry3D: ";
  if((fabs(slicedgeometry->GetExtent(0)-picheader->n[0])>mitk::eps) || (fabs(slicedgeometry->GetExtent(1)-picheader->n[1])>mitk::eps)
    || (picheader->dim>2 && (fabs(slicedgeometry->GetExtent(2)-picheader->n[2])>mitk::eps))
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing extent in units of image of TimeSlicedGeometry: ";
  if((fabs(timegeometry->GetExtent(0)-picheader->n[0])>mitk::eps) || (fabs(timegeometry->GetExtent(1)-picheader->n[1])>mitk::eps)
    || (picheader->dim>2 && (fabs(timegeometry->GetExtent(2)-picheader->n[2])>mitk::eps))
    || (picheader->dim>3 && (abs((mitkIpInt4_t) timegeometry->GetTimeSteps()- (mitkIpInt4_t) picheader->n[3])>0))
    )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing consistency of spacing from matrix and stored spacing in the first SlicedGeometry3D: ";
  mitk::Vector3D spacing;
  spacing[0] = slicedgeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0).two_norm();
  spacing[1] = slicedgeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1).two_norm();
  spacing[2] = slicedgeometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2).two_norm();
  mitk::Vector3D readspacing=slicedgeometry->GetSpacing();
  mitk::Vector3D dist = spacing-readspacing;
  if(dist.GetSquaredNorm()>mitk::eps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //independently read the overall spacing 
  spacing.Fill(1);
  mitk::PicHelper::GetSpacing(picheader, spacing);

  std::cout << "Testing correct reading of overall spacing stored in the first SlicedGeometry3D: ";
  dist = spacing-readspacing;
  if(dist.GetSquaredNorm()>mitk::eps)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  if(picheader->dim==4)
  {
    std::cout << "4D dataset: Testing that timebounds are not infinite: ";
    if((slicedgeometry->GetTimeBounds()[0] == mitk::ScalarTypeNumericTraits::NonpositiveMin()) && 
       (slicedgeometry->GetTimeBounds()[1] == mitk::ScalarTypeNumericTraits::max())
      )
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    std::cout<<"[PASSED]"<<std::endl;
  }

  mitkIpPicFree(picheader);

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
