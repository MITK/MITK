/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef IMAGETOIMAGEFILTER_H_HEADER_INCLUDED_C1E5E869
#define IMAGETOIMAGEFILTER_H_HEADER_INCLUDED_C1E5E869

#include "mitkCommon.h"
#include "mitkImageSource.h"

namespace mitk {

//##ModelId=3E1A05C3005C
//##Documentation
//## @brief Superclass of all classes having one or more Images as input and
//## generating Images as output
//## @ingroup Process
class MITK_CORE_EXPORT ImageToImageFilter : public ImageSource
{
public:
	mitkClassMacro(ImageToImageFilter,ImageSource);

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Superclass typedefs. */
    //##ModelId=3E1A0A0303E6
	typedef Superclass::OutputImageRegionType OutputImageRegionType;

	/** Some convenient typedefs. */
    //##ModelId=3E1A0A040012
	typedef mitk::Image                    InputImageType;
    //##ModelId=3E1A0A040030
	typedef InputImageType::Pointer        InputImagePointer;
    //##ModelId=3E1A0A040044
	typedef InputImageType::ConstPointer   InputImageConstPointer;
    //##ModelId=3E1A0A040062
	typedef SlicedData::RegionType         InputImageRegionType; 

	/** Set/Get the image input of this process object.  */
    //##ModelId=3E1A0A04018E
	virtual void SetInput( const InputImageType *image);
    //##ModelId=3E1A0A0401CB
	virtual void SetInput( unsigned int, const InputImageType * image);
    //##ModelId=3E1A0A040225
	const InputImageType * GetInput(void);
    //##ModelId=3E1A0A040257
	const InputImageType * GetInput(unsigned int idx);


protected:
	//##ModelId=3E1A06100338
	ImageToImageFilter();
	//##ModelId=3E1A06100356
	virtual ~ImageToImageFilter();

    //##ModelId=3E1A0A040293
	virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

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
    //##ModelId=3E1A0A040301
	virtual void GenerateInputRequestedRegion();

private:
    //##ModelId=3E1A0A040316
	void operator=(const Self&); //purposely not implemented
};

} // namespace mitk



#endif /* IMAGETOIMAGEFILTER_H_HEADER_INCLUDED_C1E5E869 */
