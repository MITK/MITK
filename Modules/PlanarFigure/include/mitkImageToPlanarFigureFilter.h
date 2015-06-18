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


#ifndef IMAGETOPLANARFIGUREFILTER_H_HEADER_INCLUDED_C1E5E869
#define IMAGETOPLANARFIGUREFILTER_H_HEADER_INCLUDED_C1E5E869

#include "mitkCommon.h"
#include "mitkPlanarFigureSource.h"
#include "mitkImage.h"

namespace mitk {

//##Documentation
//## @brief Superclass of all classes having one or more Images as input and
//## generating PlanarFigures as output
//## @ingroup MitkPlanarFigureModule
class MITKPLANARFIGURE_EXPORT ImageToPlanarFigureFilter : public PlanarFigureSource
{
public:
  mitkClassMacro(ImageToPlanarFigureFilter,PlanarFigureSource);
  //itkFactorylessNewMacro(Self)
  //itkCloneMacro(Self)

  /** Some convenient typedefs. */
  typedef mitk::Image                    InputImageType;
  typedef InputImageType::Pointer        InputImagePointer;
  typedef InputImageType::ConstPointer   InputImageConstPointer;
  typedef SlicedData::RegionType         InputImageRegionType;

  /** Set/Get the image input of this process object.  */
  using Superclass::SetInput;
  virtual void SetInput( const InputImageType *image);
  virtual void SetInput( unsigned int, const InputImageType * image);
  const InputImageType * GetInput(void);
  const InputImageType * GetInput(unsigned int idx);


protected:
  ImageToPlanarFigureFilter();
  ~ImageToPlanarFigureFilter();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  /** What is the input requested region that is required to produce the
  * output requested region? The base assumption for image processing
  * filters is that the input requested region can be set to match the
  * output requested region.  If a filter requires more input (for instance
  * a filter that uses neighborhoods needs more input than output to avoid
  * introducing artificial boundary conditions) or less input (for instance
  * a magnify filter) will have to override this method.  In doing so, it
  * should call its superclass' implementation as its first step. Note that
  * this imaging filters operate differently than the classes to this
  * point in the class hierachy.  Up till now, the base assumption has been
  * that the largest possible region will be requested of the input.
  *
  * \sa ProcessObject::GenerateInputRequestedRegion(),
  *     ImageSource::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() override;

private:
  void operator=(const Self&); //purposely not implemented
};

} // namespace mitk



#endif /* IMAGETOPLANARFIGUREFILTER_H_HEADER_INCLUDED_C1E5E869 */
