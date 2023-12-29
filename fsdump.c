//#include <linux/ext2_fs.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ext2_fs.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

struct dirBlockPtr {
	__uint32_t blockPtr;
};

void scanDirEntSingly(unsigned int * singlyCalls, unsigned int * ptrScanDirCalled, unsigned int i_num, int fd, struct ext2_dir_entry * dirent, unsigned int blockNum) {
	//printf("HEREEE");
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the singly block
	
	//__uint32_t blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int indirOffset = 0;
	
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		myDirPtr = buffer + (i*4);
		
		//blockPtr = buffer[i*4]; //edit was here
		//memcpy( &blockPtr, buffer + 1 + (i*4), sizeof( __uint32_t ) );
		//printf("BLOCK PTR:%d\n", myDirPtr->blockPtr);

		//printf("INDIR BUF:%x,%x,%x,%x", buffer[0], buffer[1], buffer[2], buffer[3]);
		

		
		if (myDirPtr->blockPtr != 0 ) {		//edit -- was blockPtr

			//BIG INTEGRATION EDIT
			indirOffset = 12 + (256*(*singlyCalls)) + i;
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 1, indirOffset, blockNum, myDirPtr->blockPtr);
		
	
	

			//READ THE BLOCK FOR DIR ENTRIES
			unsigned int offset = 0;
			//unsigned int d_block_num = inode->i_block[i];
			//if (d_block_num == 0) {
			//	break;
			//} else {
			unsigned char dBuffer[1024];

			pread(fd, dBuffer, 1024, 1024*(myDirPtr->blockPtr)); //was multing just the int blockptr till it was ptr
			//have a directory entry in a block
			//go thru the direct blocks first. then indirect
			//1024 bytes in a block. read thru this for each block. 
			while (offset < 1024) {
				dirent = dBuffer + offset;
				//printf("In scan dir ent fn, parent inum: %d, inode:%d\n", i_num, dirent->inode);
				//printf("BLOCK NUM OF INDIR:%d\n", blockNum);
				//printf("OFFSET: %d\n", (12 * 1024) + ((*ptrScanDirCalled) * 1024) + offset);
				
				if (dirent->inode == 0 && dirent->rec_len != 0) {
					//break;
					offset += dirent->rec_len; //edit. was break in both spots
				} else if (dirent->inode == 0) {
					break;
				} else {
				
					char * nameEnt = (char *)malloc(dirent->name_len);

					for (int i = 0; i < dirent->name_len; i++) {
						nameEnt[i] = dirent->name[i];
					}
					printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", i_num, (12 * 1024) + ((*ptrScanDirCalled) * 1024) + offset, dirent->inode, dirent->rec_len, dirent->name_len, nameEnt); //EDIT - removed dirent->name
					//adjust offset
					offset += dirent->rec_len;
					free(nameEnt);
				}
			}
			*ptrScanDirCalled = *ptrScanDirCalled + 1; //inc after the actual block is finished.!!
		}
	}
	*singlyCalls = (*singlyCalls) + 1;
	
}

void scanDirEntDoubly(unsigned int * singlyCalls, unsigned int * ptrScanDirCalled, unsigned int i_num, int fd, struct ext2_dir_entry * dirent, unsigned int blockNum) {
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the singly block
	
	//__u32 blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int indiroffset;
	
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		//blockPtr = buffer[i*4];
		myDirPtr = buffer + (i*4);

		//INTEGRATION EDIT
		if (myDirPtr->blockPtr != 0 ) {
			indiroffset = 12 + (256*(*singlyCalls));
			
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 2, indiroffset, blockNum, myDirPtr->blockPtr);
			
			//now read the singly block
			//readSingly(singlyCalls, fd, i_num, myDirPtr->blockPtr);

			scanDirEntSingly(singlyCalls, ptrScanDirCalled, i_num, fd, dirent, myDirPtr->blockPtr);
		} else {
			//edit added else
			*singlyCalls = (*singlyCalls) + 1; //ensure it is updated for offset tracking
		}
	}
}

void scanDirEntTriply(unsigned int * singlyCalls, unsigned int * ptrScanDirCalled, unsigned int i_num, int fd, struct ext2_dir_entry * dirent, unsigned int blockNum) {
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the singly block
	
	//__u32 blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int indirOffset;
	
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		//blockPtr = buffer[i*4];
		myDirPtr = buffer + (i*4);
		
		if (myDirPtr->blockPtr != 0 ) {
			indirOffset = 12 + (256*(*singlyCalls));
		
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 3, indirOffset, blockNum, myDirPtr->blockPtr);
			
			//now read the singly block
			//readDoubly(singlyCalls, fd, i_num, myDirPtr->blockPtr);

			scanDirEntDoubly(singlyCalls, ptrScanDirCalled, i_num, fd, dirent, myDirPtr->blockPtr);
		} else {
			*singlyCalls = (*singlyCalls) + 1;
		}
	}
}

void readSingly(unsigned int * singlyCalls, int fd, unsigned int i_num, unsigned int blockNum) {
	
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the singly block
	
	//__u32 blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int offset;
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		//blockPtr = buffer[i*4];
		myDirPtr = buffer + (i*4);
		
		if (myDirPtr->blockPtr != 0 ) {
			offset = 12 + (256*(*singlyCalls)) + i;
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 1, offset, blockNum, myDirPtr->blockPtr);
		}
	}
	*singlyCalls = (*singlyCalls) + 1;
}

void readDoubly(unsigned int * singlyCalls, int fd, unsigned int i_num, unsigned int blockNum) {
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the doubly block
	
	//__u32 blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int offset;
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		//blockPtr = buffer[i*4];
		myDirPtr = buffer + (i*4);
		if (myDirPtr->blockPtr != 0 ) {
			offset = 12 + (256*(*singlyCalls));
			
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 2, offset, blockNum, myDirPtr->blockPtr);
			
			//now read the singly block
			readSingly(singlyCalls, fd, i_num, myDirPtr->blockPtr);
		} else {
			*singlyCalls = (*singlyCalls) + 1; //ensure it is updated for offset tracking
		}
	}
	//*singlyCalls = (*singlyCalls) + 256;
}

void readTriply(unsigned int * singlyCalls, int fd, unsigned int i_num, unsigned int blockNum) {
	unsigned char buffer[1024]; //1024 byte buffer 
	ssize_t success = pread(fd, buffer, 1024, 1024*blockNum); //read the triply block
	
	//__u32 blockPtr;
	struct dirBlockPtr * myDirPtr;
	unsigned int offset;
	for (int i = 0; i < 256; i++) {
		//scan if blocks are allocated
		//blockPtr = buffer[i*4];
		myDirPtr = buffer + (i*4);
		offset = 12 + (256*(*singlyCalls));
		if (myDirPtr->blockPtr != 0 ) {
			printf("INDIRECT,%d,%d,%d,%d,%d\n", i_num, 3, offset, blockNum, myDirPtr->blockPtr);
			
			//now read the singly block
			readDoubly(singlyCalls, fd, i_num, myDirPtr->blockPtr);
		} else {
			*singlyCalls = (*singlyCalls) + 1;
		}
	}
}

int main(int argc, char * argv[]) {

	
	
	FILE * filePtr = fopen(argv[1], "r");
	int fd = fileno(filePtr);
	

	//use the header file to handle reading the bytes better
	struct ext2_super_block * supBlock;// = (struct ext2_super_block *) malloc (sizeof(struct ext2_super_block));
	
	
	//read superblock
	unsigned char supBuffer[1024]; //1024 byte buffer superblock
	ssize_t success = pread(fd, supBuffer, 1024, 1024);
	
	if (success > 0) {
		supBlock = supBuffer;
		//read successfully. parse super block info
		//only getting 1 byte. 
		unsigned int blockCount = supBlock->s_blocks_count;
		unsigned int inodeCount = supBlock->s_inodes_count;
		unsigned int blockSize = 1024 << supBlock->s_log_block_size;
		unsigned short inodeSize = supBlock->s_inode_size;
		unsigned int blocksPerGrp = supBlock->s_blocks_per_group;
		unsigned int inodesPerGrp = supBlock->s_inodes_per_group;
		unsigned int firNonRsvInode = supBlock->s_first_ino;
		
		
		
		printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", supBlock->s_blocks_count, supBlock->s_inodes_count, 1024 << supBlock->s_log_block_size, supBlock->s_inode_size, supBlock->s_blocks_per_group, supBlock->s_inodes_per_group, supBlock->s_first_ino);
		
		unsigned int grpCount = inodeCount / inodesPerGrp;
		//calculate num of block in group. 
		unsigned int blocksLeft = blockCount;
		for (int i = 0; i < grpCount; i++) {
			unsigned char grpBuffer[1024];
			
			success = pread(fd, grpBuffer, 1024, 2048 + 1024*i);
			if (success > 0) {
				//read the group descriptor
				struct ext2_group_desc * grpDesc;// = (struct ext2_group_desc *) malloc (sizeof(struct ext2_group_desc));
				grpDesc = grpBuffer;
				unsigned int curBlocksPerGrp;
				if (blocksLeft < blocksPerGrp) {
					curBlocksPerGrp = blocksLeft;
					blocksLeft = 0;
				} else {
					curBlocksPerGrp = blocksPerGrp;
					blocksLeft = blocksLeft - blocksPerGrp;
				}
				
				printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", i, curBlocksPerGrp, inodesPerGrp, grpDesc->bg_free_blocks_count, grpDesc->bg_free_inodes_count, grpDesc->bg_block_bitmap, grpDesc->bg_inode_bitmap, grpDesc->bg_inode_table);
				
				//free(grpDesc);
			}
		}
		
		//scan free block bitmap for each group.
		for (int i = 0; i < grpCount; i++) {
			unsigned char grpBuffer[1024];
			
			success = pread(fd, grpBuffer, 1024, 2048 + 1024*i);
			if (success > 0) {
				struct ext2_group_desc * grpDesc;// = (struct ext2_group_desc *) malloc (sizeof(struct ext2_group_desc));
				grpDesc = grpBuffer;
				
				
				//now read
				unsigned int bitmap = grpDesc->bg_block_bitmap;
				//bitmap is the block num for block bitmap
				unsigned char bitmapBuff[1024];
				success = pread(fd, bitmapBuff, 1024, 1024*bitmap);
				
				for (int j = 0; j < blocksPerGrp / 8; j++) {
					//if ( ( (bitmapBuff[j] & ( 1 << j)) >> j) == 0) {
						//printf("BFREE,%d\n", j);
					//}
					if (((bitmapBuff[j] >> (0) ) % 2) ==0){
						printf("BFREE,%d\n", j*8 + 1);
					}
					if (((bitmapBuff[j] >> (1) ) % 2) ==0){
						printf("BFREE,%d\n", j*8 + 2);
					}
					if (((bitmapBuff[j] >> (2) ) % 2)==0) {
						printf("BFREE,%d\n", j*8 + 3);
					}
					if (((bitmapBuff[j] >> (3) ) % 2) ==0){
						printf("BFREE,%d\n", j*8 + 4);
					}
					if (((bitmapBuff[j] >> (4) ) % 2)==0) {
						printf("BFREE,%d\n", j*8 + 5);
					}
					if (((bitmapBuff[j] >> (5) ) % 2)==0) {
						printf("BFREE,%d\n", j*8 + 6);
					}
					if (((bitmapBuff[j] >> (6) ) % 2)==0) {
						printf("BFREE,%d\n", j*8 + 7);
					}
					if (((bitmapBuff[j] >> (7) ) % 2)==0) {
						printf("BFREE,%d\n", j*8 + 8);
					}
				}
				//free(grpDesc);
			}
		}
		
		//scan the free inodes
		//scan free block bitmap for each group.
		for (int i = 0; i < grpCount; i++) {
			unsigned char grpBuffer[1024];
			
			success = pread(fd, grpBuffer, 1024, 2048 + 1024*i);
			if (success > 0) {
				//struct ext2_group_desc * grpDesc = (struct ext2_group_desc *) malloc (sizeof(struct ext2_group_desc));
				struct ext2_group_desc * grpDesc;
				grpDesc = grpBuffer;
				
				//now read
				unsigned int bitmap = grpDesc->bg_inode_bitmap;
				unsigned char bitmapBuff[1024];
				success = pread(fd, bitmapBuff, 1024, 1024*bitmap);
				
				for (int j = 0; j < inodeCount / 8; j++) {
					//if ( ( (bitmap & ( 1 << j)) >> j) == 0) {
						//printf("IFREE,%d\n", j+1);
					//}
					if (((bitmapBuff[j] >> (0) ) % 2) ==0){
						printf("IFREE,%d\n", j*8 + 1);
					}
					if (((bitmapBuff[j] >> (1) ) % 2) ==0){
						printf("IFREE,%d\n", j*8 + 2);
					}
					if (((bitmapBuff[j] >> (2) ) % 2)==0) {
						printf("IFREE,%d\n", j*8 + 3);
					}
					if (((bitmapBuff[j] >> (3) ) % 2) ==0){
						printf("IFREE,%d\n", j*8 + 4);
					}
					if (((bitmapBuff[j] >> (4) ) % 2)==0) {
						printf("IFREE,%d\n", j*8 + 5);
					}
					if (((bitmapBuff[j] >> (5) ) % 2)==0) {
						printf("IFREE,%d\n", j*8 + 6);
					}
					if (((bitmapBuff[j] >> (6) ) % 2)==0) {
						printf("IFREE,%d\n", j*8 + 7);
					}
					if (((bitmapBuff[j] >> (7) ) % 2)==0) {
						printf("IFREE,%d\n", j*8 + 8);
					}
				}
				//free(grpDesc);
			}
		}
		
		//inode summary scan
		//for each group, scan the inode table block
		//this block will have the inodes for the group
		//use the superblock inode size to parse it correctly, 
		//as well as using the inode per group sup block value
		for (int i = 0; i < grpCount; i++) {
			//printf("TIMES ITER\n");
			unsigned char grpBuffer[1024];
			
			success = pread(fd, grpBuffer, 1024, 2048 + 1024*i);
			if (success > 0) {
				struct ext2_group_desc * grpDesc;// = (struct ext2_group_desc *) malloc (sizeof(struct ext2_group_desc));
				grpDesc = grpBuffer;
				
				//now read
				unsigned int inode_table_block = grpDesc->bg_inode_table;
				
				
				//get block buffer for inode table
				unsigned int sizeInodeTable = inodesPerGrp * 128; //size of inode * nbr in grp. 
				unsigned char inode_block[sizeInodeTable]; //got idea there is 24 inodes in the table... and each is 128 bytes
				pread(fd, inode_block, sizeInodeTable, 1024*inode_table_block); //size was 3072
				//struct ext2_inode * inode = (struct ext2_inode *) malloc (sizeof(struct ext2_inode));
				struct ext2_inode * inode;
				for (int j = 0; j < inodesPerGrp; j++) {
					
					//read an inode
					//inode size is in superblock field. 
					
					inode = inode_block + (j * inodeSize); //want the address
					//grpNum * inodespergrp + 1 + localindex
					unsigned int i_num = (i * inodesPerGrp) + 1 + j;
					//print out for inodes that are not free. 
					//SEG FAULTING HERE
					if (inode->i_mode != 0 && inode->i_links_count != 0) {
						
						
						uint16_t mask = 0xf000; //mask for file type
						uint16_t masked = (mask & inode->i_mode);
						uint16_t mode_mask = 0x0fff;
						uint16_t masked_mode = mode_mask & inode->i_mode;
						
						//find times for inode
						
						//inode change time --> creation time?
						time_t t = inode->i_ctime;
						struct tm * tmp;
						char my_time[50];
						//time(&t);
						tmp = gmtime( &t);
						strftime(my_time, sizeof(my_time), "%x %X", tmp);
						time_t m = inode->i_mtime;
						struct tm * tmp2;
						char my_time2[50];
						
						tmp2 = gmtime( &m);
						strftime(my_time2, sizeof(my_time2), "%x %X", tmp2);
						
						time_t a = inode->i_atime;
						struct tm * tmp3;
						char my_time3[50];
						
						tmp3 = gmtime( &a);
						strftime(my_time3, sizeof(my_time3), "%x %X", tmp3);
						unsigned int singly = 0;
						
						unsigned int * singlyCalls = &singly; //help determine offset for data blocks in singly read calls
						
						if (masked == 0x8000) {
							
							
						
							printf("INODE,%u,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", i_num, "f", masked_mode, inode->i_uid, inode->i_gid, inode->i_links_count, my_time, my_time2, my_time3, inode->i_size, inode->i_blocks, inode->i_block[0],inode->i_block[1],inode->i_block[2],inode->i_block[3],inode->i_block[4],inode->i_block[5],inode->i_block[6],inode->i_block[7],inode->i_block[8],inode->i_block[9],inode->i_block[10],inode->i_block[11], inode->i_block[12],inode->i_block[13],inode->i_block[14]);
							//for each non zero indirect block ptr
							//find out lvl of indirection
							
							if (inode->i_block[12] != 0) {
								readSingly(singlyCalls, fd, i_num, inode->i_block[12]);
								//printf("Singly: %d\n", *singlyCalls);
							}
							if (inode->i_block[13] != 0) {
								readDoubly(singlyCalls, fd, i_num, inode->i_block[13]);
								//printf("Singly: %d\n", *singlyCalls);
							}
							if (inode->i_block[14] != 0) {
								readTriply(singlyCalls, fd, i_num, inode->i_block[14]);
								//printf("Singly: %d\n", *singlyCalls);
							}
							
							
							
						}
						else if (masked == 0x4000) {
							
							printf("INODE,%u,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", i_num, "d", masked_mode, inode->i_uid, inode->i_gid, inode->i_links_count, my_time, my_time2, my_time3, inode->i_size, inode->i_blocks, inode->i_block[0],inode->i_block[1],inode->i_block[2],inode->i_block[3],inode->i_block[4],inode->i_block[5],inode->i_block[6],inode->i_block[7],inode->i_block[8],inode->i_block[9],inode->i_block[10],inode->i_block[11], inode->i_block[12],inode->i_block[13],inode->i_block[14]);
							
							unsigned int scanDirCalled = 0;
							unsigned int * ptrScanDirCalled = &scanDirCalled;
							
							struct ext2_dir_entry * dirent;
							
							
							//Scan the directory entries
							//read the blocks for this directory inode
							for (int i = 0; i < 15; i++) {
								int offset = 0;
								unsigned int d_block_num = inode->i_block[i];
								if (d_block_num == 0) {
									; //EDIT - removed break keyword
								} else {
									unsigned char dBuffer[1024]; //edit - was 1024
			
									pread(fd, dBuffer, 1024, 1024*d_block_num);//edit was 1024
									//have a directory entry in a block
									//go thru the direct blocks first. then indrect
									//1024 bytes in a block. read thru this for each block. 
									

									if (i < 12) {
										while (offset < 1024) {
									
										
											dirent = dBuffer + offset;
											
											if (dirent->inode == 0 && dirent->rec_len != 0) {
												//break;
												offset += dirent->rec_len; //edit. was break in both spots
											} else if (dirent->inode == 0) {
												break;
											} else {

												char * nameEnt = (char *)malloc(dirent->name_len);

												for (int i = 0; i < dirent->name_len; i++) {
													nameEnt[i] = dirent->name[i];
												}
												
												printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", i_num, (i * 1024) + offset, dirent->inode, dirent->rec_len, dirent->name_len, nameEnt); //EDIT -- removed dirent->name . 
												//adjust offset
												offset += dirent->rec_len;
											}
										}
										
											
									} else {
										//dirent = dBuffer + offset;
										
										//pass the indir buffer to fn
										//read and check for valid blocks
										//if valid ones, read dir entries as before
										if (i == 12) {
											//printf("scan dir singly\n");
											
											scanDirEntSingly(singlyCalls, ptrScanDirCalled, i_num, fd, dirent, d_block_num);
											// if (inode->i_block[12] != 0) {
											// 	readSingly(singlyCalls, fd, i_num, inode->i_block[12]);
											// }
										} else if (i == 13) {
											//printf("scan dir doubly\n");
											
											scanDirEntDoubly(singlyCalls, ptrScanDirCalled, i_num, fd, dirent, d_block_num);
											// if (inode->i_block[13] != 0) {
											// 	readDoubly(singlyCalls, fd, i_num, inode->i_block[13]);
												
											// }
										} else if (i == 14) {
											//printf("scan dir triply\n");
											
											scanDirEntTriply(singlyCalls, ptrScanDirCalled, i_num, fd, dirent, d_block_num);
											// if (inode->i_block[14] != 0) {
											// 	readTriply(singlyCalls, fd, i_num, inode->i_block[14]);
												
											// }
										} else {
											break;
										}
											
									}
									
								}
							}
							//for each non zero indirect block ptr
							//find out lvl of indirection
							
							// if (inode->i_block[12] != 0) {
								
							// 	readSingly(singlyCalls, fd, i_num, inode->i_block[12]);
								
							// }
							// if (inode->i_block[13] != 0) {
							// 	readDoubly(singlyCalls, fd, i_num, inode->i_block[13]);
								
							// }
							// if (inode->i_block[14] != 0) {
							// 	readTriply(singlyCalls, fd, i_num, inode->i_block[14]);
								
							// }
							
							
						} 
						else if (masked == 0xA000) {
							
							//figure out if inline or not
							if (inode->i_size < 60) {
								//inline. 
								
								printf("INODE,%u,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d,%d\n", i_num, "s", masked_mode, inode->i_uid, inode->i_gid, inode->i_links_count, my_time, my_time2, my_time3, inode->i_size, inode->i_blocks, inode->i_block[0] );
							} else {
								//print only non zero blocks for sym link
								unsigned int symBlocks[15];
								int countNonZero = 0;
								for (int i =0; i < 15; i++) {
									if (inode->i_block[i] != 0) {
										symBlocks[countNonZero] = inode->i_block[i];
										countNonZero++;
									}
								}
							
								printf("INODE,%u,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d", i_num, "s", masked_mode, inode->i_uid, inode->i_gid, inode->i_links_count, my_time, my_time2, my_time3, inode->i_size, inode->i_blocks );
								
								if (countNonZero > 0) {
									//sym blocks to be printed along
									for (int i = 0; i < countNonZero; i++) {
										printf(",%d", symBlocks[i]);
									}
									printf("\n");								
								} else {
									printf("\n"); //end the line
								}
							}
						} 
						else {
							
							printf("INODE,%u,%s,%o,%d,%d,%d,%s,%s,%s,%d,%d\n", i_num, "?", masked_mode, inode->i_uid, inode->i_gid, inode->i_links_count, my_time, my_time2, my_time3, inode->i_size, inode->i_blocks);
						}
						
					
						
					
					}
				}
				//free(grpDesc);
			}
		}
	
	//free(supBlock);
		
	} else {
		printf("failed to read superblock\n");
	}
	
	
	
	return 1;
}
