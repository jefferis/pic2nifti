/*
 *  biorad.h
 *  pic2nifti
 *
 *  Created by Gregory Jefferis on 2009-05-06.
 *  Copyright 2009 Gregory Jefferis. All rights reserved.
 *
 */
#define BIORAD_HEADER_SIZE 76
typedef struct
	{
		unsigned short nx, ny;    //  0   2*2     image width and height in pixels
		short npic;               //  4   2       number of images in file
		short ramp1_min;          //  6   2*2     LUT1 ramp min. and max.
		short ramp1_max;
		int32_t notes;                // 10   4       no notes=0; has notes=non zero
		short byte_format;        // 14   2       bytes=TRUE(1); words=FALSE(0)
		unsigned short n;         // 16   2       image number within file
		char name[32];            // 18   32      file name
		short merged;             // 50   2       merged format
		unsigned short color1;    // 52   2       LUT1 color status
		unsigned short file_id;   // 54   2       valid .PIC file=12345
		short ramp2_min;          // 56   2*2     LUT2 ramp min. and max.
		short ramp2_max;
		unsigned short color2;    // 60   2       LUT2 color status
		short edited;             // 62   2       image has been edited=TRUE(1)
		short lens;               // 64   2       Integer part of lens magnification
		float mag_factor;         // 66   4       4 byte real mag. factor (old ver.)
		unsigned short dummy[3];  // 70   6       NOT USED (old ver.=real lens mag.)
	} biorad_header;

#define BIORAD_NOTE_HEADER_SIZE 16
#define BIORAD_NOTE_SIZE 80

typedef struct
	{
		short blank;		// 0	2
		int note_flag;		// 2	4
		int blank2;			// 6	4
		short note_type;	// 10	2
		int blank3;			// 12	4
	} biorad_note_header;
