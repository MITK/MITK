/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"
#include <fstream>
#include <iostream>
#include <mitkCustomMimeType.h>

mitk::ContourModelSetWriter::ContourModelSetWriter() : AbstractFileWriter(ContourModelSet::GetStaticNameOfClass())
{
  std::string category = "ContourModelSet File";
  mitk::CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("cnt_set");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  RegisterService();
}

mitk::ContourModelSetWriter::ContourModelSetWriter(const mitk::ContourModelSetWriter &other) : AbstractFileWriter(other)
{
}

mitk::ContourModelSetWriter::~ContourModelSetWriter()
{
}

void mitk::ContourModelSetWriter::Write()
{
  std::ostream *out;
  std::ofstream outStream;

  if (this->GetOutputStream())
  {
    out = this->GetOutputStream();
  }
  else
  {
    outStream.open(this->GetOutputLocation().c_str());
    out = &outStream;
  }

  if (!out->good())
  {
    mitkThrow() << "Stream not good.";
  }

  *out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";

  // Use regular ContourModel writer to write each contour of the set to a single file.
  // Just use a different file extension .cnt_set

  bool writeXMLHeader = false;
  mitk::ContourModelWriter writer(writeXMLHeader);

  mitk::ContourModelSet::ConstPointer contourModelSet = dynamic_cast<const mitk::ContourModelSet *>(this->GetInput());

  //
  // for each contour object set input of writer
  //
  for (int i = 0; i < contourModelSet->GetSize(); ++i)
  {
    const mitk::ContourModel *contour = contourModelSet->GetContourModelAt(i);
    writer.SetInput(contour);
    writer.SetOutputStream(this->GetOutputLocation(), out);
    writer.Write();
  }
}

mitk::ContourModelSetWriter *mitk::ContourModelSetWriter::Clone() const
{
  return new ContourModelSetWriter(*this);
}
