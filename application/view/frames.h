#pragma once

#include "../physics/misc/toString.h"
#include "../graphics/gui/color.h"
#include "../graphics/debug/visualDebug.h"
#include "../graphics/shader/shaderProgram.h"
#include "../graphics/renderUtils.h"
#include "../graphics/texture.h"
#include "../graphics/resource/textureResource.h"
#include "../util/resource/resourceManager.h"
#include "../util/resource/resource.h"
#include "shader/shaders.h"
#include "extendedPart.h"
#include "application.h"
#include "worlds.h"
#include "screen.h"

#include "imgui/imgui.h"

namespace Application {

struct BigFrame {

	// PropertiesFrame
	static float position[3];

	// DebugFrame
	static bool renderSpheres;

	// EnvironmentFrame
	static bool hdr;
	static float gamma;
	static float exposure;
	static Color3 sunColor;

	// LayerFrame
	static bool doEvents;
	static bool noRender;
	static bool doUpdate;
	static bool isDisabled;
	static Layer* selectedLayer;

	// ResourceFrame
	static Resource* selectedResource;

	static void render() {
		ImGui::ShowDemoWindow();
		ImGui::Begin("Inspector");
#pragma region PropertiesFrame
		if (ImGui::CollapsingHeader("Properties")) {
			ImGuiInputTextFlags flags = 0;
			ExtendedPart* sp = screen.selectedPart;

			ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::TreeNode("General")) {
				ImGui::Text("Name: %s", (sp) ? sp->name.c_str() : "-");
				ImGui::Text("Mesh ID: %s", (sp) ? str(sp->visualData.drawMeshId) : "-");

				ImGui::TreePop();
			}

			ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::TreeNode("Physical")) {
				if (sp) {
					// Position
					position[0] = sp->getPosition()[0];
					position[1] = sp->getPosition()[1];
					position[2] = sp->getPosition()[2];
					if (ImGui::InputFloat3("Position: ", position, 3)) {
						GlobalCFrame frame = sp->getCFrame();
						frame.position = Position(position[0], position[1], position[2]);
						sp->setCFrame(frame);
					}
				} else {
					// Position
					position[0] = 0;
					position[1] = 0;
					position[2] = 0;
					ImGui::InputFloat3("Position", position, 3, ImGuiInputTextFlags_ReadOnly);
				}

				ImGui::Text("Velocity: %s", (sp) ? str(sp->getMotion().velocity).c_str() : "-");
				ImGui::Text("Acceleration: %s", (sp) ? str(sp->getMotion().acceleration).c_str() : "-");
				ImGui::Text("Angular velocity: %s", (sp) ? str(sp->getMotion().angularVelocity).c_str() : "-");
				ImGui::Text("Angular acceleration: %s", (sp) ? str(sp->getMotion().angularAcceleration).c_str() : "-");
				ImGui::Text("Kinetic energy: %s", (sp) ? str(sp->parent->getKineticEnergy()).c_str() : "-");
				ImGui::Text("Potential energy: %s", (sp) ? str(screen.world->getPotentialEnergyOfPhysical(*sp->parent->mainPhysical)).c_str() : "-");
				ImGui::Text("Total energy: %s", (sp) ? str(screen.world->getPotentialEnergyOfPhysical(*sp->parent->mainPhysical) + sp->parent->getKineticEnergy()).c_str() : "-");
				ImGui::Text("Mass: %s", (sp) ? str(sp->getMass()).c_str() : "-");
				ImGui::Text("Friction: %s", (sp) ? str(sp->properties.friction).c_str() : "-");
				ImGui::Text("Density: %s", (sp) ? str(sp->properties.density).c_str() : "-");
				ImGui::Text("Bouncyness: %s", (sp) ? str(sp->properties.bouncyness).c_str() : "-");
				ImGui::Text("Conveyor: %s", (sp) ? str(sp->properties.conveyorEffect).c_str() : "-");
				ImGui::Text("Inertia: %s", (sp) ? str(sp->getInertia()).c_str() : "-");

				ImGui::TreePop();
			}

			ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::TreeNode("Material")) {
				if (sp) {
					ImGui::ColorEdit4("Ambient", sp->material.ambient.data);
					ImGui::ColorEdit3("Diffuse", sp->material.diffuse.data);
					ImGui::ColorEdit3("Specular", sp->material.specular.data);
					ImGui::SliderFloat("Reflectance", &sp->material.reflectance, 0, 1);
					if (ImGui::Button(sp ? (sp->renderMode == Renderer::FILL ? "Render mode: fill" : "Render mode: wireframe") : "l"))
						if (sp) sp->renderMode = sp->renderMode == Renderer::FILL ? Renderer::WIREFRAME : Renderer::FILL;
				} else {
					ImGui::Text("No part selected");
				}

				ImGui::TreePop();
			}
		}
#pragma endregion

#pragma region LayerFrame

		if (ImGui::CollapsingHeader("Layers")) {
			ImGui::Columns(2, 0, true);
			ImGui::Separator();
			for (Layer* layer : screen.layerStack) {
				if (ImGui::Selectable(layer->name.c_str(), selectedLayer == layer))
					selectedLayer = layer;
			}

			ImGui::NextColumn();

			ImGui::Text("Name: %s", (selectedLayer) ? selectedLayer->name.c_str() : "-");
			if (selectedLayer) {
				ImGui::SetNextTreeNodeOpen(true);
				if (ImGui::TreeNode("Flags")) {
					char& flags = selectedLayer->flags;

					doEvents = !(flags & Layer::NoEvents);
					doUpdate = !(flags & Layer::NoUpdate);
					noRender = !(flags & Layer::NoRender);
					isDisabled = flags & Layer::Disabled;

					ImGui::Checkbox("Disabled", &isDisabled);
					ImGui::Checkbox("Events", &doEvents);
					ImGui::Checkbox("Update", &doUpdate);
					ImGui::Checkbox("Render", &noRender);

					flags = (isDisabled) ? flags | Layer::Disabled : flags & ~Layer::Disabled;
					flags = (doEvents) ? flags & ~Layer::NoEvents : flags | Layer::NoEvents;
					flags = (doUpdate) ? flags & ~Layer::NoUpdate : flags | Layer::NoUpdate;
					flags = (noRender) ? flags & ~Layer::NoRender : flags | Layer::NoRender;

					ImGui::TreePop();
				}
			}
			
			ImGui::Columns(1);
			ImGui::Separator();
		}

#pragma endregion

#pragma region ResourceFrame

		if (ImGui::CollapsingHeader("Resources")) {
			ImGui::Columns(2, 0, true);
			ImGui::Separator();

			auto map = ResourceManager::getResourceMap();
			for (auto iterator = map.begin(); iterator != map.end(); ++iterator) {
				if (ImGui::TreeNodeEx(iterator->first.c_str(), ImGuiTreeNodeFlags_SpanFullWidth)) {
					for (Resource* resource : iterator->second) {
						if (ImGui::Selectable(resource->getName().c_str(), resource == selectedResource))
							selectedResource = resource;
					}
					ImGui::TreePop();
				}
			}

			ImGui::NextColumn();
			
			if (selectedResource) {
				ImGui::Text("Name: %s", selectedResource->getName().c_str());
				ImGui::Text("Path: %s", selectedResource->getPath().c_str());
				ImGui::Text("Type: %s", selectedResource->getTypeName().c_str());
				switch (selectedResource->getType()) {
					case ResourceType::Texture: {
						TextureResource* texture = static_cast<TextureResource*>(selectedResource);

						ImGui::Text("ID: %d", texture->getID());
						ImGui::Text("Width: %d", texture->getWidth());
						ImGui::Text("Height: %d", texture->getHeight());
						ImGui::Text("Channels: %d", texture->getChannels());
						ImGui::Text("Unit: %d", texture->getUnit());

						if (ImGui::TreeNodeEx("Preview", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen)) {
							float cw = ImGui::GetColumnWidth();
							float ch = ImGui::GetWindowHeight();
							float tw = ch * texture->getAspect();
							float th = cw / texture->getAspect();

							ImVec2 size;
							if (th > ch)
								size = ImVec2(tw, ch);
							else
								size = ImVec2(cw, th);

							Vec4f c = COLOR::ACCENT;
							ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
							ImGui::Image((void*) (intptr_t) texture->getID(), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(c.x, c.y, c.z, c.w));
							
							ImGui::TreePop();
						}
						}

						break;
					default:
						ImGui::Text("Visual respresentation not supported.");
						break;
				}
			} else {
				ImGui::Text("No resource selected.");
			}

			ImGui::Columns(1);
			ImGui::Separator();


		}
#pragma endregion

#pragma region EnvironmentFrame
		if (ImGui::CollapsingHeader("Environment")) {
			if (ImGui::Checkbox("HDR", &hdr))
				ApplicationShaders::basicShader.updateHDR(hdr);

			if (ImGui::SliderFloat("Gamma", &gamma, 0, 3))
				ApplicationShaders::basicShader.updateGamma(gamma);

			if (ImGui::SliderFloat("Exposure", &exposure, 0, 2))
				ApplicationShaders::basicShader.updateExposure(exposure);

			if (ImGui::ColorEdit3("Sun color", sunColor.data))
				ApplicationShaders::basicShader.updateSunColor(sunColor);
		}
#pragma endregion;

#pragma region DebugFrame
		if (ImGui::CollapsingHeader("Debug")) {
			if (ImGui::TreeNode("Vectors")) {
				ImGui::Checkbox("Info", &Debug::vectorDebugEnabled[Debug::INFO_VEC]);
				ImGui::Checkbox("Position", &Debug::vectorDebugEnabled[Debug::POSITION]);
				ImGui::Checkbox("Velocity", &Debug::vectorDebugEnabled[Debug::VELOCITY]);
				ImGui::Checkbox("Acceleration", &Debug::vectorDebugEnabled[Debug::ACCELERATION]);
				ImGui::Checkbox("Moment", &Debug::vectorDebugEnabled[Debug::MOMENT]);
				ImGui::Checkbox("Force", &Debug::vectorDebugEnabled[Debug::FORCE]);
				ImGui::Checkbox("Angular impulse", &Debug::vectorDebugEnabled[Debug::ANGULAR_IMPULSE]);
				ImGui::Checkbox("Impulse", &Debug::vectorDebugEnabled[Debug::IMPULSE]);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Points")) {
				ImGui::Checkbox("Info", &Debug::pointDebugEnabled[Debug::INFO_POINT]);
				ImGui::Checkbox("Center of mass", &Debug::pointDebugEnabled[Debug::CENTER_OF_MASS]);
				ImGui::Checkbox("Intersections", &Debug::pointDebugEnabled[Debug::INTERSECTION]);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Render")) {
				ImGui::Checkbox("Render pies", &Debug::renderPiesEnabled);
				if (ImGui::Button("Switch collision sphere render mode")) Debug::colissionSpheresMode = static_cast<Debug::SphereColissionRenderMode>((static_cast<int>(Debug::colissionSpheresMode) + 1) % 3);

				ImGui::TreePop();
			}
		}
#pragma endregion
		ImGui::End();
	}
};

};