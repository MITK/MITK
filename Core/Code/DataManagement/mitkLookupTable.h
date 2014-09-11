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
#ifndef MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D
#define MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D

#include <mitkCommon.h>
#include <MitkCoreExports.h>

#include <itkDataObject.h>
#include <itkObjectFactory.h>

#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>


class vtkColorTransferFunction;
class vtkPiecewiseFunction;

namespace mitk
{
/**
 * @brief The LookupTable class mitk wrapper for a vtkLookupTable
 * @ingroup DataManagement
 *
 * This class can be used to color images with a LookupTable, such as the
 * vtkLookupTable.
 * @note If you want to use this as a property for an mitk::Image, make sure
 * to use the mitk::LookupTableProperty and set the mitk::RenderingModeProperty
 * to a mode which supports lookup tables (e.g. LOOKUPTABLE_COLOR). Make
 * sure to check the documentation of the mitk::RenderingModeProperty. For a
 * code example how to use the mitk::LookupTable check the
 * mitkImageVtkMapper2DLookupTableTest.cpp in Core\Code\Testing.
 */
class MITK_CORE_EXPORT LookupTable : public itk::DataObject
{
public:
  /**
     *@brief Some convenient typedefs.
     */
  typedef unsigned char RawLookupTableType;

  mitkClassMacro( LookupTable, itk::DataObject );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
     * @returns the associated vtkLookupTable
     */
  virtual vtkSmartPointer<vtkLookupTable> GetVtkLookupTable() const;

  virtual RawLookupTableType * GetRawLookupTable() const;

  virtual void SetVtkLookupTable( vtkSmartPointer<vtkLookupTable> lut );

  virtual void ChangeOpacityForAll( float opacity );

  virtual void ChangeOpacity(int index, float opacity );

  virtual void GetColor(int, double rgb[3]);

  virtual void GetTableValue(int, double rgba[4]);

  virtual void SetTableValue(int, double rgba[4]);


  itkSetMacro(Window, float);
  itkSetMacro(Level, float);
  itkSetMacro(Opacity, float);


  /*!
    * \brief equality operator implementation
    */
  virtual bool operator==( const mitk::LookupTable& LookupTable ) const;

  /*!
    * \brief non equality operator implementation
    */
  virtual bool operator!=( const LookupTable& LookupTable ) const;

  /*!
    * \brief implementation necessary because operator made
    * private in itk::Object
    */
  virtual LookupTable& operator=( const LookupTable& LookupTable );

  /**
     * Updates the output information of the current object by calling
     * updateOutputInformation of the data objects source object.
     */
  virtual void UpdateOutputInformation( );

  /**
     * Sets the requested Region to the largest possible region.
     * This method is not implemented, since this is the default
     * behaviour of the itk pipeline and we do not support the
     * requested-region mechanism for lookup-tables
     */
  virtual void SetRequestedRegionToLargestPossibleRegion( );

  /**
     * Checks, if the requested region lies outside of the buffered region by
     * calling verifyRequestedRegion().
     */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion( );

  /**
     * Checks if the requested region is completely contained in
     * the buffered region. Since we always want to process the lookup
     * table as a whole, this method always returns true
     */
  virtual bool VerifyRequestedRegion( );

  /**
     * This method has no effect for lookup tables, since we do
     * not support the region-mechanism
     */
  virtual void SetRequestedRegion(const itk::DataObject *data );

  LookupTable();
  virtual ~LookupTable();
  /**
     * \deprecatedSince{2014_03} Please use CreateColorTransferFunction() instead
     */
  DEPRECATED(void CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction));
  /**
    * \deprecatedSince{2014_03} Please use CreateOpacityTransferFunction() instead
    */
  DEPRECATED(void CreateOpacityTransferFunction(vtkPiecewiseFunction*& opacityFunction));
  /**
   * \deprecatedSince{2014_03} Please use CreateGradientTransferFunction() instead
   */
  DEPRECATED(void CreateGradientTransferFunction(vtkPiecewiseFunction*& gradientFunction));

  vtkSmartPointer<vtkColorTransferFunction> CreateColorTransferFunction();
  vtkSmartPointer<vtkPiecewiseFunction> CreateOpacityTransferFunction();
  vtkSmartPointer<vtkPiecewiseFunction> CreateGradientTransferFunction();

  /**
   * @brief The LookupTableType enum for different predefined lookup tables.
   *
   * \li GRAYSCALE Our default level-window (sometimes referred to as window-level by other sources) setup for a test image looks like this:
   * \image html ExampleLevelWindowColor.png
   * \li INVERSE_GRAYSCALE Inverse LookupTable of GRAYSCALE.
   * \li HOT_IRON A LookupTable for red colors.
   * \li JET A LookupTable for JET color rendering.
   * \li LEGACY_BINARY A LookupTable for binary images.
   * \li LEGACY_RAINBOW_COLOR A rainbow-like LookupTable.
   * \li MULTILABEL A LookupTable for multilabel images.
   * \li PET_COLOR A LookupTable for PET color rendering.
   * \li PET_20 A LookupTable for PET_20 color rendering.
   *
   * The different LookupTableTypes can be applied in the MitkWorkbench via right-clicking
   * on an image and choosing a color map.
   */
  enum LookupTableType
  {
    GRAYSCALE,
    INVERSE_GRAYSCALE,
    HOT_IRON,
    JET,
    LEGACY_BINARY,
    LEGACY_RAINBOW_COLOR,
    MULTILABEL,
    PET_COLOR,
    PET_20
  };

  static const char* const typenameList[];

  /*!
     *  \brief Set the look-up table type by enum (or integer).
     *  \details Returns if the given type doesn't exist. Only changes the type if it is different
     *           from the current one.
     */
  virtual void SetType(const LookupTableType type);

  /*!
     *  \brief Set the look-up table type by string.
     *  \details Returns if the given type doesn't exist. Only changes the type if it is different
     *           from the current one.
     */
  virtual void SetType(const std::string& typeName);

  /*!
     *  \brief Return the current look-up table type as a string.
     */
  virtual const std::string GetActiveTypeAsString();
protected:

  void PrintSelf(std::ostream &os, itk::Indent indent) const;

  LookupTable(const LookupTable& other);

  virtual void BuildGrayScaleLookupTable();
  virtual void BuildLegacyBinaryLookupTable();
  virtual void BuildLegacyRainbowColorLookupTable();
  virtual void BuildInverseGrayScaleLookupTable();
  virtual void BuildHotIronLookupTable();
  virtual void BuildJetLookupTable();
  virtual void BuildPETColorLookupTable();
  virtual void BuildPET20LookupTable();
  virtual void BuildMultiLabelLookupTable();

  vtkSmartPointer<vtkLookupTable> m_LookupTable;

  float m_Window;

  float m_Level;

  float m_Opacity;

  LookupTableType m_type;

private:

  virtual itk::LightObject::Pointer InternalClone() const;
};
} // namespace mitk

#endif /* LookupTable_H_HEADER_INCLUDED_C1EBD53D */
