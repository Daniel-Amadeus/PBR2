#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/Texture.h>

#include <gloperate/painter/Painter.h>

#include <glm/mat4x4.hpp>

#include <PBRMaterial.h>

enum class Preset { manual, gold, plastic, stone, tiles };
enum class AlbedoPreset { color, metal, plastic, stone, tiles};
enum class NormalMapPreset { none, metal, plastic, stone, tiles };
enum class EnvMapPreset { none, indoor };


namespace globjects
{
    class Program;
}

namespace gloperate
{
	class AdaptiveGrid;
	class ResourceManager;
    class Icosahedron;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}


class EmptyExample : public gloperate::Painter
{
public:
    EmptyExample(gloperate::ResourceManager & resourceManager);
    virtual ~EmptyExample();

	void EmptyExample::setupPropertyGroup();

    void setupProjection();

	AlbedoPreset albedoPreset() const;
	void setAlbedoPreset(AlbedoPreset preset);

	float albedoColorR() const;
	void setAlbedoColorR(float red);
	float albedoColorG() const;
	void setAlbedoColorG(float green);
	float albedoColorB() const;
	void setAlbedoColorB(float blue); 

	float microsurface() const;
	void setMicrosurface(float newMicrosurface);

	float reflectivityR() const;
	void setReflectivityR(float red);
	float reflectivityG() const;
	void setReflectivityG(float green);
	float reflectivityB() const;
	void setReflectivityB(float blue);

	NormalMapPreset normalMapPreset() const;
	void setNormalMapPreset(NormalMapPreset preset);

	ProgramPreset getProgramPreset() const;
	void setProgramPreset(ProgramPreset newShader);

	Preset getPreset() const;
	void setPreset(Preset newPreset);

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

private:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
	globjects::ref_ptr<gloperate::Icosahedron> m_icosahedron;
	
	globjects::ref_ptr<globjects::Program> m_programNoEnv;
	globjects::ref_ptr<globjects::Program> m_programWithEnv;
	gl::GLint m_transformLocation;
	gl::GLint m_eyeLocation;
	glm::mat4x4 m_icoTransform;

	void setupPropertyGroupColor();
	void setupPropertyGroupTex();

	PropertyGroup *m_manualSettingsPropertyGroup;
	reflectionzeug::Property<Preset> *m_presetProperty; 

	reflectionzeug::Property<ProgramPreset> *m_programPresetProperty;
	reflectionzeug::Property<AlbedoPreset> *m_albedoPresetProperty;

	PropertyGroup *m_albedoColorGroup;

	reflectionzeug::Property<float> *m_microsurfaceProperty;

	PropertyGroup *m_reflectivityGroup;

	reflectionzeug::Property<NormalMapPreset> *m_normalMapPresetProperty;
	
	globjects::Texture *m_albedoMetal;
	globjects::Texture *m_albedoPlastic;
	globjects::Texture *m_albedoStone;
	globjects::Texture *m_albedoTiles;

	globjects::Texture *m_normalMetal;
	globjects::Texture *m_normalPlastic;
	globjects::Texture *m_normalStone;
	globjects::Texture *m_normalTiles;

	globjects::Texture *m_envmap;

	Preset m_preset;
	AlbedoPreset m_albedoPreset;
	NormalMapPreset m_normalMapPreset;

	PBRMaterial m_material;
	PBRMaterial m_gold;
	PBRMaterial m_plastic;
	PBRMaterial m_stone;
	PBRMaterial m_tiles;
};
