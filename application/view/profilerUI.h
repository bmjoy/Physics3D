#pragma once

#include <string>
#include "../../engine/profiling.h"
#include "screen.h"
#include "bindable.h"
#include "font.h"

extern const Vec3f pieColors[30];

struct DataPoint {
	float weight;
	std::string value;
	Vec3f color;
	const char* label;
	DataPoint() : color(), weight(0) {}
	DataPoint(float weight, std::string value, Vec3f color, const char* label) : weight(weight), value(value), color(color), label(label) {}
};
struct PieChart {
	const char* title;
	Vec2f piePosition;
	float pieSize;
	std::vector<DataPoint> parts;
	std::string totalValue;

	inline PieChart(const char* title, std::string totalValue, Vec2f piePosition, float pieSize) : title(title), totalValue(totalValue), piePosition(piePosition), pieSize(pieSize) {}
	void renderPie(Screen& screen) const;
	void renderText(Screen& screen, Font* font) const;
	void add(DataPoint& p);
	float getTotal() const;
};
struct BarChart {
	const char* title;
	Vec2f piePosition;
	float pieSize;
	std::vector<DataPoint> parts;
	std::string totalValue;

	inline BarChart(const char* title, std::string totalValue, Vec2f piePosition, float pieSize) : title(title), totalValue(totalValue), piePosition(piePosition), pieSize(pieSize) {}
	void renderBars(Screen& screen) const;
	void renderText(Screen& screen, Font* font) const;
	void add(DataPoint& p);
	float getTotal() const;
};
void startPieRendering(Screen& screen);
void endPieRendering(Screen& screen);