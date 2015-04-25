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

#include <mitkInteractionConst.h>
//#include "vtkMath.h"
#include "vnl/vnl_cross.h"
#include "vnl/vnl_rotation_matrix.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkExtractSliceFilter.h"
#include "time.h"
#include "mitkImagePixelReadAccessor.h"
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include "mitkRotationOperation.h"

/*
*
*/
int mitkRotatedSlice4DTest(int  argc , char* argv[])
{
   MITK_TEST_BEGIN("mitkRotatedSlice4DTest");

   // Load preprocessed Image // TODO
   mitk::Image::Pointer image4D = mitk::IOUtil::LoadImage(std::string(
                                                            "D:/MITK-MBI11/bin/CMakeExternals/Source/MITK-MBI-Data/Mitral/img.nrrd"));
   // check inputs
   if ( image4D.IsNull() )
   {
      MITK_INFO << "Could not load all files";
      return false;
   }
   MITK_INFO << "Loading successful!";

   for (unsigned int ts = 0; ts < image4D->GetTimeSteps(); ts++)
   {
     MITK_INFO << "Timestep: " << ts;
     mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
     timeSelector->SetInput( image4D );
     timeSelector->SetTimeNr( ts );
     timeSelector->Update();
     mitk::Image::Pointer image3D = timeSelector->GetOutput();

     int sliceNumber = 5;

     mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
     plane->InitializeStandardPlane(image3D->GetGeometry(),
        mitk::PlaneGeometry::Frontal,sliceNumber, true,false);

     // rotate about an arbitrary point and axis...
     float angle = 30;
     mitk::Point3D point;
     point.Fill(sliceNumber);
     mitk::Vector3D rotationAxis;
     rotationAxis[0] = 1;
     rotationAxis[1] = 2;
     rotationAxis[2] = 3;
     rotationAxis.Normalize();

     // Create Rotation Operation
     mitk::RotationOperation* op = new mitk::RotationOperation(mitk::OpROTATE, point, rotationAxis, angle);
     plane->ExecuteOperation(op);
     delete op;

     // Now extract
     mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New(); // do not need NEW call when bug-13689 is merged in master
     extractor->SetInput(image3D);
     extractor->SetWorldGeometry(plane);
     MITK_INFO<< "Update Extraction" ;
     extractor->Update();
     mitk::Image::Pointer extractedPlane;
     extractedPlane = extractor->GetOutput();


   }
   MITK_TEST_END();
}
