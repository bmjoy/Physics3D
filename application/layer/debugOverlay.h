#pragma once

#include "../engine/layer/layer.h"

namespace Application {

class Screen;

class DebugOverlay : public Layer {
public:
	inline DebugOverlay() : Layer() {};
	inline DebugOverlay(Screen* screen, char flags = NoEvents) : Layer("Debug overlay", screen, flags) {};

	virtual void onInit() override;
	virtual void onUpdate() override;
	virtual void onEvent(Engine::Event& event) override;
	virtual void onRender() override;
	virtual void onClose() override;
};

};