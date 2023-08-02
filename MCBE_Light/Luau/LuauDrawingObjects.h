#pragma once

enum DrawingEnum {
	DE_Anonymous,
	DE_Rectangle
};

int _uid = 0xFF; // doesnt actually mean anything, just a random number

class Drawing_Rectangle {
public:
	Drawing_Rectangle(
		Vector2<float> position,
		Vector2<float> size,
		UIColor colour,
		float alpha = 1,
		int radius = 1,
		bool filled = true
	) {
		this->position = position;
		this->size = size;
		this->colour = colour;
		this->alpha = alpha;
		this->radius = radius;
		this->filled = filled;

		this->visible = true;
		this->uid = _uid++;
	};

public:
	Vector2<float> position;
	Vector2<float> size;
	UIColor colour;

	float alpha;
	int radius;
	bool filled;
	bool visible;
	int uid;
};

std::vector<Drawing_Rectangle*> drawing_rectangles{
	/*new Drawing_Rectangle(
		Vector2<float>(25, 25),
		Vector2<float>(25, 25),
		UIColor(255, 0, 0),
		1,
		1,
		true
	),*/
};