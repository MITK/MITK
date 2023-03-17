/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDummyLsetReader_h
#define mitkDummyLsetReader_h

#include <mitkAbstractFileReader.h>

namespace mitk
{

/**
 * Read deprecated *.lset format (Multilabel Image format 2014)
 * @ingroup Process
 */
class DummyLsetFileReader : public mitk::AbstractFileReader
{
public:

    DummyLsetFileReader();
    DummyLsetFileReader(const mitk::DummyLsetFileReader& other);
    ~DummyLsetFileReader() override;

    using AbstractFileReader::Read;

protected:
   std::vector<itk::SmartPointer<BaseData>> DoRead() override;


  private:
    DummyLsetFileReader * Clone() const override;


};


} // end of namespace mitk

#endif
