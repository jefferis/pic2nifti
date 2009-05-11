#include "libpic2nifti.h"

int main (int argc, const char * argv[]) {
	nifti_image* ni;
	if (argc < 2) {
		printf("Usage: pic2nifti input.pic\n");
		return 1;
	}
	ni=pic_image_read(argv[1],1);
	nifti_image_infodump(ni);
	nifti_image_write(ni);
	return 0;
}

