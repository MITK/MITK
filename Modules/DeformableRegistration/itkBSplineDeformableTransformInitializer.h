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

#ifndef __itkBSplineDeformableTransformInitializer_h
#define __itkBSplineDeformableTransformInitializer_h

#include "itkObject.h"
#include "itkObjectFactory.h"

#include <iostream>

namespace itk
{

/** \class BSplineDeformableTransformInitializer
 * \brief BSplineDeformableTransformInitializer is a helper class intended to
 * initialize the grid parameters of a BSplineDeformableTransform based on the
 * parameters of an image.
 *
 * In the context of image registration, the image to be used are reference will
 * be the fixed image. The BSpline grid will use the fixed image as a base for
 * computing the grid spacing, orientation and origin, among other things.
 *
 *
 * \ingroup Transforms
 */
template < class TTransform, class TImage >
class ITK_EXPORT BSplineDeformableTransformInitializer : public Object
{
public:
  /** Standard class typedefs. */
  typedef BSplineDeformableTransformInitializer     Self;
  typedef Object                                    Superclass;
  typedef SmartPointer<Self>                        Pointer;
  typedef SmartPointer<const Self>                  ConstPointer;

  /** New macro for creation of through a Smart Pointer. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro( BSplineDeformableTransformInitializer, Object );

  /** Type of the transform to initialize */
  typedef TTransform                                TransformType;

  /** Types defined from transform traits */
  typedef typename TransformType::Pointer           TransformPointer;
  typedef typename TransformType::RegionType        TransformRegionType;
  typedef typename TransformRegionType::SizeType    TransformSizeType;


  /** Dimension of parameters. */
  itkStaticConstMacro(SpaceDimension, unsigned int,
                      TransformType::InputSpaceDimension);

  /** Image Types to use in the initialization of the transform */
  typedef   TImage                             ImageType;
  typedef   typename ImageType::ConstPointer   ImagePointer;


  /** Set the transform to be initialized */
  itkSetObjectMacro( Transform,   TransformType   );

  /** Set the fixed image used in the registration process */
  itkSetConstObjectMacro( Image,  ImageType  );

  /** Set the number of grid nodes that we want to place inside the image. This
   * method will override the settings of any previous call to
   * SetNumberOfGridNodesInsideTheImage().*/
  itkSetMacro( GridSizeInsideTheImage,  TransformSizeType  );

  /** Set the number of grid nodes that we want to place inside the image. This
   * number of node is used along one dimension of the image.  Therefore, if
   * you pass the number 5 as argument of this method, in a 3D space, then the
   * total number of grid nodes inside the image will be \$ 5 x 5 x 5 \$ .
   * This method will override the settings of any previous call to
   * SetGridSizeInsideTheImage().*/
  void SetNumberOfGridNodesInsideTheImage( unsigned int numberOfNodes )
    {
    this->m_GridSizeInsideTheImage.Fill( numberOfNodes );
    this->Modified();
    }

  /** Initialize the transform using data from the images */
  virtual void InitializeTransform() const;

protected:
  BSplineDeformableTransformInitializer();
  ~BSplineDeformableTransformInitializer(){};

  void PrintSelf(std::ostream &os, Indent indent) const override;

private:
  BSplineDeformableTransformInitializer(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  TransformPointer    m_Transform;

  ImagePointer        m_Image;

  TransformSizeType   m_GridSizeInsideTheImage;

  unsigned int        m_NumberOfGridNodesInsideTheImage;

}; //class BSplineDeformableTransformInitializer


}  // namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineDeformableTransformInitializer.txx"
#endif

#endif /* __itkBSplineDeformableTransformInitializer_h */
