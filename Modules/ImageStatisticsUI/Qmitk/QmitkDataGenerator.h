/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __QMITK_DATA_GENERATOR_H
#define __QMITK_DATA_GENERATOR_H

#include "QmitkDataGeneratorBase.h"

/*!
\brief QmitkRTDataGenerator
Class that implements the generation of the required data
\tparam taskgenerationRule a specific taskgenerationRule class, where details about (job) requirements and dependencies
are defined (e.g. which jobs needs to compute the DVH)
\details Required inputs are:
- storage,
- doseNodes (>=1)
- structNodes (>=1).
The given dataStorage is used to save all generated data.
The taskgenerationRule is executed on all combinations of doseNodes and structNodes.
If new data is generated, the signal NewDataAvailable is emitted.
\note has a base class, because templated classes can't have QObject as parent class.
@sa QmitkRTDataGeneratorBase
*/
template <class taskGenerationRule>
class QmitkRTDataGenerator : public QmitkRTDataGeneratorBase
{
  public:
    /*! @brief Constructor
    @param storage the data storage that should be used
    */
    QmitkRTDataGenerator(mitk::DataStorage::Pointer storage) : QmitkRTDataGeneratorBase(storage) {};
    QmitkRTDataGenerator() : QmitkRTDataGeneratorBase() {};
    ~QmitkRTDataGenerator() override {};

    mitk::DataStorage::SetOfObjects::ConstPointer GetImageNodes() const;
    mitk::DataStorage::SetOfObjects::ConstPointer GetStructureNodes() const;

public slots:
    /*! @brief Setter for dose nodes
    */
    void SetImageNodes(mitk::DataStorage::SetOfObjects::ConstPointer imageNodes);
    /*! @brief Setter for struct nodes
    */
    void SetStructureNodes(mitk::DataStorage::SetOfObjects::ConstPointer structureNodes);

protected:
  /*! @brief Generate the data (by default all combinations of doseNodes and structNodes with the specified taskgenerationRule)
  */
  void DoGenerate() override;
  bool NodeChangeIsRelevant(const mitk::DataNode* changedNode) const override;
  std::vector<std::pair<mitk::DataNode::Pointer, mitk::DataNode::Pointer>> GetAllImageStructCombinations() const override;

  mitk::DataStorage::SetOfObjects::ConstPointer m_DoseNodes = nullptr;
  mitk::DataStorage::SetOfObjects::ConstPointer m_StructureNodes = nullptr;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "QmitkDataGenerator.tpp"
#endif

#endif
