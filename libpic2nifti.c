#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include "libpic2nifti.h"

nifti_image *pic_image_read (const char *filename, int load_data){
	FILE *f;
	unsigned char buffer[BIORAD_HEADER_SIZE];
	
	int n;
	biorad_header header;

	f = fopen(filename, "rb");
	if (f)
		n = fread(&buffer, BIORAD_HEADER_SIZE, 1, f);
	if(!f || n!=1)
		printf("Problem reading file!\n");
	
	memcpy( &header.nx, buffer+0, sizeof( header.nx ) );
	memcpy( &header.ny, buffer+2, sizeof( header.ny ) );
	memcpy( &header.npic, buffer+4, sizeof( header.npic ) );
	memcpy( &header.byte_format, buffer+14, sizeof( header.byte_format ) );
	
	memcpy( &header.file_id, buffer+54, sizeof( header.file_id ) );
	
	header.nx=CFSwapInt16LittleToHost(header.nx);
	header.ny=CFSwapInt16LittleToHost(header.ny);
	header.npic=CFSwapInt16LittleToHost(header.npic);
	header.file_id=CFSwapInt16LittleToHost(header.file_id);
	
	if(header.file_id!=12345){
		printf("This is not a valid biorad pic file!\n");
		return NULL;
	}
	
	nifti_image* nim = nifti_simple_init_nim();
	if(!nim) {
		printf("Failed to allocate memory to nifti image structure!\n");
		return NULL;
	}
	
	nim->nifti_type=NIFTI_FTYPE_NIFTI1_1;
	nim->ndim = 3;
	nim->nx = header.nx;
	nim->ny = header.ny;
	nim->nz = header.npic;
	nim->nt = 0;
	nim->nu = 0;
	nim->nv = 0;
	nim->nw = 0;
	nim->dim[0] = 3;
	nim->dim[1] = header.nx;
	nim->dim[2] = header.ny;
	nim->dim[3] = header.npic;
	nim->nbyper=header.byte_format?1:2;
	nim->nvox=nim->nx*nim->ny*nim->nz;
	nim->datatype=header.byte_format?DT_UNSIGNED_CHAR:NIFTI_TYPE_UINT16;
	nim->fname=malloc(strlen(filename)+1);
	strcpy(nim->fname, filename);
	strcpy(nim->fname+strlen(filename)-3,"nii");
	
	if(load_data){
		if(!(nim->data=calloc(nim->nvox,nim->nbyper))){
			printf("Failed to allocate memory to load image!\n");
			return NULL;
		}
		if((n=fread(nim->data, nim->nbyper, nim->nvox, f))!=nim->nvox){
			printf("Failed to load image data!\n");
			return NULL;
//		} else {
//			printf("Safely loaded data!\n");
		}
		// Byte swap if necessary
		if(nim->nbyper==2 && CFByteOrderGetCurrent()!=CFByteOrderLittleEndian){
			int i=0;
			for(;nim->nvox;i++)
				CFSwapInt16(((unsigned short*)nim->data)[i]);
		}
	} else {
		if(fseek(f, nim->nvox*nim->nbyper, SEEK_CUR)!=0)
			printf("Failed to seek to biorad footer\n");
	}
	
	// Now read footer which includes calibration information
	char noteheaderbuf[BIORAD_NOTE_HEADER_SIZE];
	char note[BIORAD_NOTE_SIZE];
	biorad_note_header nh;
	
	while (!feof(f)) {
		fread(&noteheaderbuf, BIORAD_NOTE_HEADER_SIZE, 1, f);
		fread(&note, BIORAD_NOTE_SIZE, 1, f);
		memcpy(&nh.note_flag, noteheaderbuf+2, sizeof(nh.note_flag));
		memcpy(&nh.note_type, noteheaderbuf+10, sizeof(nh.note_type));
		
//		printf("regular note line %s\n",note);
//		printf("note flag = %d, note type = %d\n",nh.note_flag,nh.note_type);
		
		// These are not interesting notes
		if(nh.note_type==1) continue;
		
		// Look for calibration information
		double d1, d2, d3;
		if ( 3 == sscanf( note, "AXIS_2 %lf %lf %lf", &d1, &d2, &d3 ) ){
//			printf("X Calibration = %lf",d3);
			nim->dx = d3;
			nim->pixdim[1] = d3;
		} 
		if ( 3 == sscanf( note, "AXIS_3 %lf %lf %lf", &d1, &d2, &d3 ) ){
			nim->dy = d3;
			nim->pixdim[2] = d3;
		}
		if ( 3 == sscanf( note, "AXIS_4 %lf %lf %lf", &d1, &d2, &d3 ) ){
			nim->dz = d3;
			nim->pixdim[3] = d3;
		}

		if(nh.note_flag==0) break;
	}
	nim->dt = 0.0;
	nim->du = 0.0;
	nim->dv = 0.0;
	nim->dw = 0.0;
	nim->qfac = 1.0;
	nim->qoffset_x = 0.0;
	nim->qoffset_y = 0.0;
	nim->qoffset_z = 0.0;
	
	// Assume units are microns (all the Biorads I have ever seen are)
	nim->xyz_units=NIFTI_UNITS_MICRON;

	fclose(f);

	return nim;
}

