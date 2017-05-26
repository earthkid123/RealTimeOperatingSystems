// eFile.c
// Runs on either TM4C123 or MSP432
// High-level implementation of the file system implementation.
// Daniel and Jonathan Valvano
// September 13, 2016
#include <stdint.h>
#include "eDisk.h"
#include "FlashProgram.h"

uint8_t Buff[512];
uint8_t Directory[256], FAT[256];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active
// Return the larger of two integers.
int16_t max(int16_t a, int16_t b){
  if(a > b){
    return a;
  }
  return b;
}
// if directory and FAT not loaded,
// bring it into RAM from disk
void MountDirectory(void){ 
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
// **write this function**
	uint16_t i;
if(bDirectoryLoaded == 0)
		{
		eDisk_ReadSector(Buff,255);
		for(i = 0; i<256;i++)
		{
			Directory[i] = Buff[i];
			FAT[i] = Buff[i+256];
		}
		bDirectoryLoaded = 1;	
	}
}

// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
// **write this function**
	/*uint8_t index=start, m;
	if(index == 255)
		return index;
	else{
		m = FAT[index];
		while(m != 255)
		{	
			index = m;
			m=FAT[index];
		}
		return index; // replace this line*/
	uint8_t m = 0;
  if(start == 255){
    return 255;
  }
  m = FAT[start];
  while(m != 255){
    start = m;
    m = FAT[start];
  }
  return start;
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
// **write this function**
  uint8_t i=0;
	int fs=-1;
	uint8_t ls;
	ls=lastsector(Directory[i]);
	while(ls!=255)
	{
		fs=max(fs,ls);
		i=i+1;
		ls=lastsector(Directory[i]);
	}
  return fs+1; // replace this line

}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
// **write this function**
	uint8_t i;
  uint8_t m;
	
	i = Directory[num];
	
	if (255 == i) {
		Directory[num] = n;
	} else {
		
		m = FAT[i];
		while(255 != m) {
			i = m;
			m = FAT[i];
		}
		FAT[i] = n;
	}

  return 0;
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: none
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(void){
// **write this function**
	uint16_t i = 0;
	if(!bDirectoryLoaded)
		MountDirectory();
	while(Directory[i] != 255)
	{
		i++;
		if(i==255)
			return 255;
	}
	return i;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
// **write this function**
	uint8_t index=num, m;
	uint8_t Size = 0;
	if(Directory[index] == 255)
		return Size;
	else{
		//m = FAT[index];
		m=Directory[index];
		//Size++;
		while(m != 255)
		{	
			index = m;
			m=FAT[index];
			Size++;
		}
		return Size;
	}
  
	
  //return 0; // replace this line
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
// **write this function**
  uint8_t n;
	n=findfreesector();
	if(n!=255)
	{
		eDisk_WriteSector(buf,n);
		appendfat(num,n);
		return 0;
	}
  return 255;
}

//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
// **write this function**
  uint8_t next = 0;
	uint8_t count = 0;
											 
	next = Directory[num];
	if (num == 255 || location == 255)
	//if (next == 255 || location == 255)
		 return 255;
										 
	while(count != location)
	{
			next = FAT[next];
			if (next != 255)
			{
				count++;
			}
			else
				return next;
	}
	return eDisk_ReadSector(buf,next);
  //return 0; // replace this line
}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){
// **write this function**

	uint16_t i;
	uint8_t buff1[512];
	uint32_t* pt;
	pt = (uint32_t*)buff1;
	for (i=0;i<256;i++)
	{
   buff1[i]=Directory[i];
   buff1[i+256]=FAT[i];
	}
	//Flash_WriteArray(pt,EDISK_ADDR_MAX	,512);
	eDisk_WriteSector(buff1,255);
	return RES_OK;				// replace this line
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero
// **write this function**
	eDisk_Format();
	bDirectoryLoaded = 0;
	
  return 0; // replace this line
}
