#pragma once

#include "layer.h"

class Screen;

class ModelLayer : public Layer {
public:
	ModelLayer();
	ModelLayer(Screen* screen, char flags = None);

	void init() override;
	void update() override;
	void event(Event& event) override;
	void render() override;
	void close() override;
};