// (c) by Stefan Roettger, licensed under GPL 2+

#include "codebase.h"
#include "ddsbase.h"

#include <iostream>


int main(int argc, char *argv[])
{
    unsigned char *volume; // 8 bit voxels

    unsigned int width, height, depth, components;

    float scalex, scaley, scalez;

    if (argc!=2 && argc!=3)
    {
        printf("usage: %s <input.pvm> [<output.dat>]\n",argv[0]);
        exit(1);
    }

    printf("reading PVM file\n");

    if ((volume = readPVMvolume(argv[1], &width, &height, &depth, &components, &scalex, &scaley, &scalez)) == NULL) exit(1);
    if (volume == NULL) exit(1);

    printf("found volume with width=%d height=%d depth=%d components=%d\n", width, height, depth, components);

    if (components > 1)
    {
        printf("color volumes not supported.");
        exit(1);
    }

    if (argc>2)
    {

        unsigned int byteSizeHeader = 2*4;
        unsigned int byteSizeVolume = width*height*depth*components; // assuming 8-bit voxels

        // HEADER: 16 bit width, 16 bit height, 16 bit depth, 8 bit bitsPerVoxel intensity resolution
        // DATA: voxel intensities (here we always use 8 bit resolution)

        FILE *file;
        const char *filename = argv[2];

        if (byteSizeVolume < 1) ERRORMSG();

        if ((file = fopen(filename, "wb")) == NULL) IOERROR();
        unsigned short bitsPerVoxel = 8;
        unsigned short header[] = { (unsigned short)width, (unsigned short)height, (unsigned short)depth, bitsPerVoxel };
        if (fwrite(header, 2, 4, file) != byteSizeHeader) IOERROR(); // write wid
        if (fwrite(volume, 1, byteSizeVolume, file) != byteSizeVolume) IOERROR(); // write volume

        fclose(file);

    }

    free(volume);

    return(0);
}
