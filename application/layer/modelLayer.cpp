#include "core.h"

#include "modelLayer.h"

#include "view/screen.h"
#include "shader/shaders.h"
#include "extendedPart.h"
#include "worlds.h"

#include "ecs/light.h"
#include "ecs/material.h"
#include "ecs/model.h"
#include "ecs/mesh.h"

#include "../engine/meshRegistry.h"

#include "../graphics/mesh/indexedMesh.h"
#include "../graphics/meshLibrary.h"
#include "../graphics/debug/visualDebug.h"

#include "../graphics/path/path3D.h"
#include "../graphics/batch/batch.h"
#include "../graphics/batch/batchConfig.h"
#include "../graphics/resource/textureResource.h"
#include "../graphics/gui/color.h"

#include "../physics/math/linalg/vec.h"
#include "../physics/sharedLockGuard.h"
#include "../physics/misc/filters/visibilityFilter.h"

#include "../util/resource/resourceManager.h"

namespace Application {

// Light uniforms
std::vector<Light*> lights;
Vec3f sunDirection;
Vec3f sunColor;

enum class RelationToSelectedPart {
	NONE,
	SELF,
	DIRECT_ATTACH,
	MAINPART,
	PHYSICAL_ATTACH,
	MAINPHYSICAL_ATTACH
};

static RelationToSelectedPart getRelationToSelectedPart(const Part* selectedPart, const Part* testPart) {
	if (selectedPart == nullptr)
		return RelationToSelectedPart::NONE;

	if (testPart == selectedPart)
		return RelationToSelectedPart::SELF;

	if (selectedPart->parent != nullptr && testPart->parent != nullptr) {
		if (testPart->parent == selectedPart->parent) {
			if (testPart->isMainPart())
				return RelationToSelectedPart::MAINPART;
			else
				return RelationToSelectedPart::DIRECT_ATTACH;
		} else if (testPart->parent->mainPhysical == selectedPart->parent->mainPhysical) {
			if (testPart->parent->isMainPhysical())
				return RelationToSelectedPart::MAINPHYSICAL_ATTACH;
			else
				return RelationToSelectedPart::PHYSICAL_ATTACH;
		}
	}

	return RelationToSelectedPart::NONE;
}

static Color getAmbientForPartForSelected(Screen* screen, Part* part) {
	switch (getRelationToSelectedPart(screen->selectedPart, part)) {
		case RelationToSelectedPart::NONE:
			return Color(0.0f, 0, 0, 0);
		case RelationToSelectedPart::SELF:
			return Color(0.5f, 0, 0, 0);
		case RelationToSelectedPart::DIRECT_ATTACH:
			return Color(0, 0.25f, 0, 0);
		case RelationToSelectedPart::MAINPART:
			return Color(0, 1.0f, 0, 0);
		case RelationToSelectedPart::PHYSICAL_ATTACH:
			return Color(0, 0, 0.25f, 0);
		case RelationToSelectedPart::MAINPHYSICAL_ATTACH:
			return Color(0, 0, 1.0f, 0);
	}

	return Color(0, 0, 0, 0);
}

static Color getAlbedoForPart(Screen* screen, Part* part) {
	Color computedAmbient = getAmbientForPartForSelected(screen, part);
	if (part == screen->intersectedPart)
		computedAmbient += Vec4f(-0.1f, -0.1f, -0.1f, 0);

	return computedAmbient;
}

void ModelLayer::onInit() {
	using namespace Graphics;
	Screen* screen = static_cast<Screen*>(this->ptr);

	Attenuation attenuation = { 1, 1, 1 };
	lights.push_back(new Light(Vec3f(10, 5, -10), Color3(1, 0.84f, 0.69f), 300, attenuation));
	lights.push_back(new Light(Vec3f(10, 5, 10), Color3(1, 0.84f, 0.69f), 300, attenuation));
	lights.push_back(new Light(Vec3f(-10, 5, -10), Color3(1, 0.84f, 0.69f), 200, attenuation));
	lights.push_back(new Light(Vec3f(-10, 5, 10), Color3(1, 0.84f, 0.69f), 500, attenuation));
	lights.push_back(new Light(Vec3f(0, 5, 0), Color3(1, 0.90f, 0.75f), 400, attenuation));

	ApplicationShaders::basicShader.updateLight(lights);
	ApplicationShaders::instanceShader.updateLight(lights);
	ApplicationShaders::instanceShader.updateTexture(false);
}

void ModelLayer::onUpdate() {

}

void ModelLayer::onEvent(Engine::Event& event) {

}

void ModelLayer::onRender() {
	using namespace Graphics;
	using namespace Graphics::Renderer;
	Screen* screen = static_cast<Screen*>(this->ptr);

	beginScene();

	graphicsMeasure.mark(GraphicsProcess::UPDATE);
	ApplicationShaders::basicShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);
	ApplicationShaders::instanceShader.updateProjection(screen->camera.viewMatrix, screen->camera.projectionMatrix, screen->camera.cframe.position);

	// Filter on mesh ID and transparency
	size_t maxMeshCount = 0;
	std::map<int, size_t> meshCounter;
	std::multimap<int, ExtendedPart*> visibleParts;
	std::map<double, ExtendedPart*> transparentParts;
	graphicsMeasure.mark(GraphicsProcess::PHYSICALS);
	screen->world->syncReadOnlyOperation([this, &visibleParts, &transparentParts, &meshCounter, &maxMeshCount, screen] () {
		VisibilityFilter filter = VisibilityFilter::forWindow(screen->camera.cframe.position, screen->camera.getForwardDirection(), screen->camera.getUpDirection(), screen->camera.fov, screen->camera.aspect, screen->camera.zfar);
		for (ExtendedPart& part : screen->world->iterPartsFiltered(filter, ALL_PARTS)) {
			if (part.material.albedo.w < 1) {
				transparentParts.insert({ lengthSquared(Vec3(screen->camera.cframe.position - part.getPosition())), &part });
			} else {
				visibleParts.insert({ part.visualData.drawMeshId, &part });
				maxMeshCount = fmax(maxMeshCount, meshCounter[part.visualData.drawMeshId]++);
				;
				if (meshCounter[part.visualData.drawMeshId] > maxMeshCount)
					maxMeshCount = meshCounter[part.visualData.drawMeshId];
			}
		}
		});

	// Ensure correct size
	uniforms.reserve(maxMeshCount);
	while (uniforms.size() < maxMeshCount)
		uniforms.push_back(Uniform());

	// Render normal meshes
	ApplicationShaders::instanceShader.bind();
	for (auto iterator : meshCounter) {
		int meshID = iterator.first;
		int meshCount = iterator.second;

		if (meshID == -1) continue;

		// Collect uniforms
		int offset = 0;
		auto meshes = visibleParts.equal_range(meshID);
		for (auto mesh = meshes.first; mesh != meshes.second; ++mesh) {
			ExtendedPart* part = mesh->second;
			Material material = part->material;
			material.albedo += getAlbedoForPart(screen, part);

			Mat4f modelMatrix = Mat4f(Mat3f(part->getCFrame().getRotation().asRotationMatrix()) * DiagonalMat3f(part->hitbox.scale), Vec3f(part->getCFrame().getPosition() - Position(0, 0, 0)), Vec3f(0.0f, 0.0f, 0.0f), 1.0f);

			uniforms[offset] = Uniform {
				modelMatrix,
				part->material.albedo,
				part->material.metalness,
				part->material.roughness,
				part->material.ao
			};

			offset++;
		}
		
		Engine::MeshRegistry::meshes[meshID]->fillUniformBuffer(uniforms.data(), meshCount * sizeof(Uniform), Renderer::STREAM_DRAW);
		Engine::MeshRegistry::meshes[meshID]->renderInstanced(meshCount);
	}

	// Render transparent meshes
	ApplicationShaders::basicShader.bind();
	Renderer::enableBlending();
	for (auto iterator = transparentParts.rbegin(); iterator != transparentParts.rend(); ++iterator) {
		ExtendedPart* part = (*iterator).second;

		Material material = part->material;
		material.albedo += getAlbedoForPart(screen, part);

		if (part->visualData.drawMeshId == -1)
			continue;

		ApplicationShaders::basicShader.updateMaterial(material);
		ApplicationShaders::basicShader.updatePart(*part);
		Engine::MeshRegistry::meshes[part->visualData.drawMeshId]->render(part->renderMode);
	}

	endScene();
}

void ModelLayer::onClose() {

}

};