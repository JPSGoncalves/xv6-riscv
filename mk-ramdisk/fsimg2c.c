#include "stdio.h"
#include "string.h"
#include "stdlib.h"

void main(void) {
	FILE *fIn, *fOut;
	fIn = fopen("fs.img", "rb");

	if (fIn) {
		unsigned int fileLength;

		fseek(fIn, 0, SEEK_END);
		fileLength = ftell(fIn);
		fseek(fIn, 0, SEEK_SET);
		
		fOut = fopen("ram_disk.h", "wt");
		if (fOut) {
			unsigned char uchar;
			int nRead;
			int byteCountOnLine = 0;
			unsigned int fileIndex = 0;			
			
			char line1[] = "unsigned char fs_img[] = {";

			printf("%s\n ",line1);
			fprintf(fOut,"%s\n ",line1);

			do {
				unsigned char uchar;
				nRead = fread(&uchar, 1, 1, fIn);
				fileIndex++;
				if (nRead == 1) {
					if (fileIndex == fileLength) {
						// last line should not have a comma
						printf(" 0x%02x\n",uchar);
						fprintf(fOut," 0x%02x\n",uchar);
					} else {
						printf(" 0x%02x,",uchar);
						fprintf(fOut," 0x%02x,",uchar);
					}
					byteCountOnLine++;
					if (byteCountOnLine == 12) {
						byteCountOnLine = 0;
						printf("\n ");
						fprintf(fOut,"\n ");
					}
				} else {
				    printf("File read in error (nRead != 1) before end of file\n");
				}
			} while (fileIndex < fileLength);
			
			printf("};\n");
			fprintf(fOut,"};\n");
			
			printf("unsigned int fs_img_len = %d;\n",fileLength);
			fprintf(fOut,"unsigned int fs_img_len = %d;\n",fileLength);			

			fclose(fOut);
		}
		fclose(fIn);
	}
}
