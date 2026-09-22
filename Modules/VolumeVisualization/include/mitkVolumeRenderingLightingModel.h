/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeRenderingLightingModel_h
#define mitkVolumeRenderingLightingModel_h

#include <MitkVolumeVisualizationExports.h>

#include <mitkVtkPropRenderer.h>

#include <string>
#include <vector>

namespace mitk
{
  class DataNode;

  /**
   * \brief One fixed point in VTK's volume lighting parameter space, covering
   *        both the light rig installed on the renderer and the material and
   *        scattering properties written to the node.
   *
   * The models are not weak and strong versions of one effect: they pick which
   * of the ray caster's two shading paths the render takes, and everything else
   * follows from that.
   *
   * Headlight is the only rig for which the ray caster compiles its default
   * lighting path, which requires exactly one switched-on light, at intensity
   * exactly 1.0, of headlight type. That path is the one where ambient is
   * multiplied by the sample colour. The cost is that a light at the camera
   * illuminates precisely what the camera sees, so it casts no visible shadow
   * and scattering would have nothing to darken - hence no scattering here.
   *
   * Key light gives that path up to put a light off-axis, which is what lets a
   * shadow ray describe shape. The remaining fields are what that costs.
   *
   * Default lighting is the renderer's own five-light kit, carried here so that
   * the look of an unconfigured node has a name that can be chosen back. It
   * takes the multi-light path like the key rig but carries no scattering:
   * five lights refill each other's shadows, so a shadow ray buys nothing
   * there and costs five times as much.
   *
   * Blend mixes Phong out and a scattering model in rather than adding
   * occlusion on top of it: finalColor = (1 - c) * phong + c * scattering,
   * where c cannot exceed blend. So the value reads as the largest share of
   * the shading a shadow ray may account for. Keep it under 1.0; above that,
   * scattering can replace Phong outright and takes the specular highlight
   * with it.
   *
   * Reach bounds the shadow ray and is where essentially all the cost lives.
   * It is steeply non-linear: the traced fraction of the volume diagonal is
   * 1 - (1 - reach)^0.33, so 0.12 traces about 4% and 0.50 about 20%. Short is
   * deliberate. A long ray lets one structure cast onto another, which
   * describes the lighting rather than the patient; a short one darkens
   * contacts and recesses, which is the depth cue worth having. What short
   * rays cost is grain, and that cannot be bought off by sampling more finely:
   * vtkSmartVolumeMapper locks the sample distance to the input spacing and
   * discards any other value.
   *
   * Ambient is the single field the models disagree on, and not by preference -
   * the shader computes it differently for each. Under the headlight it carries
   * the sample colour and works as a fill. Under any rig with more than one
   * light it does not, so it lays a flat grey over the whole image and
   * desaturates the render long before it rescues an occluded voxel. The key
   * rig therefore holds it at zero and fills from its fill light instead, whose
   * contribution does carry the sample colour and does get its own shadow ray.
   *
   * Diffuse rises to make up for that zero, which the key rig can afford
   * because its key and fill together exceed a single light's intensity.
   *
   * Specular is written by every model rather than inherited. The shader adds
   * it without the sample colour, so it is white light laid over the render,
   * and a light near the camera puts its lobe across everything visible at
   * once; high values clip bright tissue to white.
   *
   * Anisotropy stays at 0. VTK's Henyey-Greenstein phase function carries no
   * 1/4pi normalisation, so it is exactly 1.0 at zero but swings either side
   * of that with the light and view geometry - a brightness change, not a
   * shape cue.
   */
  struct MITKVOLUMEVISUALIZATION_EXPORT VolumeRenderingLightingModel
  {
    /**
     * \brief Stable identifier, recorded on the node by ApplyTo.
     *
     * Kept separate from the label so that reordering the models or renaming
     * one cannot change what an already-saved scene means.
     */
    std::string id;

    /** \brief Name for a control that selects the model. */
    std::string label;

    float blend;
    float reach;
    float anisotropy;
    bool normalsFromOpacity;
    float ambient;
    float diffuse;
    float specular;
    float specularPower;

    /**
     * \brief The light rig this model needs on the 3D renderer.
     *
     * Deliberately not applied by ApplyTo: lights belong to the renderer, not
     * to the node, and reaching one needs a render window that only the caller
     * has. A caller that ignores this gets the node's scattering applied
     * against whatever rig happens to be installed, which for the default
     * five-light kit is both muddy and about five times the cost.
     */
    VtkPropRenderer::LightingMode lightingMode;

    /**
     * \brief Write this model's material and scattering properties onto the
     *        node, and record the model on it.
     *
     * The values are a starting point the user is free to move afterwards
     * without leaving the model, which is why \c id is recorded rather than
     * left to be inferred from them: inferring would both deselect the model
     * when a single value changes and leave two models that happen to write
     * the same values indistinguishable.
     *
     * \param[in] node The node to configure.
     * \throws mitk::Exception if \p node is nullptr.
     */
    void ApplyTo(DataNode *node) const;

    /** \brief The models on offer, in the order they should be presented. */
    static const std::vector<VolumeRenderingLightingModel> &GetAllModels();

    /**
     * \brief The model a node records having had applied.
     * \param[in] node The node to inspect; nullptr is allowed.
     * \return The model, or nullptr when the node names none - an older scene,
     *         or a volume configured outside a view that records it. Callers
     *         should leave the renderer's default rig in place then, rather
     *         than guess which directional one was meant.
     */
    static const VolumeRenderingLightingModel *FromNode(const DataNode *node);

    /**
     * \brief The model with the given identifier.
     * \return The model, or nullptr for an identifier no model uses.
     */
    static const VolumeRenderingLightingModel *FromId(const std::string &id);

    /**
     * \brief The model tuned for the given light rig.
     *
     * The rig a window is on is what a volume entering it has to be shaded for,
     * so this is the direction the rig-first controls ask in.
     *
     * \return The model, or nullptr for a rig no model is tuned for.
     */
    static const VolumeRenderingLightingModel *FromLightingMode(VtkPropRenderer::LightingMode mode);

    /**
     * \brief Key under which ApplyTo records the model's identifier.
     *
     * Public because callers read it back to tell which model a node is on.
     * The mapper does not read it; it describes a choice the mapper properties
     * alone cannot express.
     */
    static constexpr const char *MODEL_PROPERTY = "volumerendering.lightingmodel";
  };
}

#endif
