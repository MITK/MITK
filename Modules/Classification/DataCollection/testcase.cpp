/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <testcase.h>

#include <mitkDataCollectionImageIterator.h>

void test::bla::test()
  {
    mitk::DataCollection::Pointer c = mitk::DataCollection::New();
    mitk::DataCollectionImageIterator<double, 3> ci(c,"test");
    ci.ToBegin();
    ci.IsAtEnd();
    ci++;
    ++ci;
    ci.GetVoxel();
    ci.SetVoxel(3);
    ci.GetFilePrefix();
};
