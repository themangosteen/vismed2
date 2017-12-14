#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstdio>

#include <QProgressBar>


//-------------------------------------------------------------------------------------------------
// Voxel
//-------------------------------------------------------------------------------------------------

class Voxel
{
public:

	Voxel();
	Voxel(const Voxel &other);
	Voxel(const float value);

	~Voxel();


	// VOXEL VALUE

	void					setValue(const float value);
	const float				getValue() const;


	// OPERATORS

	const bool				operator==(const Voxel &other) const;
	const bool				operator!=(const Voxel &other) const;
	const bool				operator>(const Voxel &other) const;
	const bool				operator>=(const Voxel &other) const;
	const bool				operator<(const Voxel &other) const;
	const bool				operator<=(const Voxel &other) const;

	const Voxel				operator+(const Voxel &other) const;
	const Voxel				operator-(const Voxel &other) const;
	const Voxel				operator*(const float &value) const;
	const Voxel				operator/(const float &value) const;

	const Voxel&			operator+=(const Voxel &other);
	const Voxel&			operator-=(const Voxel &other);
	const Voxel&			operator*=(const float &value);
	const Voxel&			operator/=(const float &value);


private:

	float					value;

};


//-------------------------------------------------------------------------------------------------
// Volume
//-------------------------------------------------------------------------------------------------

class Volume
{

public:

	Volume();
	~Volume();


	// VOLUME DATA

	const Voxel& getVoxel(const int i) const;
	const Voxel& getVoxel(const int x, const int y, const int z) const;
	float valueAt(const int x, const int y, const int z) const;
	const Voxel* getVoxels() const;

	const int getWidth() const;
	const int getHeight() const;
	const int getDepth() const;

	const int getSize() const;

	bool loadFromFile(QString filename, QProgressBar* progressBar);

private:

	std::vector<Voxel> voxels;

	int width;
	int height;
	int depth;

	int size;

};
