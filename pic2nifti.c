#include "libpic2nifti.h"

int main (int argc, const char * argv[]) {
	nifti_image* nim;
	if (argc < 2) {
		printf("Usage: pic2nifti input.pic\n");
		return 1;
	}
	nim = pic_image_read(argv[1],1);
	nifti_image_infodump(nim);
	nifti_image_write(nim);
	nifti_image_free(nim);
	return 0;
}

