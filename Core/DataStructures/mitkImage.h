#ifndef MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2

#include "mitkCommon.h"
#include "SlicedData.h"
#include "ImageDataItem.h"
#include "PixelType.h"
#include "BaseData.h"
#include "LevelWindow.h"
#include "PlaneGeometry.h"
//#include "mitkImageToItk.h"

namespace mitk {

class SubImageSelector;
//template <class TPixel, unsigned int VImageDimension=2> class ImageToItk;

//##ModelId=3DCBC1E300FE
//##Documentation
//## @brief Image class for storing images
//## @ingroup Data
//## Can be asked for header information, the
//## data vector, ipPicDescriptors or vtkImageData objects. If not the complete
//## data is required, the appropriate SubImageSelector class should be used
//## for access.
//## Image organizes sets of slices (s x 2D), volumes (t x 3D) and channels (n
//## x ND). Channels are for different kind of data (with the same Geometry!),
//## e.g., morphology in channel 0, velocities in channel 1.
class Image : public SlicedData
{
    friend class SubImageSelector;
//    template <typename, unsigned int> friend class ImageToItk;

  public:
	mitkClassMacro(Image, SlicedData);    

    //##ModelId=3E18748C0045
    itkNewMacro(Self);

	/** Smart Pointer type to a ImageDataItem. */
    //##ModelId=3E0B7B3C00F0
	typedef ImageDataItem::Pointer ImageDataItemPointer;

    //##ModelId=3E0B7B3C0172
    //##Documentation
	//## @brief Type for STL Array of SmartPointers to ImageDataItems */
	typedef std::vector<ImageDataItemPointer> ImageDataItemPointerArray;

  public:

	//##ModelId=3DCBC2B50345
    const mitk::PixelType& GetType(int n = 0) const;

    //##ModelId=3DCBC5AA0112
    //##Documentation
    //## @brief Get dimension of the image
    //##
    unsigned int GetDimension() const;

    //##ModelId=3DCBC6040068
    //##Documentation
    //## @brief Get the size of dimension @a i (e.g., i=0 results in the number of pixels in x-direction).
    //##
    //## @sa GetDimensions()
    unsigned int GetDimension(int i) const;

    //##ModelId=3E0B494802D6
    //## @brief Get the data vector of the complete image, i.e., of all channels linked together.
    //##
    //## If you only want to access a slice, volume at a specific time or single channel
    //## use one of the SubImageSelector classes.
    virtual void* GetData();

    //##ModelId=3DCBEF2902C6
    //##Documentation
    //## @brief Get a volume at a specific time @a t of channel @a n as a vtkImageData.
    virtual vtkImageData* GetVtkImageData(int t = 0, int n = 0);

    //##ModelId=3DCBE2B802E4
    //##Documentation
    //## @brief Get the complete image, i.e., all channels linked together, as a @a ipPicDescriptor.
    //##
    //## If you only want to access a slice, volume at a specific time or single channel
    //## use one of the SubImageSelector classes.
    virtual ipPicDescriptor* GetPic();

	//##ModelId=3E1012990305
    //##Documentation
    //## @brief Check whether slice @a s at time @a t in channel @a n is set
    virtual bool IsSliceSet(int s = 0, int t = 0, int n = 0) const;

    //##ModelId=3E10139001BF
    //##Documentation
    //## @brief Check whether volume at time @a t in channel @a n is set
    virtual bool IsVolumeSet(int t = 0, int n = 0) const;

    //##ModelId=3E1550E700E2
    //##Documentation
    //## @brief Check whether the channel @a n is set
    virtual bool IsChannelSet(int n = 0) const;
  
    //##ModelId=3E10148003D7
    //##Documentation
    //## @brief Set @a data as slice @a s at time @a t in channel @a n. It is in
    //## the responsibility of the caller to ensure that the data vector @a data 
    //## is really a slice (at least is not smaller than a slice), since there is 
    //## no chance to check this.
    //## 
    //## @sa SetPicSlice
    virtual bool SetSlice(void *data, int s = 0, int t = 0, int n = 0);

    //##ModelId=3E1014A00211
    //##Documentation
    //## @brief Set @a data as volume at time @a t in channel @a n. It is in
    //## the responsibility of the caller to ensure that the data vector @a data 
    //## is really a volume (at least is not smaller than a volume), since there is 
    //## no chance to check this.
    //## 
    //## @sa SetPicVolume
    virtual bool SetVolume(void *data, int t = 0, int n = 0);

    //##Documentation
    //## @brief Set @a data in channel @a n. It is in
    //## the responsibility of the caller to ensure that the data vector @a data 
    //## is really a channel (at least is not smaller than a channel), since there is 
    //## no chance to check this.
    //## 
    //## @sa SetPicChannel
    virtual bool SetChannel(void *data, int n = 0);

    //##ModelId=3E1027F8023D
    //##Documentation
    //## @brief Set @a pic as slice @a s at time @a t in channel @a n. 
    //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
    //## are updated according to the information provided in the tags of @a pic.
    //## @return @a false : dimensions and/or data-type of @a pic does not
    //## comply with image 
    //## @a true success
    virtual bool SetPicSlice(ipPicDescriptor *pic, int s = 0, int t = 0, int n = 0);


    //##ModelId=3E102818024D
    //##Documentation
    //## @brief Set @a pic as volume at time @a t in channel @a n.
    //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
    //## are updated according to the information provided in the tags of @a pic.
    //## @return @a false : dimensions and/or data-type of @a pic does not
    //## comply with image 
    //## @a true success
    virtual bool SetPicVolume(ipPicDescriptor *pic, int t = 0, int n = 0);

    //##Documentation
    //## @brief Set @a pic in channel @a n. 
    //## @todo The corresponding @a Geomety3D and depending @a Geometry2D entries 
    //## are updated according to the information provided in the tags of @a pic.
    //## @return @a false : dimensions and/or data-type of @a pic does not
    //## comply with image 
    //## @a true success
    virtual bool SetPicChannel(ipPicDescriptor *pic, int n = 0);

    //##ModelId=3E102AE9004B
    //##Documentation
    //## initialize new (or re-initialize) image
    //## @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
    virtual void Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, unsigned int channels = 1);

    //##ModelId=3E102D2601DF
    //##Documentation
    //## initialize new (or re-initialize) image by @a pic. Dimensions and @a
    //## Geometry3D /@a Geometry2D  are set according to the tags in @a pic.
    //## @param tDim override time dimension (@a n[3]) in @a pic (if >0 and <)
    //## @param sDim override z-space dimension (@a n[2]) in @a pic (if >0 and <)
    //## @warning Initialize() by pic assumes a plane, evenly spaced geometry starting at (0,0,0).
    virtual void Initialize(ipPicDescriptor* pic, int channels = 1, int tDim = -1, int sDim = -1);

    //##ModelId=3E102D2601DF
    //##Documentation
    //## initialize new (or re-initialize) image by @a itkimage, an templated itk-image. Dimensions and @a
    //## Geometry3D /@a Geometry2D  are set according to the tags in @a pic.
    //## @param tDim override time dimension in @a itkimage (if >0 and <)
    //## @param sDim override z-space dimension in @a itkimage (if >0 and <)
    template <typename itkImageType> void InitializeByItk(itkImageType* itkimage, int channels = 1, int tDim = -1, int sDim=-1)
	 {
		if(itkimage==NULL) return;

		m_Dimension=itkimage->GetImageDimension();
		unsigned int i, *tmpDimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
		for(i=0;i<m_Dimension;++i) tmpDimensions[i]=itkimage->GetLargestPossibleRegion().GetSize().GetSize()[i];
		if(m_Dimension<4)
		{
			unsigned int *p;
			for(i=0,p=tmpDimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
				*p=1;
		}

		if((m_Dimension>2) && (sDim>=0))
			tmpDimensions[2]=sDim;
		if((m_Dimension>3) && (tDim>=0))
			tmpDimensions[3]=tDim;

		Initialize(mitk::PixelType(typeid(typename itkImageType::PixelType)), 
			m_Dimension, 
			tmpDimensions,
			channels);

    const double *spacinglist = itkimage->GetSpacing();
    Vector3D spacing(spacinglist[0],1.0,1.0);
    if(m_Dimension>=2)
        spacing.y=spacinglist[1];
    if(m_Dimension>=3)
        spacing.z=spacinglist[2];
    m_Geometry3D->SetSpacing(spacing);

    mitk::Point3D origin, right, bottom;
    origin.set(0,0,0);               m_Geometry3D->UnitsToMM(origin, origin);
    right.set(m_Dimensions[0],0,0);  m_Geometry3D->UnitsToMM(right, right);
    bottom.set(0,m_Dimensions[1],0); m_Geometry3D->UnitsToMM(bottom, bottom);

    PlaneView view_std(origin, right, bottom);

    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetSizeInUnits(m_Dimensions[0], m_Dimensions[1]);

    m_Geometry3D->SetGeometry2D(planegeometry.GetPointer(), 0, 0);
    m_Geometry3D->SetEvenlySpaced();

		delete tmpDimensions;
	};
  
	//##ModelId=3E155CF000F6
    //##Documentation
    //## @brief Check whether slice @a s at time @a t in channel @a n is valid, i.e., 
    //## is (or can be) inside of the image
    virtual bool IsValidSlice(int s = 0, int t = 0, int n = 0) const;

    //##ModelId=3E155D2501A7
    //##Documentation
    //## @brief Check whether volume at time @a t in channel @a n is valid, i.e., 
    //## is (or can be) inside of the image
    virtual bool IsValidVolume(int t = 0, int n = 0) const;

    //##ModelId=3E157C53030B
    //##Documentation
    //## @brief Check whether the channel @a n is valid, i.e., 
    //## is (or can be) inside of the image
    virtual bool IsValidChannel(int n = 0) const;

    //##ModelId=3E19EA110396
    //##Documentation
    //## @brief Check whether the image has been initialize, i.e., at least the header has to be set
	virtual bool IsInitialized() const;
    //##ModelId=3E1A11530384
    //##Documentation
    //## @brief Get the sizes of all dimensions as an integer-array.
    //##
    //## @sa GetDimension(int i);
    unsigned int* GetDimensions() const;

    //##ModelId=3ED91D060027
    //##Documentation
    //## @todo LevelWindow extraction/calculation not yet implemented. 
    virtual const mitk::LevelWindow& GetLevelWindow() const;

    //##ModelId=3ED91D060085
    //##Documentation
    //## @todo calculation of min/max not yet implemented. Timestamp must be added to track last calculation!
    virtual float GetScalarValueMin() const;

    //##ModelId=3ED91D0600E2
    //##Documentation
    //## @todo calculation of min/max not yet implemented. Timestamp must be added to track last calculation!
    virtual float GetScalarValueMax() const;

    //##ModelId=3E0B4A6A01EC
    //##Documentation
    //## @warning for internal use only
    virtual mitk::ImageDataItem::Pointer GetSliceData(int s = 0, int t = 0, int n = 0);
    //##ModelId=3E0B4A82001A
    //##Documentation
    //## @warning for internal use only
    virtual mitk::ImageDataItem::Pointer GetVolumeData(int t = 0, int n = 0);
    //##ModelId=3E0B4A9100BC
    //##Documentation
    //## @warning for internal use only
    virtual mitk::ImageDataItem::Pointer GetChannelData(int n = 0);

  protected:
    //##ModelId=3E155C940248
    int GetSliceIndex(int s = 0, int t = 0, int n = 0) const;

    //##ModelId=3E155C76012D
    int GetVolumeIndex(int t = 0, int n = 0) const;

    //##ModelId=3E155E7A0374
    void ComputeOffsetTable();

    //##ModelId=3E1569310328
    virtual mitk::ImageDataItem::Pointer AllocateSliceData(int s = 0, int t = 0, int n = 0);

    //##ModelId=3E15694500EC
    virtual mitk::ImageDataItem::Pointer AllocateVolumeData(int t = 0, int n = 0);

    //##ModelId=3E1569500322
    virtual mitk::ImageDataItem::Pointer AllocateChannelData(int n = 0);

    //##ModelId=3E15F6C60103
    Image();

    //##ModelId=3E15F6CA014F
    ~Image();

    //##ModelId=3E0B7B3C0245
	mutable ImageDataItemPointerArray m_Channels;
    //##ModelId=3E0B7B3C0263
	mutable ImageDataItemPointerArray m_Volumes;
    //##ModelId=3E0B4C1C02C3
	mutable ImageDataItemPointerArray m_Slices;

    //##ModelId=3E156293019B
	unsigned int m_Dimension;
    //##ModelId=3E0B890702C4
	unsigned int *m_Dimensions;
    //##ModelId=3E0B8907036E
	unsigned int *m_OffsetTable;
    //##ModelId=3E10151F001F
    ImageDataItem::Pointer m_CompleteData;
    //##ModelId=3E13FE96005F
	PixelType m_PixelType;

    //##ModelId=3E19EA110292
	bool m_Initialized;

    //##ModelId=3ED91D0503C2
    mitk::LevelWindow m_LevelWindow;
};

} // namespace mitk

#endif /* MITKIMAGE_H_HEADER_INCLUDED_C1C2FCD2 */

