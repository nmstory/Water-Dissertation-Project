#pragma once
class WaterTile {
public:
	float TILE_SIZE = 60;

	WaterTile(float centerX, float centerZ, float height) {
		this->x = centerX;
		this->z = centerZ;
		this->height = height;
	}

	float getHeight() {
		return height;
	}

	float getX() {
		return x;
	}

	float getZ() {
		return z;
	}

protected:
	float height;
	float x, z;
};