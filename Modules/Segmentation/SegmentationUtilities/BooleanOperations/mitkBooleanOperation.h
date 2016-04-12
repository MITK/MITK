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

#ifndef mitkBooleanOperation_h
#define mitkBooleanOperation_h

#include <mitkLabelSetImage.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /** \brief Executes a boolean operation on two different segmentations.
   *
   * All parameters of the boolean operations must be specified during construction.
   * The actual operation is executed when calling GetResult().
   */
  class MITKSEGMENTATION_EXPORT BooleanOperation
  {
  public:
    enum Type
    {
      None,
      Difference,
      Intersection,
      Union
    };

    /* \brief Construct a boolean operation.
     *
     * Throws an mitk::Exception when segmentations are somehow invalid.
     *
     * \param[in] type The type of the boolean operation.
     * \param[in] segmentationA The first operand of the boolean operation.
     * \param[in] segmentationB The second operand of the boolean operation.
     * \param[in] The time step at which the operation will be executed.
     */
    BooleanOperation(Type type, Image::Pointer segmentationA, Image::Pointer segmentationB, unsigned int time = 0);
    ~BooleanOperation();

    /* \brief Execute boolean operation and return resulting segmentation.
     *
     * \return The resulting segmentation.
     */
    LabelSetImage::Pointer GetResult() const;

  private:
    BooleanOperation(const BooleanOperation &);
    BooleanOperation & operator=(const BooleanOperation &);

    LabelSetImage::Pointer GetDifference() const;
    LabelSetImage::Pointer GetIntersection() const;
    LabelSetImage::Pointer GetUnion() const;

    void ValidateSegmentation(Image::Pointer segmentation) const;
    void ValidateSegmentations() const;

    Type m_Type;
    Image::Pointer m_SegmentationA;
    Image::Pointer m_SegmentationB;
    unsigned int m_Time;
  };
}

#endif
