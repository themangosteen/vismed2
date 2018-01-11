# vismed2

Lab Assignment for Cardiovascular Visualization using Volume Rendering

Minimum Requirements: OpenGL 3.3, Qt 5.5 with Qt3D module
build via CMake

VOLUME DATA

the app supports internal DAT volume data format only.
format is the following:
HEADER: 16 bit width, 16 bit height, 16 bit depth, 16 bit bitsPerVoxel
VOLUME: voxel data with bitsPerVoxel intensity resolution, e.g. if bitsPerVoxel is 8, 8-bit per voxel 

to get more datasets, a tool is provided to convert PVM files to internal 8-bit DAT files.
PVM is a file format defined by V^3: The Versatile Volume Viewer http://www.stereofx.org/volume.html
1. get PVM files
svn co svn://schorsch.efi.fh-nuernberg.de/dicom-data data
http://www9.informatik.uni-erlangen.de/External/vollib/
2. use pvm2dat <input.pvm> <output.dat> to convert
this simply uses pvm2raw provided by V^3 to get the 8-bit volume, and adds the header info.
https://bitbucket.org/lsz/pvm2raw

moreover, DICOM files can be converted to PVM files using , which is the tool that defines the PVM file format.
