/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkContourModelSet_h
#define mitkContourModelSet_h

#include <mitkCommon.h>
#include <MitkContourModelExports.h>

#include <mitkContourModel.h>

#include <deque>

namespace mitk
{
  /** \brief A collection of ContourModel instances managed as a single BaseData object.
   *
   * ContourModelSet aggregates multiple mitk::ContourModel objects into a single data
   * structure that integrates with the MITK data management framework. It provides
   * methods to add, remove, and iterate over the contained contour models.
   *
   * The bounding box is automatically updated when contour models are added or removed.
   *
   * \sa ContourModel, ContourModelSetSource, ContourModelSetMapper2D, ContourModelSetMapper3D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelSet : public mitk::BaseData
  {
  public:
    mitkClassMacro(ContourModelSet, mitk::BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Container type for storing ContourModel smart pointers. */
      typedef std::deque<mitk::ContourModel::Pointer> ContourModelListType;
    /** \brief Iterator type for traversing the contour model collection. */
    typedef ContourModelListType::iterator ContourModelSetIterator;

    //  start of inline methods

    /** \brief Return an iterator to the first contour model in the set.
     * \return Iterator pointing to the beginning of the container.
     */
    virtual ContourModelSetIterator Begin() { return this->m_Contours.begin(); }

    /** \brief Return an iterator past the last contour model in the set.
     * \return Iterator pointing past the end of the container.
     */
    virtual ContourModelSetIterator End() { return this->m_Contours.end(); }

    /** \brief Return the number of contour models in the set.
     * \return Number of contained ContourModel instances.
     */
    virtual int GetSize() const { return this->m_Contours.size(); }
    //   end of inline methods

    /** \brief Add a contour model to the set by reference.
     * \param[in] contourModel The ContourModel to add.
     */
    virtual void AddContourModel(mitk::ContourModel &contourModel);

    /** \brief Add a contour model to the set by smart pointer.
     * \param[in] contourModel Smart pointer to the ContourModel to add.
     */
    virtual void AddContourModel(mitk::ContourModel::Pointer contourModel);

    /** \brief Return the contour model at the given index.
     * \param[in] index Zero-based index of the contour model to retrieve.
     * \return Pointer to the ContourModel, or nullptr if the index is out of range.
     */
    virtual mitk::ContourModel *GetContourModelAt(int index) const;

    /** \brief Return a pointer to the internal contour model container.
     * \return Pointer to the deque of ContourModel smart pointers.
     */
    ContourModelListType *GetContourModelList();

    /** \brief Check whether the set contains no contour models.
     * \return True if the container is empty, false otherwise.
     */
    bool IsEmpty() const override;

    /** \brief Remove a specific contour model from the set.
     * \param[in] contourModel Pointer to the ContourModel to remove.
     * \return True if the contour model was found and removed, false otherwise.
     */
    virtual bool RemoveContourModel(mitk::ContourModel *contourModel);

    /** \brief Remove the contour model at the given index.
     * \param[in] index Zero-based index of the ContourModel to remove.
     * \return True if the index was valid and the contour model was removed, false otherwise.
     */
    virtual bool RemoveContourModelAt(int index);

    /** \brief Remove all contour models from the set.
     * \post IsEmpty() returns true.
     */
    void Clear() override;

    //////////////// inherit  from mitk::BaseData ////////////////////

    /*                  NO support for regions !                    */

    void SetRequestedRegionToLargestPossibleRegion() override {}
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return false; }
    bool VerifyRequestedRegion() override { return true; }
    void SetRequestedRegion(const itk::DataObject *) override {}
    /**
    \brief Update the OutputInformation of a ContourModel object

    The BoundingBox of the contour will be updated, if necessary.
    */
    void UpdateOutputInformation() override;

    //////////////// END inherit  from mitk::BaseData ////////////////////

  protected:
    mitkCloneMacro(Self);

    ContourModelSet();
    ContourModelSet(const mitk::ContourModelSet &other);
    ~ContourModelSet() override;

    // inherit from BaseData. Initial state with no contours and a single timestep.
    void InitializeEmpty() override;

    ContourModelListType m_Contours;

    // only update the bounding geometry if necessary
    bool m_UpdateBoundingBox;
  };
} // namespace mitk

#endif
