/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkNavigationDataObjectVisualizationFilter_h
#define mitkNavigationDataObjectVisualizationFilter_h

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"
#include "mitkBaseData.h"

namespace mitk {

  /**
  * \brief Class that reads NavigationData from input and transfers the information to the geometry of the associated BaseData
  *
  * Derived from NavigationDataToNavigationDataFilter
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationDataObjectVisualizationFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataObjectVisualizationFilter, NavigationDataToNavigationDataFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** Defines the rotation modes of this tracking device which results in different representations
     *  of quaternions.
     *
     *  - Standard: normal representation, rawdata from the device is not changed (DEFAULT)
     *
     *  - Transposed: the rotation is stored transposed, which is (by mistake!) expected by some older MITK classes due
     *                to an ambigious method naming in VNL.
     *
     *  CAUTION: The rotation mode can only be changed for backward compatibility of old WRONG code.
     *           PLEASE DO NOT CHANGE THE ROTATION MODE UNLESS YOU ARE KNOWING EXACTLY WHAT YOU ARE DOING!
     *
     *  use SetRotationMode to change the mode.
     */
    enum RotationMode {RotationStandard, RotationTransposed};

    /**
    * \brief Smart Pointer type to a BaseData.
    */
    typedef BaseData::Pointer RepresentationPointer;

    /**
     * \brief STL map of index to BaseData . Using map to be able to set non continuous indices
     */
    typedef std::map<unsigned int, RepresentationPointer> RepresentationPointerMap;

    /**
    * \brief STL vector map of index to BaseData . Using map to be able to set non continuous indices
    */
    typedef std::map<unsigned int, std::vector<RepresentationPointer>> RepresentationVectorPointerMap;

     /**
     * \brief Size type of an std::vector
     */
    typedef RepresentationVectorPointerMap::size_type RepresentationPointerMapSizeType;

    /**
    * \brief Set the representation object of the input
    *
    * \param data The BaseData to be associated to the index
    * \param index the index with which data will be associated
    */
    void SetRepresentationObject(unsigned int index, BaseData::Pointer data);

    /**
     * \brief Set the representation objects vector of the input
     *
     * \param data The BaseData vector to be associated to the index
     * \param index the index with which data will be associated
     */
    void SetRepresentationObjects(unsigned int index, const std::vector<BaseData::Pointer> &data);
    
    /**
    * \brief Get the representation object associated with the index idx
    *
    * \param idx the corresponding input number with which the BaseData is associated
    * \return Returns the desired BaseData if it exists for the given input; Returns nullptr
    *         if no BaseData was found.
    */
    BaseData::Pointer GetRepresentationObject(unsigned int idx) const; 

    /**
     * \brief Get all the representation objects associated with the index idx
     *
     * \param idx the corresponding input number with which the BaseData is associated
     * \return Returns the desired BaseData if it exists for the given input; Returns nullptr
     *         if no BaseData was found.
     */
    std::vector<RepresentationPointer> GetAllRepresentationObjects(unsigned int idx) const;

    virtual void SetTransformPosition(unsigned int index, bool applyTransform); ///< if set to true, the filter will use the position part of the input navigation data at the given index to transform the representation object. If set to false, it will not. If no value is set, it defaults to true.
    virtual bool GetTransformPosition(unsigned int index) const; ///< returns whether position part of the input navigation data at the given index is used for the transformation of the representation object.
    virtual void TransformPositionOn(unsigned int index);  ///< sets the TransformPosition flag to true for the given index
    virtual void TransformPositionOff(unsigned int index); ///< sets the TransformPosition flag to false for the given index

    virtual void SetTransformOrientation(unsigned int index, bool applyTransform); ///< if set to true, the filter will use the orientation part of the input navigation data at the given index to transform the representation object. If set to false, it will not. If no value is set, it defaults to true.
    virtual bool GetTransformOrientation(unsigned int index) const; ///< returns whether orientation part of the input navigation data at the given index is used for the transformation of the representation object.
    virtual void TransformOrientationOn(unsigned int index);  ///< sets the TransformOrientation flag to true for the given index
    virtual void TransformOrientationOff(unsigned int index); ///< sets the TransformOrientation flag to false for the given index

    /** @brief Defines an offset for a representation object. This offset is applied before the object is visualized.
     *         If no offset is given, no offset will be used. To deactivate the offset just set it to nullptr. The offset is deactivated by default.
     * @param index
     * @param offset The new offset which will be set. Set to nullptr to deactivate the offset.
     */
    void SetOffset(int index, mitk::AffineTransform3D::Pointer offset);

    /** Sets the rotation mode of this class. See documentation of enum RotationMode for details
     *  on the different modes.
     *  CAUTION: The rotation mode can only be changed for backward compatibility of old WRONG code.
     *           PLEASE DO NOT CHANGE THE ROTATION MODE UNLESS YOU ARE KNOWING EXACTLY WHAT YOU ARE DOING!
     */
    virtual void SetRotationMode(RotationMode r);

    /** @return Returns the offset of a represenation object. Returns nullptr if there is no offset. */
    mitk::AffineTransform3D::Pointer GetOffset(int index);

    /**
    *\brief Get the number of added BaseData associated to NavigationData
    * \return Returns the size of the internal map
    */
    RepresentationPointerMapSizeType GetNumberOfToolRepresentations() const
    {
      return m_RepresentationVectorMap.size();
    }

    /*
    * \brief Transfer the information from the input to the associated BaseData
    */
    void GenerateData() override;


  protected:
    typedef std::map<itk::ProcessObject::DataObjectPointerArraySizeType, bool> BooleanInputMap;
    typedef std::map<unsigned int, mitk::AffineTransform3D::Pointer> OffsetPointerMap;

    /**
    * \brief Constructor
    **/
    NavigationDataObjectVisualizationFilter();

    /**
    * \brief Destructor
    **/
    ~NavigationDataObjectVisualizationFilter() override;

    /**
    * \brief An array of the BaseData which represent the tools.
    */

    RepresentationVectorPointerMap m_RepresentationVectorMap;
    BooleanInputMap m_TransformPosition;    ///< if set to true, the filter will use the position part of the input navigation data at the given index for the calculation of the transform. If no entry for the index exists, it defaults to true.
    BooleanInputMap m_TransformOrientation; ///< if set to true, the filter will use the orientation part of the input navigation data at the given index for the calculation of the transform. If no entry for the index exists, it defaults to true.
    OffsetPointerMap m_OffsetList;

  private:
    RotationMode m_RotationMode; ///< defines the rotation mode Standard or Transposed, Standard is default
  };
} // namespace mitk
#endif
