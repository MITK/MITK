/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _mitkContourModelSet_H_
#define _mitkContourModelSet_H_

#include "mitkCommon.h"
#include <MitkContourModelExports.h>

#include "mitkContourModel.h"

#include <deque>

namespace mitk
{
  /** \brief
  */
  class MITKCONTOURMODEL_EXPORT ContourModelSet : public mitk::BaseData
  {
  public:
    mitkClassMacro(ContourModelSet, mitk::BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef std::deque<mitk::ContourModel::Pointer> ContourModelListType;
    typedef ContourModelListType::iterator ContourModelSetIterator;

    //  start of inline methods

    /** \brief Return an iterator a the front.
    */
    virtual ContourModelSetIterator Begin() { return this->m_Contours.begin(); }
    /** \brief Return an iterator a the front.
    */
    virtual ContourModelSetIterator End() { return this->m_Contours.end(); }
    /** \brief Returns the number of contained contours.
    */
    virtual int GetSize() const { return this->m_Contours.size(); }
    //   end of inline methods

    /** \brief Add a ContourModel to the container.
    */
    virtual void AddContourModel(mitk::ContourModel &contourModel);

    /** \brief Add a ContourModel to the container.
    */
    virtual void AddContourModel(mitk::ContourModel::Pointer contourModel);

    /** \brief Returns the ContourModel a given index
    \param index
    */
    virtual mitk::ContourModel *GetContourModelAt(int index) const;

    /** \brief Returns the container of the contours.
    */
    ContourModelListType *GetContourModelList();

    /** \brief Returns a bool whether the container is empty or not.
    */
    bool IsEmpty() const override;

    /** \brief Remove the given ContourModel from the container if exists.
    \param contourModel - the ContourModel to be removed.
    */
    virtual bool RemoveContourModel(mitk::ContourModel *contourModel);

    /** \brief Remove a ContourModel at given index within the container if exists.
    \param index - the index where the ContourModel should be removed.
    */
    virtual bool RemoveContourModelAt(int index);

    /** \brief Clear the storage container.
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

#endif // _mitkContourModelSet_H_
