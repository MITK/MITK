#ifndef MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2

#include "mitkCommon.h"
#include "BaseData.h"
#include "Geometry2D.h"

namespace mitk {

//##ModelId=3E639C6C02C1
//##Documentation
//## @brief Data class containing Geometry2D objects
//## @ingroup Geometry
//## @todo time support needs to be added
class Geometry2DData : public BaseData
{
public:
    //##ModelId=3E6423D202FA
    mitkClassMacro(Geometry2DData, BaseData);

    itkNewMacro(Self);

    //##ModelId=3E6423D2030E
    //##Documentation
    //## @brief Set the reference to the Geometry2D at time @a t that is stored
    //## by the object
    //## @warning no time support yet. @t is ignored.
    virtual void SetGeometry2D( const mitk::Geometry2D* geometry2d, int t = 0 );
    //##ModelId=3E6423D20318
    //##Documentation
    //## @brief Get the reference to the Geometry2D at time @a t that is stored
    //## by the object
    //## @warning no time support yet. @t is ignored.
    //## @todo time support needs to be added
    const mitk::Geometry2D * GetGeometry2D(int t = 0)
    {
        return m_Geometry2D.GetPointer(); 
    };

    //##ModelId=3E66CC5A0295
	virtual void UpdateOutputInformation();

    //##ModelId=3E66CC5A02B4
	virtual void SetRequestedRegionToLargestPossibleRegion();

    //##ModelId=3E66CC5A02D2
	virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

    //##ModelId=3E66CC5A02F0
	virtual bool VerifyRequestedRegion();

    //##ModelId=3E66CC5A030E
	virtual void SetRequestedRegion(itk::DataObject *data);

    //##ModelId=3E67D85E00B7
    virtual void CopyInformation(const itk::DataObject *data);

protected:
    //##ModelId=3E639CD30201
    Geometry2DData();

    //##ModelId=3E639CD30233
    virtual ~Geometry2DData();

	Geometry2D::ConstPointer m_Geometry2D;
};

} // namespace mitk
#endif /* MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2 */
