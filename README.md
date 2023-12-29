Run "make" in order to produce an executable called fsdump. 
fsdump can be executed as ./fsdump <ext2 diskimg> 
The argument provided here is a disk image that must be in ext2 format. 
This program will analyze and interpret the disk image file. 
You can also run make clean to remove the object and executable file. 

There are no limitations! 
It is similar to the dumpe2fs and debugfs command. It will provide information on the superblock,
groups, inodes, directory entries and indirect blocks. 

It has been tested on two ext2 disk images (trivial.img and EXT2_test.img). It correctly worked on those as the ext2 evidence photo demonstrates.
