/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrosshairManager_h
#define mitkCrosshairManager_h

#include <MitkCoreExports.h>

#include <mitkBaseRenderer.h>
#include <mitkCrosshairData.h>
#include <mitkDataStorage.h>

namespace mitk
{
  /**
   * \brief The CrosshairManager takes care of the correct settings for the crosshair.
   * 
   * The CrosshairManager keeps track of a crosshair data node,
   * which is used to compute the lines of a crosshair.
   * These crosshair data node can be added to / removed from the data storage
   * using this CrosshairManager. Adding / removing the node to / from the data storage
   * will change the rendering behavior for the crosshair - only data nodes
   * inside the data storage will be rendered.
   */
  class MITKCORE_EXPORT CrosshairManager : public itk::Object
  {
  public:

    mitkClassMacroItkParent(CrosshairManager, itk::Object);
    mitkNewMacro1Param(Self, BaseRenderer*);
    
    /**
    * \brief Set the position of the managed crosshair to the given 3D position.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         set a new position.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \param selectedPoint  The 3D point that will be newly set for the crosshair data.
    */
    void SetCrosshairPosition(const Point3D& selectedPoint);
    /**
    * \brief Update the position of the managed crosshair to the current slice
    *        position of the given slice navigation controller, depending on the view
    *        direction of the slice navigation controller.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         update the crosshair position.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \param sliceNavigationController  The slice navigation controller whose current position
    *                                   will be used for updating the corresponding 3D coordinate
    *                                   of the crosshair data's position.
    */
    void UpdateCrosshairPosition(const SliceNavigationController* sliceNavigationController);
    /**
    * \brief Get the position of the managed crosshair.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         get the current crosshair position.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \return The 3D point that represents the position of the crosshair data.
    */
    Point3D GetCrosshairPosition() const;
    /**
    * \brief Set the visibility of the managed crosshair to the given value.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         set the visibility.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \param visible        A boolean value that will define the visibility of the crosshair.
    * \param baseRenderer   A base renderer for which the renderer-specific visibility property
    *                       should be set.
    */
    void SetCrosshairVisibility(bool visible, const BaseRenderer* baseRenderer);
    /**
    * \brief Get the visibility of the managed crosshair.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         get the visibility.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \param baseRenderer   A base renderer for which the renderer-specific visibility property
    *                       should be get.
    * \return The boolean value that represents the visibility of the crosshair.
    */
    bool GetCrosshairVisibility(const BaseRenderer* baseRenderer) const;
    /**
    * \brief Set the gap size of the managed crosshair to the given value.
    *
    * \pre    The crosshair data of this manager needs to be not null in order to
    *         set the gap size.
    * \throw  mitk::Exception, if the crosshair data of this manager is null.
    *
    * \param gapSize  The size of the gap in the center of the crosshair.
    */
    void SetCrosshairGap(unsigned int gapSize);
    /**
    * \brief Add the managed crosshair data node to the given data storage.
    *
    * \pre    The crosshair data node of this manager needs to be not null in order to
    *         add the node to the data storage.
    * \throw  mitk::Exception, if the crosshair data node of this manager is null.
    * \pre    The given data storage needs to be not null in order to add the node to
    *         the data storage.
    * \throw  mitk::Exception, if the given data storage is null.
    *
    * \param dataStorage  The data storage to which the crosshair data node should be added.
    */
    void AddCrosshairNodeToDataStorage(DataStorage* dataStorage);
    /**
    * \brief Remove the managed crosshair data node from the given data storage.
    *
    * \pre    The crosshair data node of this manager needs to be not null in order to
    *         remove the node from the data storage.
    * \throw  mitk::Exception, if the crosshair data node of this manager is null.
    * \pre    The given data storage needs to be not null in order to remove the node
    *         from the data storage.
    * \throw  mitk::Exception, if the given data storage is null.
    *
    * \param dataStorage  The data storage from which the crosshair data node should be removed.
    */
    void RemoveCrosshairNodeFromDataStorage(DataStorage* dataStorage);

  protected:

    CrosshairManager(BaseRenderer* baseRenderer);
    ~CrosshairManager();

    DataNode::Pointer m_CrosshairDataNode;
    CrosshairData::Pointer m_CrosshairData;

  };
}

#endif
