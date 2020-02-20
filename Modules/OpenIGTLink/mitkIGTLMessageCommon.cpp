/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMessageCommon.h"
#include "mitkException.h"

mitk::IGTLMessageCommon::IGTLMessageCommon() : itk::DataObject()
{
}


mitk::IGTLMessageCommon::~IGTLMessageCommon()
{
}

//bool mitk::IGTLMessageCommon::Clone(igtl::TransformMessage::Pointer clone,
//                                    igtl::TransformMessage::Pointer original)
//{
//  bool copySuccess = false;

//  //copy all meta data
//  copySuccess = clone->Copy(original);

//  if ( !copySuccess )
//    return false;

//  //copy all data that is important for this class
//  //copy the matrix
//  igtl::Matrix4x4 mat;
//  original->GetMatrix(mat);
//  clone->SetMatrix(mat);

//  //copy the normals
//  float normals[3][3];
//  original->GetNormals(normals);
//  clone->SetNormals(normals);

//  //copy the position
//  float position[3];
//  original->GetPosition(position);
//  clone->SetPosition(position);

//  return true;
//}

//igtl::MessageBase::Pointer mitk::IGTLMessageCommon::Clone(igtl::TransformMessage::Pointer original)
//{
//  bool copySuccess = false;
//  igtl::TransformMessage::Pointer clone_ = igtl::TransformMessage::New();

//  //initialize the clone
////  clone = igtl::MessageBase::New();

//  //copy all meta data
//  copySuccess = clone_->Copy(original);

//  if ( !copySuccess )
//    return false;

//  //copy all data that is important for this class
//  //copy the matrix
//  igtl::Matrix4x4 mat;
//  original->GetMatrix(mat);
//  clone_->SetMatrix(mat);

//  //copy the normals
//  float normals[3][3];
//  original->GetNormals(normals);
//  clone_->SetNormals(normals);

//  //copy the position
//  float position[3];
//  original->GetPosition(position);
//  clone_->SetPosition(position);

//  return igtl::MessageBase::Pointer(clone_.GetPointer());

////  return true;
//}
