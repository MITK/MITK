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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/

#ifndef __itkContourExtractor2DImageFilter_h
#define __itkContourExtractor2DImageFilter_h

#include "itkImageToPathFilter.h"
#include "itkNumericTraits.h"
#include "itkPolyLineParametricPath.h"
#include "itkConceptChecking.h"
#include <itksys/hash_map.hxx>
#include "vcl_deque.h"
#include "vcl_list.h"

namespace itk
{
/** \class ContourExtractor2DImageFilter
 * \brief Computes a list of PolyLineParametricPath objects from the contours in
 * a 2D image.
 *
 * Uses the "marching squares" method to compute a the iso-valued contours of
 * the input 2D image for a given intensity value. Multiple outputs may be
 * produced because an image can have multiple contours at a given level, so it
 * is advised to call GetNumberOfOutputs() and GetOutput(n) to retrieve all of
 * the contours. The contour value to be extracted can be set with
 * SetContourValue(). Image intensities will be linearly interpolated to provide
 * sub-pixel resolution for the output contours.
 *
 * The marching squares algorithm is a special case of the marching cubes
 * algorithm (Lorensen, William and Harvey E. Cline. Marching Cubes: A High
 * Resolution 3D Surface Construction Algorithm. Computer Graphics (SIGGRAPH 87
 * Proceedings) 21(4) July 1987, p. 163-170). A simple explanation is available
 * here: http://www.essi.fr/~lingrand/MarchingCubes/algo.html
 *
 * There is a single ambiguous case in the marching squares algorithm: if a
 * given 2x2-pixel square has two high-valued and two low-valued pixels, each
 * pair diagonally adjacent. (Where high- and low-valued is with respect to the
 * contour value sought.) In this case, either the high-valued pixels can be
 * connected into the same "object" (where groups of pixels encircled by a given
 * contour are considered an object), or the low-valued pixels can be connected.
 * This is the "face connected" versus "face + vertex connected" (or 4- versus
 * 4-connected) distinction: high-valued pixels most be treated as one, and
 * low-valued as the other. By default, high-valued pixels are treated as
 * "face-connected" and low-valued pixels are treated as "face + vertex"
 * connected. To reverse this, call VertexConnectHighPixelsOn();
 *
 * Outputs are not guaranteed to be closed paths: contours which intersect the
 * image edge will be left open. All other paths will be closed. (The
 * closed-ness of a path can be tested by checking whether the beginning point
 * is the same as the end point.)
 *
 * Produced paths are oriented. Following the path from beginning to end, image
 * intensity values lower than the contour value are to the left of the path and
 * intensity values grater than the contour value are to the right. In other
 * words, the image gradient at a path segment is (approximately) in the direct
 * of that segment rotated right by 90 degrees, because the image intensity
 * values increase from left-to-right across the segment. This means that the
 * generated contours will circle clockwise around "hills" of
 * above-contour-value intensity, and counter-clockwise around "depressions" of
 * below-contour-value intensity. This convention can be reversed by calling
 * ReverseContourOrientationOn().
 *
 * By default the input image's largest possible region will be processed; call
 * SetRequestedRegion() to process a different region, or ClearRequestedRegion()
 * to revert to the default value. Note that the requested regions are usually
 * set on the output; however since paths have no notion of a "region", this
 * must be set at the filter level.
 *
 * This class was contributed to the Insight Journal by Zachary Pincus.
 *       http://insight-journal.org/midas/handle.php?handle=1926/165
 *
 * \sa Image
 * \sa Path
 * \sa PolyLineParametricPath
 *
 */
template <class TInputImage>
class ITK_EXPORT ContourExtractor2DImageFilter :
    public ImageToPathFilter< TInputImage, PolyLineParametricPath<2> >
{
public:
  /** Extract dimension from input and output image. */
  itkStaticConstMacro(InputImageDimension, unsigned int,
    TInputImage::ImageDimension);

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage                               InputImageType;
  typedef PolyLineParametricPath<2>                 OutputPathType;

  /** Standard class typedefs. */
  typedef ContourExtractor2DImageFilter                     Self;
  typedef ImageToPathFilter<InputImageType, OutputPathType> Superclass;
  typedef SmartPointer<Self>                                Pointer;
  typedef SmartPointer<const Self>                          ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ContourExtractor2DImageFilter, ImageToPathFilter);

  /** Image and path typedef support. */
  typedef typename InputImageType::Pointer                  InputImagePointer;
  typedef typename InputImageType::PixelType                InputPixelType;
  typedef typename InputImageType::IndexType                InputIndexType;
  typedef typename InputImageType::OffsetType               InputOffsetType;
  typedef typename InputImageType::RegionType               InputRegionType;
  typedef typename OutputPathType::Pointer                  OutputPathPointer;
  typedef typename OutputPathType::VertexType               VertexType;
  typedef typename OutputPathType::VertexListType           VertexListType;

  /** Real type associated to the input pixel type. */
  typedef typename NumericTraits<InputPixelType>::RealType  InputRealType;

  typedef typename VertexListType::ConstPointer
                                                       VertexListConstPointer;
   /** Control the orientation of the contours with reference to the image
  * gradient. (See class documentation.) */
  itkSetMacro(ReverseContourOrientation, bool);
  itkGetConstReferenceMacro(ReverseContourOrientation, bool);
  itkBooleanMacro(ReverseContourOrientation);

  /** Control whether high- or low-valued pixels are vertex-connected.
    * Default is for low-valued pixels to be vertex-connected.
    * (See class documentation.) */
  itkSetMacro(VertexConnectHighPixels, bool);
  itkGetConstReferenceMacro(VertexConnectHighPixels, bool);
  itkBooleanMacro(VertexConnectHighPixels);

  /** Control whether the largest possible input region is used, or if a
    * custom requested region is to be used. */
  void SetRequestedRegion(const InputRegionType region);
  itkGetConstReferenceMacro(RequestedRegion, InputRegionType);
  void ClearRequestedRegion();

  /** Set/Get the image intensity value that the contours should follow.
   *  This is the equivalent of an iso-value in Marching Squares. */
  itkSetMacro(ContourValue,InputRealType);
  itkGetConstReferenceMacro(ContourValue, InputRealType);


#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(DimensionShouldBe2,
    (Concept::SameDimension<itkGetStaticConstMacro(InputImageDimension),2>));
  itkConceptMacro(InputPixelTypeComparable,
    (Concept::Comparable<InputPixelType>));
  itkConceptMacro(InputHasPixelTraitsCheck,
    (Concept::HasPixelTraits<InputPixelType>));
  itkConceptMacro(InputHasNumericTraitsCheck,
    (Concept::HasNumericTraits<InputPixelType>));
  /** End concept checking */
#endif

protected:

  ContourExtractor2DImageFilter();
  virtual ~ContourExtractor2DImageFilter();
  void PrintSelf(std::ostream& os, Indent indent) const;

  void GenerateData();

  /** ContourExtractor2DImageFilter manually controls the input requested
    * region via SetRequestedRegion and ClearRequestedRegion, so it must
    * override the superclass method. */
  virtual void GenerateInputRequestedRegion()
            throw(InvalidRequestedRegionError);

private:
  VertexType InterpolateContourPosition(InputPixelType fromValue,
                                        InputPixelType toValue,
                                        InputIndexType fromIndex,
                                        InputOffsetType toOffset);
  void AddSegment(const VertexType from, const VertexType to);
  void FillOutputs();
  ContourExtractor2DImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  InputRealType                                   m_ContourValue;
  bool                                            m_ReverseContourOrientation;
  bool                                            m_VertexConnectHighPixels;
  bool                                            m_UseCustomRegion;
  InputRegionType                                 m_RequestedRegion;
  unsigned int                                    m_NumberOfContoursCreated;

  // Represent each contour as deque of vertices to facilitate addition of
  // nodes at beginning or end. At the end of the processing, we will copy
  // the contour into a PolyLineParametricPath.
  // We subclass the deque to store an additional bit of information: an
  // identification number for each growing contour. We use this number so
  // that when it becomes necessary to merge two growing contours, we can
  // merge the newer one into the older one. This helps because then we can
  // guarantee that the output contour list is ordered from left to right,
  // top to bottom (in terms of the first pixel of the contour encountered
  // by the marching squares). Currently we make no guarantees that this
  // pixel is the first pixel in the contour list, just that the contours
  // are so ordered in the output. Ensuring this latter condition (first
  // pixel traversed = first pixel in contour) would be possible by either
  // changing the merging rules, which would make the contouring operation
  //slower, or by storing additional data as to which pixel was first.
  class ContourType : public vcl_deque<VertexType>
    {
    public:
      unsigned int m_ContourNumber;
    };

  // Store all the growing contours in a list. We may need to delete contours
  // from anywhere in the sequence (when we merge them together), so we need to
  // use a list instead of a vector or similar.
  typedef vcl_list<ContourType>                         ContourContainer;
  typedef typename ContourContainer::iterator           ContourRef;

  // declare the hash function we are using for the hash_map.
  struct VertexHash
    {
    typedef typename VertexType::CoordRepType CoordinateType;
    inline size_t operator()(const VertexType& k) const
      {
      // Xor the hashes of the vertices together, after multiplying the
      // first by some number, so that identical (x,y) vertex indices
      // don't all hash to the same bucket. This is a decent if not
      // optimal hash.
      const size_t hashVertex1 = this->float_hash(k[0] * 0xbeef);
      const size_t hashVertex2 = this->float_hash(k[1]);
      const size_t hashValue = hashVertex1 ^ hashVertex2;
      return hashValue;
      }

    // Define hash function for floats. Based on method from
    // http://www.brpreiss.com/books/opus4/html/page217.html
    inline size_t float_hash(const CoordinateType &k) const
      {
      if (k == 0)
        {
        return 0;
        }
      int exponent;
      CoordinateType mantissa = vcl_frexp(k, &exponent);
      size_t value = static_cast<size_t>(vcl_fabs(mantissa));
      value = ( 2 * value - 1 ) * ~0U;
      return value;
      }
    };

  // We use a hash to associate the endpoints of each contour with the
  // contour itself. This makes it easy to look up which contour we should add
  // a new arc to.
  // We can't store the contours themselves in the hashtable because we
  // need to have two tables (one to hash from beginpoint -> contour and one
  // for endpoint -> contour), and sometimes will remove a contour from the
  // tables (if it has been closed or merged with another contour). So in the
  // hash table we store a reference to the contour. Because sometimes we will
  // need to merge contours, we need to be able to quickly remove contours
  // from our list when they have been merged into another. Thus, we store
  // an iterator pointing to the contour in the list.

  typedef itksys::hash_map<VertexType, ContourRef, VertexHash>    VertexToContourMap;
  typedef typename VertexToContourMap::iterator           VertexMapIterator;
  typedef typename VertexToContourMap::value_type         VertexContourRefPair;

  // The contours we find in the image are stored here
  ContourContainer                                        m_Contours;

  // And indexed by their beginning and ending points here
  VertexToContourMap                                      m_ContourStarts;
  VertexToContourMap                                      m_ContourEnds;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkContourExtractor2DImageFilter.txx"
#endif

#endif
