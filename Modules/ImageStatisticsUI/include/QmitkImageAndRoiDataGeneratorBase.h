/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkImageAndRoiDataGeneratorBase_h
#define QmitkImageAndRoiDataGeneratorBase_h

#include <QmitkDataGeneratorBase.h>
#include <MitkImageStatisticsUIExports.h>

/**
 * \brief Base class for data generators that accept image and ROI nodes as vectors.
 *
 * Extends QmitkDataGeneratorBase to accept separate vectors of image nodes and ROI nodes
 * (as typically provided by node selection widgets). This class computes the Cartesian product
 * of all image/ROI combinations, ensuring that data is generated for every pairing. If no ROI
 * nodes are set, each image is processed without a mask.
 *
 * \sa QmitkDataGeneratorBase
 * \sa QmitkImageStatisticsDataGenerator
 */
class MITKIMAGESTATISTICSUI_EXPORT QmitkImageAndRoiDataGeneratorBase : public QmitkDataGeneratorBase
{
public:
  /** \brief Superclass typedef. */
  using Superclass = QmitkDataGeneratorBase;

  /** \brief Vector of const data node pointers. */
  using ConstNodeVectorType = std::vector<mitk::DataNode::ConstPointer>;
  /** \brief Vector of mutable data node pointers. */
  using NodeVectorType = std::vector<mitk::DataNode::Pointer>;

  /**
   * \brief Returns the currently set image nodes.
   * \return A vector of const image data node pointers.
   */
  ConstNodeVectorType GetImageNodes() const;

  /**
   * \brief Returns the currently set ROI nodes.
   * \return A vector of const ROI data node pointers.
   */
  ConstNodeVectorType GetROINodes() const;


public slots:
    /**
     * \brief Sets the image nodes for data generation.
     *
     * If auto-update is enabled and the nodes differ from the current set,
     * generation is automatically re-triggered.
     *
     * \param[in] imageNodes The vector of image data nodes.
     */
    void SetImageNodes(const ConstNodeVectorType& imageNodes);

    /**
     * \brief Convenience overload accepting mutable node pointers.
     * \param[in] imageNodes The vector of image data nodes.
     */
    void SetImageNodes(const NodeVectorType& imageNodes);

    /**
     * \brief Sets the ROI nodes for data generation.
     *
     * If auto-update is enabled and the nodes differ from the current set,
     * generation is automatically re-triggered.
     *
     * \param[in] roiNodes The vector of ROI data nodes.
     */
    void SetROINodes(const ConstNodeVectorType& roiNodes);

    /**
     * \brief Convenience overload accepting mutable node pointers.
     * \param[in] roiNodes The vector of ROI data nodes.
     */
    void SetROINodes(const NodeVectorType& roiNodes);

protected:
  QmitkImageAndRoiDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent = nullptr) : QmitkDataGeneratorBase(storage, parent) {};
  QmitkImageAndRoiDataGeneratorBase(QObject* parent = nullptr) : QmitkDataGeneratorBase(parent) {};

  using InputPairVectorType = Superclass::InputPairVectorType;
  
  bool ChangedNodeIsRelevant(const mitk::DataNode *changedNode) const override;
  InputPairVectorType GetAllImageROICombinations() const override;

  ConstNodeVectorType m_ImageNodes;
  ConstNodeVectorType m_ROINodes;

  QmitkImageAndRoiDataGeneratorBase(const QmitkImageAndRoiDataGeneratorBase&) = delete;
  QmitkImageAndRoiDataGeneratorBase& operator = (const QmitkImageAndRoiDataGeneratorBase&) = delete;
};

#endif
