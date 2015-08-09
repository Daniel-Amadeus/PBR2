#include "PhysicallyBasedRenderingExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/resources/ResourceManager.h>
#include <gloperate/primitives/Scene.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/primitives/Icosahedron.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

EmptyExample::EmptyExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
	setupPropertyGroup();
}

EmptyExample::~EmptyExample() = default;

void EmptyExample::setupPropertyGroup()
{
	
	m_presetProperty = addProperty<Preset>("preset", this,
		&EmptyExample::getPreset,
		&EmptyExample::setPreset);
	m_presetProperty->setStrings({
		{ Preset::manual, "manual" },
		{ Preset::gold, "gold" },
		{ Preset::plastic, "plastic" },
		{ Preset::stone, "stone" },
		{ Preset::tiles, "tiles" } });

	//======= manual settings =======

	m_manualSettingsPropertyGroup = addGroup("manualSettings");

	m_programPresetProperty = m_manualSettingsPropertyGroup->addProperty<ProgramPreset>("ProgramPreset", this,
		&EmptyExample::getProgramPreset,
		&EmptyExample::setProgramPreset);
	m_programPresetProperty->setStrings({
			{ ProgramPreset::withoutEnvMap, "WithoutEnvMap" },
			{ ProgramPreset::withEnvMap, "WithEnvMap" } });

	//======= albedo color =======

	m_albedoPresetProperty = m_manualSettingsPropertyGroup->addProperty<AlbedoPreset>("albedo", this,
		&EmptyExample::albedoPreset,
		&EmptyExample::setAlbedoPreset);
	m_albedoPresetProperty->setStrings({
		{ AlbedoPreset::color, "color" },
		{ AlbedoPreset::metal, "metal" },
		{ AlbedoPreset::plastic, "plastic" },
		{ AlbedoPreset::stone, "stone" },
		{ AlbedoPreset::tiles, "tiles" } });

	m_albedoColorGroup = m_manualSettingsPropertyGroup->addGroup("albedoColor");

	m_albedoColorGroup->addProperty<float>("albedoColorR", this,
		&EmptyExample::albedoColorR, &EmptyExample::setAlbedoColorR)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });
	m_albedoColorGroup->addProperty<float>("albedoColorG", this,
		&EmptyExample::albedoColorG, &EmptyExample::setAlbedoColorG)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });
	m_albedoColorGroup->addProperty<float>("albedoColorB", this,
		&EmptyExample::albedoColorB, &EmptyExample::setAlbedoColorB)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_microsurfaceProperty = m_manualSettingsPropertyGroup->addProperty<float>("microsurface", this,
		&EmptyExample::microsurface, &EmptyExample::setMicrosurface);
	m_microsurfaceProperty->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	//======= reflectivity =======

	m_reflectivityGroup = m_manualSettingsPropertyGroup->addGroup("reflectivity");


	m_reflectivityGroup->addProperty<float>("reflectivityR", this,
		&EmptyExample::reflectivityR, &EmptyExample::setReflectivityR)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_reflectivityGroup->addProperty<float>("reflectivityG", this,
		&EmptyExample::reflectivityG, &EmptyExample::setReflectivityG)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	m_reflectivityGroup->addProperty<float>("reflectivityB", this,
		&EmptyExample::reflectivityB, &EmptyExample::setReflectivityB)->setOptions({
			{ "minimum", 0.0f },
			{ "maximum", 1.0f },
			{ "step", 0.1f },
			{ "precision", 2u } });

	//======= normal map =======

	m_normalMapPresetProperty = m_manualSettingsPropertyGroup->addProperty<NormalMapPreset>("normalMap", this,
		&EmptyExample::normalMapPreset,
		&EmptyExample::setNormalMapPreset);
	m_normalMapPresetProperty->setStrings({
		{ NormalMapPreset::none, "none" },
		{ NormalMapPreset::metal, "metal" },
		{ NormalMapPreset::plastic, "plastic" },
		{ NormalMapPreset::stone, "stone" },
		{ NormalMapPreset::tiles, "tiles" } });

	
}

void EmptyExample::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

AlbedoPreset EmptyExample::albedoPreset() const
{
	return m_albedoPreset;
}
void EmptyExample::setAlbedoPreset(AlbedoPreset preset)
{
	m_albedoPreset = preset;
	m_manualSettingsPropertyGroup->clear();
	switch (m_albedoPreset)
	{
	case AlbedoPreset::color:
		m_material.setAlbedo(m_material.albedoColor());
		setupPropertyGroupColor();
		break;
	case AlbedoPreset::metal:
		m_material.setAlbedo(m_albedoMetal);
		setupPropertyGroupTex();
		break;
	case AlbedoPreset::plastic:
		m_material.setAlbedo(m_albedoPlastic);
		setupPropertyGroupTex();
		break;
	case AlbedoPreset::stone:
		m_material.setAlbedo(m_albedoStone);
		setupPropertyGroupTex();
		break;
	case AlbedoPreset::tiles:
		m_material.setAlbedo(m_albedoTiles);
		setupPropertyGroupTex();
		break;
	default:
		break;
	}
}

float EmptyExample::albedoColorR() const
{
	return m_material.albedoColor().r;
}
void EmptyExample::setAlbedoColorR(float red)
{
	glm::vec3 color = m_material.albedoColor();
	color[0] = red;
	m_material.setAlbedo(color);
}
float EmptyExample::albedoColorG() const
{
	return m_material.albedoColor().g;
}
void EmptyExample::setAlbedoColorG(float green)
{
	glm::vec3 color = m_material.albedoColor();
	color[1] = green;
	m_material.setAlbedo(color);
}
float EmptyExample::albedoColorB() const
{
	return m_material.albedoColor().b;
}
void EmptyExample::setAlbedoColorB(float blue)
{
	glm::vec3 color = m_material.albedoColor();
	color[2] = blue;
	m_material.setAlbedo(color);
}

float EmptyExample::microsurface() const
{
	return m_material.microsurface();
}
void EmptyExample::setMicrosurface(float newMicrosurface)
{
	m_material.setMicrosurface(newMicrosurface);
}

float EmptyExample::reflectivityR() const
{
	return m_material.reflectivity()[0];
}
void EmptyExample::setReflectivityR(float red)
{
	glm::vec3 reflectivity = m_material.reflectivity();
	reflectivity[0] = red;
	m_material.setReflectivity(reflectivity);
}

float EmptyExample::reflectivityG() const
{
	return m_material.reflectivity()[1];
}
void EmptyExample::setReflectivityG(float green)
{
	glm::vec3 reflectivity = m_material.reflectivity();
	reflectivity[1] = green;
	m_material.setReflectivity(reflectivity);
}

float EmptyExample::reflectivityB() const
{
	return m_material.reflectivity()[2];
}
void EmptyExample::setReflectivityB(float blue)
{
	glm::vec3 reflectivity = m_material.reflectivity();
	reflectivity[2] = blue;
	m_material.setReflectivity(reflectivity);
}

NormalMapPreset EmptyExample::normalMapPreset() const
{
	return m_normalMapPreset;
}
void EmptyExample::setNormalMapPreset(NormalMapPreset preset)
{
	m_normalMapPreset = preset;
	switch (m_normalMapPreset)
	{
	case NormalMapPreset::none:
		m_material.setNormalMap(nullptr);
		break;
	case NormalMapPreset::metal:
		m_material.setNormalMap(m_normalMetal);
		break;
	case NormalMapPreset::plastic:
		m_material.setNormalMap(m_normalPlastic);
		break;
	case NormalMapPreset::stone:
		m_material.setNormalMap(m_normalStone);
		break;
	case NormalMapPreset::tiles:
		m_material.setNormalMap(m_normalTiles);
		break;
	default:
		break;
	}
}

ProgramPreset EmptyExample::getProgramPreset() const
{
	return m_material.programPreset();
}
void EmptyExample::setProgramPreset(ProgramPreset programPreset)
{
	m_material.setProgramByPreset(programPreset);
}

Preset EmptyExample::getPreset() const
{
	return m_preset;
}
void EmptyExample::setPreset(Preset newPreset)
{
	m_preset = newPreset;
	clear();
	addProperty(m_presetProperty);
	PBRMaterial newMaterial = PBRMaterial();
	switch (newPreset)
	{
	case Preset::manual:
		newMaterial.copySettings(m_material);
		m_material = newMaterial;
		addProperty(m_manualSettingsPropertyGroup);
		break;
	case Preset::gold:
		m_material = m_gold;
		break;
	case Preset::plastic:
		m_material = m_plastic;
		break;
	case Preset::stone:
		m_material = m_stone;
		break;
	case Preset::tiles:
		m_material = m_tiles;
		break;
	default:
		break;
	}
}

void EmptyExample::onInitialize()
{
    // create program

    globjects::init();

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({0.6f, 0.6f, 0.6f});

	m_icosahedron = new gloperate::Icosahedron{ 3 };

	m_programWithEnv = new Program{};
	m_programWithEnv->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_env.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_env.frag")
		);

	m_programNoEnv = new Program{};
	m_programNoEnv->attach(
		Shader::fromFile(GL_VERTEX_SHADER, "data/emptyexample/icosahedron_noEnv.vert"),
		Shader::fromFile(GL_FRAGMENT_SHADER, "data/emptyexample/icosahedron_noEnv.frag")
		);

	m_transformLocation = m_programNoEnv->getUniformLocation("transform");
	m_eyeLocation = m_programNoEnv->getUniformLocation("a_eye");
	//m_programPreset = ProgramPreset::withEnvMap;

	//std::cout << "m_eyeLocation = " << m_eyeLocation << std::endl;

	m_icoTransform = glm::mat4x4();
	m_icoTransform = glm::translate(m_icoTransform, glm::vec3(0.0f, 1.0f, 0.0f));

	m_cameraCapability->setEye(glm::vec3(-1.2f, 2.1f, -2.8));
	m_cameraCapability->setCenter(glm::vec3(0.9f, 0.5f, 2.0));

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    setupProjection();


	m_albedoMetal = m_resourceManager.load<globjects::Texture>("data/emptyexample/metal/MetalScratches_COLOR.png");
	m_albedoMetal->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoMetal->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_albedoPlastic = m_resourceManager.load<globjects::Texture>("data/emptyexample/plastic/Plastic_COLOR.png");
	m_albedoPlastic->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoPlastic->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_albedoStone = m_resourceManager.load<globjects::Texture>("data/emptyexample/stone/Stone_COLOR.png");
	m_albedoStone->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoStone->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_albedoTiles = m_resourceManager.load<globjects::Texture>("data/emptyexample/tiles/TilesPlain_COLOR.png");
	m_albedoTiles->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoTiles->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);


	m_normalMetal = m_resourceManager.load<globjects::Texture>("data/emptyexample/metal/MetalScratches_NRM.png");
	m_normalMetal->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normalMetal->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_normalPlastic = m_resourceManager.load<globjects::Texture>("data/emptyexample/plastic/Plastic_NRM.png");
	m_normalPlastic->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normalPlastic->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_normalStone = m_resourceManager.load<globjects::Texture>("data/emptyexample/stone/Stone_NRM.png");
	m_normalStone->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normalStone->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

	m_normalTiles = m_resourceManager.load<globjects::Texture>("data/emptyexample/tiles/TilesPlain_NRM.png");
	m_normalTiles->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normalTiles->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	m_envmap = m_resourceManager.load<globjects::Texture>("data/emptyexample/Panorama_0.png");
	
	setMicrosurface(0.86f); //gold

	m_albedoPreset = AlbedoPreset::color;
	m_normalMapPreset = NormalMapPreset::metal;

	m_gold = PBRMaterial(ProgramPreset::withEnvMap, m_envmap, m_normalMetal, glm::vec3(0.0f, 0.0f, 0.0f), 0.86f, glm::vec3(1.0f, 0.86f, 0.58f));
	m_plastic = PBRMaterial(ProgramPreset::withEnvMap, m_envmap, m_normalPlastic, m_albedoPlastic, 0.46f, glm::vec3(0.34f, 0.41f, 0.53f));
	m_stone = PBRMaterial(ProgramPreset::withoutEnvMap, m_envmap, m_normalStone, m_albedoStone, 0.29f, glm::vec3(0.24f, 0.24f, 0.25f));
	m_tiles = PBRMaterial(ProgramPreset::withEnvMap, m_envmap, m_normalTiles, m_albedoTiles, 0.36f, glm::vec3(0.47f, 0.49f, 0.51f));
	
	setPreset(Preset::gold);
}

void EmptyExample::onPaint()
{
	globjects::ref_ptr<globjects::Program> program;
	
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
    }

    auto fbo = m_targetFramebufferCapability->framebuffer();

    if (!fbo)
        fbo = globjects::Framebuffer::defaultFBO();

    fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
	
	const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_grid->update(eye, transform);
    m_grid->draw();

	switch (m_material.programPreset())
	{
	case ProgramPreset::withoutEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withoutEnvMap);
		break;
	case ProgramPreset::withEnvMap:
		m_material.setProgramByPreset(ProgramPreset::withEnvMap);
		break;
	default:
		break;
	}
	program = m_material.program();

	program->use();
	program->setUniform("projection", transform);
	program->setUniform("transform", m_icoTransform);
	program->setUniform(m_eyeLocation, m_cameraCapability->eye());

	m_material.use();

	m_icosahedron->draw();

	program->release();

	m_material.release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}

void EmptyExample::setupPropertyGroupColor()
{
	m_manualSettingsPropertyGroup->addProperty(m_programPresetProperty);
	m_manualSettingsPropertyGroup->addProperty(m_albedoPresetProperty);
	m_manualSettingsPropertyGroup->addProperty(m_albedoColorGroup);
	m_manualSettingsPropertyGroup->addProperty(m_microsurfaceProperty);
	m_manualSettingsPropertyGroup->addProperty(m_reflectivityGroup);
	m_manualSettingsPropertyGroup->addProperty(m_normalMapPresetProperty);
}

void EmptyExample::setupPropertyGroupTex()
{
	m_manualSettingsPropertyGroup->addProperty(m_programPresetProperty);
	m_manualSettingsPropertyGroup->addProperty(m_albedoPresetProperty);
	m_manualSettingsPropertyGroup->addProperty(m_microsurfaceProperty);
	m_manualSettingsPropertyGroup->addProperty(m_reflectivityGroup);
	m_manualSettingsPropertyGroup->addProperty(m_normalMapPresetProperty);
}