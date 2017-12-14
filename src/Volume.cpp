#include "Volume.h"

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

bool Volume::loadFromFile(QString filename, QProgressBar* progressBar)
{
	// load file
	FILE *fp = fopen(filename.toStdString().c_str(), "rb");
	if (!fp)
	{
		std::cerr << "+ Error loading file: " << filename.toStdString() << std::endl;
		return false;
	}

	// progress bar

	progressBar->setRange(0, size + 10);
	progressBar->setValue(0);


	// read header and set volume dimensions

	unsigned short uWidth, uHeight, uDepth;
	fread(&uWidth, sizeof(unsigned short), 1, fp);
	fread(&uHeight, sizeof(unsigned short), 1, fp);
	fread(&uDepth, sizeof(unsigned short), 1, fp);
	
	width = int(uWidth);
	height = int(uHeight);
	depth = int(uDepth);

	// check dataset dimensions
	if (
		width <= 0 || width > 1000 ||
		height <= 0 || height > 1000 ||
		depth <= 0 || depth > 1000)
	{
		std::cerr << "+ Error loading file: " << filename.toStdString() << std::endl;
		std::cerr << "Unvalid dimensions - probably loaded .dat flow file instead of .gri file?" << std::endl;
		return false;
	}

	// compute dimensions
	int slice = width * height;
	size = slice * depth;
	//int test = INT_MAX;
	voxels.resize(size);

	// read volume data

	// read into vector before writing data into volume to speed up process
	std::vector<unsigned short> vecData;
	vecData.resize(size);
	fread((void*)&(vecData.front()), sizeof(unsigned short), size, fp);
	fclose(fp);

	progressBar->setValue(10);


	// store volume data

	for (int i = 0; i < size; i++)
	{
		// data is converted to FLOAT values in an interval of [0.0 .. 1.0];
		// uses 4095.0f to normalize the data, because only 12bit are used for the
		// data values, and then 4095.0f is the maximum possible value
		const float value = fmax(0.0f, fmin(1.0f, float(vecData[i]) / 4095.0f));
		voxels[i] = Voxel(value);
		
		progressBar->setValue(10 + i);

	}

	progressBar->setValue(0);

	std::cout << "Loaded VOLUME with dimensions " << width << " x " << height << " x " << depth << std::endl;

	return true;
}
