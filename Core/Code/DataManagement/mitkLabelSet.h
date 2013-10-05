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


#ifndef _mitkLabelSet_H_
#define _mitkLabelSet_H_

#include <MitkExports.h>
#include <mitkCommon.h>
#include <mitkMessage.h>
#include <mitkLookupTable.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkLabel.h>

namespace mitk
{
//
//Documentation
// @brief LabelSet containing the labels corresponding to a segmentation session.
// @ingroup Data
//
class MITK_CORE_EXPORT LabelSet : public itk::Object
{
public:

    mitkClassMacro( LabelSet, itk::Object );

    mitkNewMacro1Param( Self, int );

    typedef std::vector <mitk::Label::Pointer>    LabelContainerType;
    typedef LabelContainerType::const_iterator    LabelContainerConstIteratorType;
    typedef LabelContainerType::iterator          LabelContainerIteratorType;

    void Initialize(const LabelSet* other);

    /** \brief Returns a const iterator poiting to the begining of the container.
    */
    LabelContainerConstIteratorType IteratorBegin();

    /** \brief Returns a const iterator pointing to the end of the container.
    */
    LabelContainerConstIteratorType IteratorEnd();

    /** \brief
    */
    itkSetStringMacro(LastModified);

    /** \brief
    */
    itkGetStringMacro(LastModified);

    /** \brief
    */
    itkSetStringMacro(Owner);

    /** \brief
    */
    itkGetStringMacro(Owner);

    /** \brief
    */
    itkGetStringMacro(Name);

    /** \brief
    */
    itkSetStringMacro(Name);

    /** \brief
    */
    void SetActiveLabel(int);

    /** \brief
    */
    void RemoveLabel(int);

    /** \brief
    */
    void RemoveSelectedLabels();

    /** \brief
    */
    void RemoveLabels(int begin, int count);

    /** \brief
    */
    void AddLabel(const mitk::Label& label);

    /** \brief
    */
    void AddLabel(const std::string& name, const mitk::Color& color);

    /** \brief
    */
    void RenameLabel(int, const std::string&, const mitk::Color&);

    /** \brief
    */
    int GetNumberOfLabels() const;

    /** \brief
    */
    void SetAllLabelsVisible(bool);

    /** \brief
    */
    void SetAllLabelsLocked(bool);

    /** \brief
    */
    void RemoveAllLabels();

    /** \brief
    */
    void SetLabelVisible(int index, bool value);

    /** \brief
    */
    bool GetLabelVisible(int index);

    /** \brief
    */
    void SetLabelLocked(int index, bool value);

    /** \brief
    */
    bool GetLabelLocked(int index);

    /** \brief
    */
    unsigned int GetLabelLayer(int index) const;

    /** \brief
    */
    void SetLabelSelected(int index, bool value);

    /** \brief
    */
    bool GetLabelSelected(int index);

    /** \brief
    */
    void SetLabelOpacity(int index, float value);

    /** \brief
    */
    float GetLabelOpacity(int index);

    /** \brief
    */
    void SetLabelVolume(int index, float value);

    /** \brief
    */
    float GetLabelVolume(int index);

    /** \brief
    */
    void SetLabelName(int index, const std::string &name);

    /** \brief
    */
    std::string GetLabelName(int index);

    /** \brief
    */
    void SetLabelColor(int index, const mitk::Color &color);

    /** \brief
    */
    const mitk::Color& GetLabelColor(int index);

    /** \brief
    */
    const mitk::Label* GetActiveLabel() const { return m_ActiveLabel; };

    /** \brief
    */
    mitk::Label::ConstPointer GetLabel(int index) const;

    /** \brief
    */
    int GetActiveLabelIndex() const;

    /** \brief
    */
    int GetActiveLabelLayer() const;

    /** \brief
    */
    void SetLabelCenterOfMassIndex(int index, const mitk::Point3D& center);

    /** \brief
    */
    const mitk::Point3D& GetLabelCenterOfMassIndex(int index);

    /** \brief
    */
    void SetLabelCenterOfMassCoordinates(int index, const mitk::Point3D& center);

    /** \brief
    */
    const mitk::Point3D& GetLabelCenterOfMassCoordinates(int index);

    /** \brief
    */
    void ResetLabels();

    unsigned int GetLayer() { return m_Layer; };

    mitk::LookupTable* GetLookupTable();

    void SetLookupTable( mitk::LookupTable* lut);
    /** \brief
    */
    static bool IsSelected(mitk::Label::Pointer label);

protected:

    LabelSet(int layer);
    virtual ~LabelSet();

//    LabelSet(const LabelSet& other);

    /** \brief
    */
    void CheckHasLabel(int);

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    LabelContainerType m_LabelContainer;

    mitk::LookupTable::Pointer m_LookupTable;

    mitk::Label* m_ActiveLabel;

    std::string m_Owner;

    std::string m_LastModified;

    std::string m_Name;

    unsigned int m_Layer;

private:

  // purposely not implemented
  LabelSet& operator=(const LabelSet&);

  itk::LightObject::Pointer InternalClone() const;

};

} // namespace mitk

#endif // _mitkLabelSet_H_
