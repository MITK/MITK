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

#ifndef _MITK_DecisionForestFileIO__H_
#define _MITK_DecisionForestFileIO__H_

#include <mitkAbstractFileIO.h>
#include "vigra/random_forest.hxx"

namespace mitk
{

/**
 * Writes vigra based mitk::DecisionForest
 * @ingroup Process
 */
class RandomForestFileIO : public mitk::AbstractFileIO
{
public:

    RandomForestFileIO();
    RandomForestFileIO(const mitk::RandomForestFileIO& other);
    virtual ~RandomForestFileIO();

    using AbstractFileIO::Write;
    virtual void Write();

    using AbstractFileIO::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

    virtual ConfidenceLevel GetReaderConfidenceLevel() const;
    virtual ConfidenceLevel GetWriterConfidenceLevel() const;


  protected:

    mutable vigra::RandomForest<int> m_rf;
//    DecisionForestFileIO(const DecisionForestFileIO& other);
//    virtual mitk::DecisionForestFileIO* Clone() const;

  private:
    virtual AbstractFileIO* IOClone() const;




};


} // end of namespace mitk

#endif
