/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkAbstractClassifier_h
#define mitkAbstractClassifier_h

#include <MitkCLCoreExports.h>


#include <mitkBaseData.h>

// Eigen
#include <itkeigen/Eigen/Dense>

// STD Includes

// MITK includes
#include <mitkConfigurationHolder.h>

namespace mitk
{
/**
 * \brief Abstract base class for machine learning classifiers.
 *
 * This class defines a common interface for training and prediction with
 * classifiers that operate on Eigen matrices. Derived classes must implement
 * training, prediction, and declare whether they support sample-level
 * weighting and probability estimation.
 *
 * Features include:
 * - Training from feature matrix X and label vector Y
 * - Prediction returning class labels
 * - Optional per-sample weighting
 * - Optional per-sample class probability output
 * - An item list stored via the property system for configuration
 *
 * \sa AbstractGlobalImageFeature
 */
class MITKCLCORE_EXPORT AbstractClassifier : public BaseData
{
public:

  mitkClassMacro(AbstractClassifier,BaseData);

  /**
   * \brief Train the classifier on a labeled dataset.
   *
   * \param[in] X Training input samples. Matrix of shape [n_samples, n_features].
   * \param[in] Y Target class labels. Matrix of shape [n_samples, 1].
   */
  virtual void Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y) = 0;

  /**
   * \brief Predict class labels for the given input samples.
   *
   * \param[in] X Input samples. Matrix of shape [n_samples, n_features].
   * \return Predicted class labels. Matrix of shape [n_samples, 1].
   */
  virtual Eigen::MatrixXi Predict(const Eigen::MatrixXd &X) = 0;

  /**
   * \brief Get the predicted label matrix from the last prediction.
   *
   * \return Reference to the label matrix of shape [n_samples, 1].
   */
  Eigen::MatrixXi & GetLabels()
  {
    return m_OutLabel;
  }

protected:
  Eigen::MatrixXi m_OutLabel;


public:
  // * --------------- *
  // PointWiseWeight
  // * --------------- *

  /**
   * \brief Query whether this classifier supports per-sample weighting.
   *
   * \return True if pointwise weighting is supported, false otherwise.
   */
  virtual bool SupportsPointWiseWeight() = 0;

  /**
   * \brief Get the per-sample weight matrix.
   *
   * \return Reference to the weight matrix of shape [n_samples, 1].
   */
  virtual Eigen::MatrixXd & GetPointWiseWeight()
  {
    return m_PointWiseWeight;
  }

  /**
   * \brief Set the per-sample weight matrix.
   *
   * \param[in] W Weight matrix of shape [n_samples, 1].
   */
  virtual void SetPointWiseWeight(const Eigen::MatrixXd& W)
  {
    this->m_PointWiseWeight = W;
  }

  /**
   * \brief Enable or disable per-sample weighting during training.
   *
   * \param[in] value True to enable weighting, false to disable.
   */
  virtual void UsePointWiseWeight(bool value)
  {
    this->m_IsUsingPointWiseWeight = value;
  }

  /**
   * \brief Query whether per-sample weighting is currently enabled.
   *
   * \return True if pointwise weighting is active.
   */
  virtual bool IsUsingPointWiseWeight()
  {
    return this->m_IsUsingPointWiseWeight;
  }

protected:
  Eigen::MatrixXd m_PointWiseWeight;
  bool m_IsUsingPointWiseWeight;

  // * --------------- *
  // PointWiseProbabilities
  // * --------------- *

public:
  /**
   * \brief Query whether this classifier supports per-sample class probability output.
   *
   * \return True if pointwise probability estimation is supported, false otherwise.
   */
  virtual bool SupportsPointWiseProbability() = 0;

  /**
   * \brief Get the per-sample class probability matrix from the last prediction.
   *
   * \return Reference to the probability matrix.
   */
  virtual Eigen::MatrixXd & GetPointWiseProbabilities()
  {
    return m_OutProbability;
  }

  /**
   * \brief Enable or disable per-sample class probability computation.
   *
   * \param[in] value True to enable probability output, false to disable.
   */
  virtual void UsePointWiseProbability(bool value)
  {
    m_IsUsingPointWiseProbability = value;
  }

  /**
   * \brief Query whether per-sample class probability output is currently enabled.
   *
   * \return True if pointwise probability output is active.
   */
  virtual bool IsUsingPointWiseProbability()
  {
    return m_IsUsingPointWiseProbability;
  }

protected:
  Eigen::MatrixXd m_OutProbability;
  bool m_IsUsingPointWiseProbability;

private:
  void MethodForBuild();

public:

  /**
   * \brief Set an item in the property-based item list at the given index.
   *
   * Items are stored as string properties under keys "itemlist.0", "itemlist.1", etc.
   *
   * \param[in] val The string value to store.
   * \param[in] idx Zero-based index position.
   */
  void SetNthItems(const char *val, unsigned int idx);

  /**
   * \brief Get the item string at the given index from the property-based item list.
   *
   * \param[in] idx Zero-based index position.
   * \return The string value at the given index.
   */
  std::string GetNthItems(unsigned int idx) const;

  /**
   * \brief Replace the entire item list with the given vector of strings.
   *
   * \param[in] list A vector of string items to store.
   */
  void SetItemList(std::vector<std::string> list);

  /**
   * \brief Retrieve all items from the property-based item list.
   *
   * \return A vector of strings in index order until a gap is found.
   */
  std::vector<std::string> GetItemList() const;

#ifndef DOXYGEN_SKIP

  void SetRequestedRegionToLargestPossibleRegion() override{}
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override{return true;}
  bool VerifyRequestedRegion() override{return false;}

  void SetRequestedRegion(const itk::DataObject* /*data*/) override{}


  // Override
  bool IsEmpty() const override
  {
    if(IsInitialized() == false)
      return true;
    const TimeGeometry* timeGeometry = const_cast<AbstractClassifier*>(this)->GetUpdatedTimeGeometry();
    if(timeGeometry == nullptr)
      return true;
    return false;
  }

#endif // Skip Doxygen

};
}

#endif
