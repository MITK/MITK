/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToOpenGLFilter.h"
#include <mitkVector.h>

mitk::NavigationDataToOpenGLFilter::NavigationDataToOpenGLFilter()
  : mitk::NavigationDataToNavigationDataFilter()
{
}


mitk::NavigationDataToOpenGLFilter::~NavigationDataToOpenGLFilter()
{
}

void mitk::NavigationDataToOpenGLFilter::GenerateData()
{
  this->CreateOutputsForAllInputs(); // make sure that we have the same number of outputs as inputs

  /* update outputs with tracking data from tools */
  unsigned int numberOfOutputs = this->GetNumberOfOutputs();
  for (unsigned int i = 0; i < numberOfOutputs ; ++i)
  {
    mitk::NavigationData* output = this->GetOutput(i);
    assert(output);
    const mitk::NavigationData* input = this->GetInput(i);
    assert(input);

    if (input->IsDataValid() == false || !input->GetHasOrientation()
      || !input->GetHasOrientation())
    {
      MITK_WARN("mitk::NavigationDataToOpenGLFilter::GenerateData()")
        << "Input Navigation Data is either invalid or has no position or "
        << "has no orientation.";
      continue;
    }

    /*
    vnl_vector<mitk::ScalarType> vnlTranslation
        = input->GetPosition().GetVnlVector();

    vnl_matrix<mitk::ScalarType> vnlRotation(
      input->GetOrientation().rotation_matrix_transpose());

    vnl_matrix_fixed<mitk::ScalarType, 4, 4> m;
    m.fill(0);
    m(3,3) = 1;
    m.update(vnlRotation);
    m.set_column(3, vnlTranslation);
    MITK_DEBUG << "left handed matrix: " << m;

    // switch to right/left handed
    m[1][0] = -m[1][0];
    m[1][1] = -m[1][1];
    m[1][2] = -m[1][2];
    m[1][3] = -m[1][3];
    m[2][0] = -m[2][0];
    m[2][1] = -m[2][1];
    m[2][2] = -m[2][2];
    m[2][3] = -m[2][3];

    MITK_DEBUG << "right handed matrix: " << m;

    vnlTranslation = m.get_column(3);
    vnlTranslation = vnlTranslation.extract(3);
    vnlRotation = m.extract(3, 3);

    output->SetPosition( mitk::Point3D( vnlTranslation.data_block() ) );
    output->SetOrientation( vnl_quaternion<mitk::ScalarType> ( vnlRotation ) );
    */

    output->Graft(input); // First, copy all information from input to output

    vnl_matrix<mitk::ScalarType> vnlRotation(
      input->GetOrientation().rotation_matrix_transpose());

    // normalize rows of rotation matrix
    vnlRotation.normalize_rows();

    vnl_matrix<mitk::ScalarType> vnlInverseRotation(3,3);
    // invert rotation
    vnlInverseRotation = vnl_matrix_inverse<mitk::ScalarType>(vnlRotation);

    vnl_vector<mitk::ScalarType> vnlTranslation
        = input->GetPosition().GetVnlVector();
    // rotate translation vector by inverse rotation P = P'
    vnlTranslation = vnlInverseRotation * vnlTranslation;
    vnlTranslation *= -1;  // save -P'

    // set new translation and rotation on output navdata objects
    mitk::Point3D position;
    mitk::FillVector3D(position,
                       vnlTranslation[0],
                       vnlTranslation[1],
                       vnlTranslation[2]);

    // set position
    output->SetPosition( mitk::Point3D( vnlTranslation.data_block() ) );
    output->SetOrientation( mitk::Quaternion( vnlRotation ) );

  }
}
