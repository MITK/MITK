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

#ifndef _POINT_BASED_REGISTRATION_CONTROL_PARAMETERS_H_
#define _POINT_BASED_REGISTRATION_CONTROL_PARAMETERS_H_

/*!
  class holds all parameters which are accessible or controllable
  via the GUI and which are needed for the deformable model algorithm
*/
class PointBasedRegistrationControlParameters
{

public:
  PointBasedRegistrationControlParameters(){
    transformationType=0;
  }

  void setTransformationType(int type){
    transformationType=type;
  }
  int getTransformationType(){
    return transformationType;
  }

  void setFixed(bool fixed){
    this->fixed=fixed;
  }
  bool getFixed(){
    return fixed;
  }

  void setMoving(bool moving){
    this->moving=moving;
  }
  bool getMoving(){
    return moving;
  }

private:
  //0:rigid with ICP, 1:similarity with ICP, 2:affine with ICP, 3:rigid, 4:similarity, 5:affine, 6:landmarkwarping
  int transformationType;
  bool fixed;
  bool moving;

};
#endif

