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

#ifndef _MITK_DummyLsetFileReader__H_
#define _MITK_DummyLsetFileReader__H_

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
    virtual ~DummyLsetFileReader();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

  private:
    virtual DummyLsetFileReader * Clone() const;


};


} // end of namespace mitk

#endif
