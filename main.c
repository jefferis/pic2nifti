#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include "nifti1_io.h"
#include "biorad.h"


nifti_image *pic_image_read (const char *filename, bool load_data){
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
	
    nifti_image* ni;
    ni=malloc(sizeof(nifti_image));
    if(!ni) {
        printf("Failed to allocate memory to nifti image structure!\n");
        return NULL;
    }
    
    ni->nifti_type=NIFTI_FTYPE_NIFTI1_1;
	ni->ndim=3;
	ni->nx=header.nx;
	ni->ny=header.ny;
	ni->nz=header.npic;
	ni->nbyper=header.byte_format?1:2;
	ni->nvox=ni->nx*ni->ny*ni->nz;
	ni->datatype=header.byte_format?DT_UNSIGNED_CHAR:NIFTI_TYPE_UINT16;
	ni->fname=malloc(strlen(filename)+1);
    strcpy(ni->fname, filename);
    strcpy(ni->fname+strlen(filename)-3,"nii");
	
	if(load_data){
		if(!(ni->data=calloc(ni->nvox,ni->nbyper))){
			printf("Failed to allocate memory to load image!\n");
			return NULL;
		}
		if((n=fread(ni->data, ni->nbyper, ni->nvox, f))!=ni->nvox){
			printf("Failed to load image data!\n");
			return NULL;
//		} else {
//			printf("Safely loaded data!\n");
		}
		// Byte swap if necessary
		if(ni->nbyper==2 && CFByteOrderGetCurrent()!=CFByteOrderLittleEndian){
			int i=0;
			for(;ni->nvox;i++)
				CFSwapInt16(((unsigned short*)ni->data)[i]);
		}
	} else {
		if(fseek(f, ni->nvox*ni->nbyper, SEEK_CUR)!=0)
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
		
		printf("regular note line %s\n",note);
		printf("note flag = %d, note type = %d\n",nh.note_flag,nh.note_type);
		
		// These are not interesting notes
		if(nh.note_type==1) continue;
		
		// Look for calibration information
		double d1, d2, d3;
		if ( 3 == sscanf( note, "AXIS_2 %lf %lf %lf", &d1, &d2, &d3 ) ){
			printf("X Calibration = %lf",d3);
			ni->dx=d3;
		} 
		if ( 3 == sscanf( note, "AXIS_3 %lf %lf %lf", &d1, &d2, &d3 ) )
			ni->dy=d3;
		if ( 3 == sscanf( note, "AXIS_4 %lf %lf %lf", &d1, &d2, &d3 ) )
			ni->dz=d3;

		if(nh.note_flag==0) break;
	}
	
	// Assume units are microns (all the Biorads I have ever seen are)
	ni->xyz_units=NIFTI_UNITS_MICRON;

	fclose(f);

	return ni;
}

int main (int argc, const char * argv[]) {
    nifti_image* ni;
	if(argc>0){
		ni=pic_image_read(argv[1],TRUE);
        nifti_image_infodump(ni);
        nifti_image_write(ni);
	}
    return 0;
}
