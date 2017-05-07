#include "StdAfx.h"
#include "AppCompress.h"
#include <iostream>

using namespace std;

CAppCompress::CAppCompress(void)
{
	// Class Constructor
}

CAppCompress::~CAppCompress(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppCompress::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
}

void CAppCompress::CustomFinal(void) {
	// Add custom finalization code here
}

typedef struct _Tuple {
	int p;
	int l;
	unsigned char c;
	struct _Tuple *next;
} Tuple;

typedef struct {
	unsigned char *data;
	int len;
	int curr;
	int sb_len;
	int lab_len;
} LAB;

int matchString(unsigned char *a, unsigned char *b, unsigned int len_a) {
	int matchedCount = 0;

	while (a[matchedCount] == b[matchedCount] && matchedCount < len_a) {
		matchedCount++;
	}
	return matchedCount;
}

Tuple longestMatchedString(LAB *lab) {
	int longestLen = 0;
	int matchedLen = 0;
	int position = 0;
	Tuple C;

	int min, max;

	min = lab->curr - lab->sb_len;
	if (min < 0) min = 0;
	max = lab->curr + lab->lab_len - 1;
	if (max >= lab->len) max = lab->len - 1;

	int i;

	for (i = min; i < lab->curr; i++) {
		matchedLen = matchString(lab->data + lab->curr, lab->data + i, lab->lab_len);
		if (matchedLen > longestLen) {
			longestLen = matchedLen;
			position = lab->curr - i;
		}
	}

	C.p = position;
	C.l = longestLen;
	lab->curr += longestLen;
	C.c = lab->data[lab->curr];
	C.next = NULL;

	return C;
}

Tuple *Encode_LZ77(LAB lab) {

	Tuple *C = new Tuple;
	Tuple *CPtr = NULL;

	CPtr = C;
	while (lab.curr < lab.len) {
		*CPtr = longestMatchedString(&lab);
		lab.curr++;
		if (lab.curr < lab.len) {
			CPtr->next = new Tuple;
			CPtr = CPtr->next;
		}
		else {
			CPtr->next = NULL;
		}
	}

	return C;
}

void Decode_LZ77(Tuple *C, LAB *lab) {

	Tuple *Cnext;

	Cnext = C;
	while (Cnext) {
		for (int i = 0; i < Cnext->l; i++) {
			lab->data[lab->curr] = lab->data[lab->curr - Cnext->p];
			lab->curr++;
		}
		lab->data[lab->curr] = Cnext->c;
		lab->curr++;
		Cnext = Cnext->next;
	}
}

Tuple *ctest;

// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {

	// You can modify anything within this function, but you cannot change the function prototype.
	unsigned char *compressedData ;

	//cDataSize = width * height * 3 ;				// You need to determine the size of the compressed data. 
													// Here, we simply set it to the size of the original image
	//compressedData = new unsigned char[cDataSize] ; // As an example, we just copy the original data as compressedData.

	LAB lab;
	Tuple *C, *Cnext;
	lab.data = pInput;
	lab.sb_len = 4095;
	lab.lab_len = 15;
	lab.curr = 0;
	lab.len = width * height * 3;
	C = Encode_LZ77(lab);
	Cnext = C;	
	int count = 0;
	unsigned char *tempData;
	tempData = new unsigned char[width * height * 6];
	while (Cnext) {
		tempData[count * 3] = ( Cnext->p << 4 |(Cnext->l)) & 0xFF;
		tempData[count * 3 + 1] = (Cnext->p >> 4) & 0xFF;
		tempData[count * 3 + 2] = Cnext->c & 0xFF;
		count++;
		Cnext = Cnext->next;
	}
	
	cDataSize = count * 3;
	compressedData = new unsigned char[cDataSize];
	for (int i = 0; i < cDataSize; i++)
		compressedData[i] = tempData[i];

	return compressedData ;		// return the compressed data
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

	// You can modify anything within this function, but you cannot change the function prototype.
	//memcpy(uncompressedData, compressedData, cDataSize) ;	// Here, we simply copy the compressedData into the output buffer.

	LAB lab;
	Tuple *CPtr = NULL;
	Tuple *C = new Tuple;
	CPtr = C;
	for (int i = 0; i < cDataSize / 3; i++) {
		CPtr->l = compressedData[i * 3] & 0xF;
		CPtr->p = ((compressedData[i * 3] >> 4) | (compressedData[i * 3 + 1] << 4)) & 0xFFF;
		CPtr->c = compressedData[i * 3 + 2] & 0xFF;
		if (i < (cDataSize / 3) - 1) {
			CPtr->next = new Tuple;
			CPtr = CPtr->next;
		}
		else
			CPtr->next = NULL;
	}
	
	lab.sb_len = 4095;
	cDataSize = height * width * 3;
	lab.lab_len = 15;
	lab.curr = 0;
	lab.len = 0;
	lab.data = new unsigned char[cDataSize * 2];
	lab.data[0] = 0;
	Decode_LZ77(C, &lab);
	memcpy(uncompressedData, lab.data, cDataSize);

}


void CAppCompress::Process(void) {

	// Don't change anything within this function.

	int i, cDataSize ;

	unsigned char *compressedData ;
	unsigned char *verifyCompressedData ;

	SetTitle(pOutput, _T("Lossless Decompressed Image")) ;

	compressedData = Compress(cDataSize) ;

	verifyCompressedData = new unsigned char [cDataSize] ;

	memcpy(verifyCompressedData, compressedData, cDataSize) ;

	delete [] compressedData ;

	Decompress(verifyCompressedData, cDataSize, pOutput) ;

	for(i = 0; i < width * height * 3; i++) {
		if(pInput[i] != pOutput[i]) {
			printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n")) ;
			break ;
		}
	}

	printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize, (double) width * height * 3 / cDataSize) ;

	PutDC(pOutput) ;
}
