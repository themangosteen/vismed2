#include "volume.h"

#include <math.h>


//-------------------------------------------------------------------------------------------------
// Voxel
//-------------------------------------------------------------------------------------------------

Voxel::Voxel()
{
	setValue(0.0f);
}

Voxel::Voxel(const Voxel &other)
{
	setValue(other.getValue());
}

Voxel::Voxel(const float value)
{
	setValue(value);
}

Voxel::~Voxel()
{
}

void Voxel::setValue(const float value)
{
	this->value = value;
}

const float Voxel::getValue() const
{
	return value;
};

const bool Voxel::operator==(const Voxel &other) const
{
	return (getValue() == other.getValue());
};

const bool Voxel::operator!=(const Voxel &other) const
{
	return !(*this == other);
};

const bool Voxel::operator>(const Voxel &other) const
{
	return getValue() > other.getValue();
};

const bool Voxel::operator>=(const Voxel &other) const
{
	return getValue() >= other.getValue();
};

const bool Voxel::operator<(const Voxel &other) const
{
	return getValue() < other.getValue();
};

const bool Voxel::operator<=(const Voxel &other) const
{
	return getValue() <= other.getValue();
};

const Voxel& Voxel::operator+=(const Voxel &other)
{
	value += other.value;
	return *this;
};

const Voxel& Voxel::operator-=(const Voxel &other)
{
	value -= other.value;
	return *this;
};

const Voxel& Voxel::operator*=(const float &value)
{
	this->value *= value;
	return *this;
};

const Voxel& Voxel::operator/=(const float &value)
{
	this->value /= value;
	return *this;
};

const Voxel Voxel::operator+(const Voxel &other) const
{
	Voxel voxNew = *this;
	voxNew += other;
	return voxNew;
};

const Voxel Voxel::operator-(const Voxel &other) const
{
	Voxel voxNew = *this;
	voxNew -= other;
	return voxNew;
};

const Voxel Voxel::operator*(const float &value) const
{
	Voxel voxNew = *this;
	voxNew *= value;
	return voxNew;
};

const Voxel Voxel::operator/(const float &value) const
{
	Voxel voxNew = *this;
	voxNew /= value;
	return voxNew;
};


//-------------------------------------------------------------------------------------------------
// Volume
//-------------------------------------------------------------------------------------------------

Volume::Volume()
	: voxels(1), width(1), height(1), depth(1), size(0)
{
}

Volume::~Volume()
{
}

const Voxel& Volume::getVoxel(const int x, const int y, const int z) const
{
	return voxels[x + y*width + z*width*height];
}

const Voxel& Volume::getVoxel(const int i) const
{
	return voxels[i];
}

float Volume::valueAt(const int x, const int y, const int z) const
{
	if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= depth)
		return 0;

	return voxels[x + y*width + z*width*height].getValue();
}

const Voxel* Volume::getVoxels() const
{
	return &(voxels.front());
};

const int Volume::getWidth() const
{
	return width;
};

const int Volume::getHeight() const
{
	return height;
};

const int Volume::getDepth() const
{
	return depth;
};

const int Volume::getSize() const
{
	return size;
};



//-------------------------------------------------------------------------------------------------
// Volume File Loader
//-------------------------------------------------------------------------------------------------

bool Volume::loadFromFileDAT(QString filepath, QProgressBar* progressBar)
{
	// open file
	FILE *fp = fopen(filepath.toStdString().c_str(), "rb");
	if (!fp) {
		std::cerr << "Error opening file: " << filepath.toStdString() << std::endl;
		return false;
	}

	progressBar->setRange(0, size + 10);
	progressBar->setValue(0);

	// READ HEADER AND SET VOLUME DIMENSIONS

	// header format: 16 bit width, 16 bit height, 16 bit depth, 16 bit bitsPerVoxel
	// then voxel data with bitsPerVoxel intensity resolution

	unsigned short uWidth, uHeight, uDepth, uBitsPerVoxel;
	fread(&uWidth, sizeof(unsigned short), 1, fp);
	fread(&uHeight, sizeof(unsigned short), 1, fp);
	fread(&uDepth, sizeof(unsigned short), 1, fp);
	fread(&uBitsPerVoxel, sizeof(unsigned short), 1, fp);
	
	width = int(uWidth);
	height = int(uHeight);
	depth = int(uDepth);
	bitsPerVoxel = int(uBitsPerVoxel);

	// check dataset dimensions
	if (
	    width  <= 0 || width  > 1000 ||
		height <= 0 || height > 1000 ||
	    depth  <= 0 || depth  > 1000)
	{
		std::cerr << "Error loading file. Invalid volume dimensions: " << filepath.toStdString() << std::endl;
		return false;
	}

	// compute dimensions
	int slice = width * height;
	size = slice * depth;
	voxels.resize(size);

	// READ VOLUME DATA

	// read into vector before writing data into volume to speed up process
	// TODO unify this into using only bytes or shorts in all cases
	std::vector<unsigned short> vecDataShorts;
	std::vector<unsigned char> vecDataBytes;
	if (bitsPerVoxel <= 8) {
		vecDataBytes.resize(size);
		fread((void*)&(vecDataBytes.front()), sizeof(unsigned char), size, fp);
	}
	else {
		vecDataShorts.resize(size);
		fread((void*)&(vecDataShorts.front()), sizeof(unsigned short), size, fp);
	}
	fclose(fp);

	progressBar->setValue(10);

	// store volume data
	for (int i = 0; i < size; ++i)
	{
		// intensities are converted to float, mapping range [0, 2^bitsPerVoxel] to [0.0, 1.0]
		float value;
		if (bitsPerVoxel <= 8)
			value = fmax(0.0f, fmin(1.0f, float(vecDataBytes[i]) / (1 << bitsPerVoxel)));
		else
			value = fmax(0.0f, fmin(1.0f, float(vecDataShorts[i]) / (1 << bitsPerVoxel)));

		voxels[i] = Voxel(value);
		
		progressBar->setValue(10 + i);
	}

	progressBar->setValue(0);

	std::cout << "Loaded " << bitsPerVoxel << "-bit VOLUME with dimensions " << width << " x " << height << " x " << depth << std::endl;

	return true;
}

