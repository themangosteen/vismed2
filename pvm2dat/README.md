# README #

This PVM to DAT converter is based on the PVM to RAR converter from the V^3 volume rendering package available at http://sourceforge.net/projects/volren/.
http://sourceforge.net/p/volren/codxe/HEAD/tree/viewer/tools/pvm2raw.cpp

File format is
HEADER: 16 bit width, 16 bit height, 16 bit depth, 16 bit bitsPerVoxel
DATA: voxel intensities in bitsPerVoxel resolution, note that currently this produces 8-bit voxels.

Some PVM files are supplied by the Volume Library at 
http://lgdv.cs.fau.de/External/vollib/

Usage: pvm2dat input.pvm output.dat


TO COMPILE

g++ ddsbase.cpp pvm2dat.cpp -o pvm2dat
