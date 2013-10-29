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


#ifndef __mitkLabelSet_H_
#define __mitkLabelSet_H_

#include "SegmentationExports.h"
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
class Segmentation_EXPORT LabelSet : public itk::Object
{
public:

    mitkClassMacro( LabelSet, itk::Object );
    itkNewMacro(Self);

    typedef std::vector <Label::Pointer>          LabelContainerType;
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
    void SetLayer(int);

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
    void AddLabel(const Label& label);

    /** \brief
    */
    void AddLabel(const std::string& name, const Color& color);

    /** \brief
    */
    void RenameLabel(int, const std::string&, const Color&);

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
    int GetLabelLayer(int index) const;

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
    void SetLabelColor(int index, const Color &color);

    /** \brief
    */
    const Color& GetLabelColor(int index);

    /** \brief
    */
    const Label* GetActiveLabel() const { return m_ActiveLabel; };

    /** \brief
    */
    Label::ConstPointer GetLabel(int index) const;

    /** \brief
    */
    int GetActiveLabelIndex() const;

    /** \brief
    */
    int GetActiveLabelLayer() const;

    /** \brief
    */
    void SetLabelCenterOfMassIndex(int index, const Point3D& center);

    /** \brief
    */
    const Point3D& GetLabelCenterOfMassIndex(int index);

    /** \brief
    */
    void SetLabelCenterOfMassCoordinates(int index, const Point3D& center);

    /** \brief
    */
    const Point3D& GetLabelCenterOfMassCoordinates(int index);

    /** \brief
    */
    void ResetLabels();

    /** \brief
    */
    int GetLayer() { return m_Layer; }

    /** \brief
    */
    LookupTable* GetLookupTable();

    /** \brief
    */
    void SetLookupTable( LookupTable* lut );

    /** \brief
    */
    static bool IsSelected( Label::Pointer label );

protected:

    LabelSet();
    virtual ~LabelSet();

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    LabelContainerType m_LabelContainer;

    LookupTable::Pointer m_LookupTable;

    Label* m_ActiveLabel;

    int m_Layer;
};

} // namespace mitk

#endif // __mitkLabelSet_H_
