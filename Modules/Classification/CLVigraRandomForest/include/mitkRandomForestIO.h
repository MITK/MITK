/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    ~RandomForestFileIO() override;

    using AbstractFileIO::Write;
    void Write() override;

    using AbstractFileIO::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;


protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;


    mutable vigra::RandomForest<int> m_rf;
//    DecisionForestFileIO(const DecisionForestFileIO& other);
//    virtual mitk::DecisionForestFileIO* Clone() const;

  private:
    AbstractFileIO* IOClone() const override;




};


} // end of namespace mitk

#endif
