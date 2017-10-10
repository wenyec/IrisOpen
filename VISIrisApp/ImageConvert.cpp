#include "StdAfx.h"
#include "ImageConvert.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */

CImageConvert::CImageConvert(void)
{ }

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CImageConvert::~CImageConvert(void)
{ }

/*
 =======================================================================================================================
    Description:: Convert a bmp image to raw 8 bit byte array ;
    Argument:: pBmpImage - the bmp image that the number of bits per pixel is 8 ;
    Argument:: pRawImageWidth - the width of the converted raw image ;
    Argument:: pRawImageHeight - the height of the converted raw image ;
    Return:: Raw 8 bit byte array or NULL
 =======================================================================================================================
 */
BYTE *CImageConvert::BmpToRaw8BitByteArray(BYTE *pBmpImage, long *pRawImageWidth, long *pRawImageHeight)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long				lRawImageSize;
	BITMAPFILEHEADER	*pBmpFileHeader;
	BITMAPINFO			*pBmpInfo;
	BYTE				*pBmpData;
	BYTE				*pRawImage;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Initialize local variables */
	lRawImageSize = 0;
	pBmpFileHeader = NULL;
	pBmpInfo = NULL;
	pBmpData = NULL;
	pRawImage = NULL;

	/* Check input parameters */
	if((pBmpImage == NULL) || (pRawImageWidth == NULL) || (pRawImageHeight == NULL))
	{
		TRACE("Wrong parameters\n");
		return NULL;
	}

	/* Get head information of the bmp image */
	pBmpFileHeader = (BITMAPFILEHEADER *) pBmpImage;
	pBmpInfo = (BITMAPINFO *) (pBmpImage + sizeof(BITMAPFILEHEADER));
	pBmpData = pBmpImage + pBmpFileHeader->bfOffBits;

	/* Check if the file is acceptable */
	if((pBmpInfo->bmiHeader.biBitCount != 8) || (pBmpInfo->bmiHeader.biCompression != BI_RGB))
	{
		TRACE("Invalid bmp image\n");
		return NULL;
	}

	/* Get width and height of the image */
	*pRawImageWidth = pBmpInfo->bmiHeader.biWidth;
	*pRawImageHeight = pBmpInfo->bmiHeader.biHeight;

	/* Get the size of the image */
	lRawImageSize = pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biHeight;

	/* Allocate the memory */
	pRawImage = new BYTE[lRawImageSize];

	/* Copy the raw image on the memory */
	for(long lIndex = 0; lIndex < *pRawImageHeight; lIndex++)
	{
		memcpy
		(
			pRawImage + (lIndex **pRawImageWidth),
			pBmpData + (*pRawImageWidth * (*pRawImageHeight - lIndex - 1)),
			*pRawImageWidth
		);
	}

	return pRawImage;
}

/*
 =======================================================================================================================
    Description:: Convert raw 8 bit byte array to a bmp image ;
    Argument:: pRawImage - Raw 8 bit byte array ;
    Argument:: lRawImageWidth - the width of the raw image ;
    Argument:: lRawImageHeight - the height of the raw image ;
    Return:: the bmp image that the number of bits per pixel is 8
 =======================================================================================================================
 */
BYTE *CImageConvert::Raw8BitByteArrayToBmp(BYTE *pRawImage, long lRawImageWidth, long lRawImageHeight)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BYTE				*pBmp;
	BITMAPFILEHEADER	bmpFileHeader;
	IRISBMPINFO			bmpInfo;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Initialize variables */
	pBmp = NULL;

	/* Check input parameters */
	if((pRawImage == NULL) || (lRawImageWidth <= 0) || (lRawImageHeight <= 0))
	{
		TRACE("Wrong parameters\n");
		return NULL;
	}

	/* Prepare BMP file header */
	bmpFileHeader.bfType = (WORD) (('M' << 8) + 'B');
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(IRISBMPINFO);
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + lRawImageWidth * lRawImageHeight;

	/* Prepare Bmp information */
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = lRawImageWidth;
	bmpInfo.bmiHeader.biHeight = lRawImageHeight;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 8;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 3780;
	bmpInfo.bmiHeader.biYPelsPerMeter = 3780;
	bmpInfo.bmiHeader.biClrUsed = 256;
	bmpInfo.bmiHeader.biClrImportant = 256;

	for(long lIndex = 0; lIndex < 256; lIndex++)
	{
		bmpInfo.bmiColors[lIndex].rgbBlue = (BYTE) lIndex;
		bmpInfo.bmiColors[lIndex].rgbGreen = (BYTE) lIndex;
		bmpInfo.bmiColors[lIndex].rgbRed = (BYTE) lIndex;
		bmpInfo.bmiColors[lIndex].rgbReserved = 0;
	}

	pBmp = new BYTE[bmpFileHeader.bfSize];

	/* Copy BMP file header */
	memcpy(pBmp, &bmpFileHeader, sizeof(BITMAPFILEHEADER));

	/* Copy BMP information */
	memcpy(pBmp + sizeof(BITMAPFILEHEADER), &bmpInfo, sizeof(IRISBMPINFO));

	/* Copy raw image */
	for(long lIndex = 0; lIndex < lRawImageHeight; lIndex++)
	{
		memcpy
		(
			pBmp + bmpFileHeader.bfOffBits + (lIndex * lRawImageWidth),
			pRawImage + (lRawImageWidth * (lRawImageHeight - lIndex - 1)),
			lRawImageWidth
		);
	}

	return pBmp;
}
/*
 =======================================================================================================================
 =======================================================================================================================
*/
BYTE *CImageConvert::RawByteArrayToBmp(BYTE *pRawImage, long lRawImageWidth, long lRawImageHeight)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BYTE				*pBmp;
	BITMAPFILEHEADER	bmpFileHeader;
	IRISBMPINFO			bmpInfo;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Initialize variables */
	pBmp = NULL;

	/* Check input parameters */
	if ((pRawImage == NULL) || (lRawImageWidth <= 0) || (lRawImageHeight <= 0))
	{
		TRACE("Wrong parameters\n");
		return NULL;
	}

	/* Prepare BMP file header */
	bmpFileHeader.bfType = (WORD)(('M' << 8) + 'B');
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(IRISBMPINFO);
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + lRawImageWidth * lRawImageHeight;

	/* Prepare Bmp information */
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = lRawImageWidth;
	bmpInfo.bmiHeader.biHeight = lRawImageHeight;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 8;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = 0;
	bmpInfo.bmiHeader.biXPelsPerMeter = 3780;
	bmpInfo.bmiHeader.biYPelsPerMeter = 3780;
	bmpInfo.bmiHeader.biClrUsed = 256;
	bmpInfo.bmiHeader.biClrImportant = 256;

	for (long lIndex = 0; lIndex < 256; lIndex++)
	{
		bmpInfo.bmiColors[lIndex].rgbBlue = (BYTE)lIndex;
		bmpInfo.bmiColors[lIndex].rgbGreen = (BYTE)lIndex;
		bmpInfo.bmiColors[lIndex].rgbRed = (BYTE)lIndex;
		bmpInfo.bmiColors[lIndex].rgbReserved = 0;
	}

	pBmp = new BYTE[bmpFileHeader.bfSize];

	/* Copy BMP file header */
	memcpy(pBmp, &bmpFileHeader, sizeof(BITMAPFILEHEADER));

	/* Copy BMP information */
	memcpy(pBmp + sizeof(BITMAPFILEHEADER), &bmpInfo, sizeof(IRISBMPINFO));

	/* Copy raw image */
	for (long lIndex = 0; lIndex < lRawImageHeight; lIndex++)
	{
		memcpy
			(
			pBmp + bmpFileHeader.bfOffBits + (lIndex * lRawImageWidth),
			pRawImage + (lRawImageWidth * (lRawImageHeight - lIndex - 1)),
			lRawImageWidth
			);
	}

	return pBmp;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BYTE *CImageConvert::CropBmp24FromCenter(BYTE *pInputBmpImage, long lCroppedWidth, long lCroppedHeight)
{

	/*
	 * input image width and height should be multiple of 4 ;
	 * cropped image width and height will be modified to make them a multiple of 4 ;
	 * Check input parameters
	 */
	if((pInputBmpImage == NULL) || (lCroppedWidth < 100) || (lCroppedHeight < 100))
	{
		TRACE("Wrong parameters\n");
		return NULL;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BITMAPFILEHEADER	*l_pInputBmpFileHeader;
	BITMAPINFO			*l_pInputBmpInfo;
	BYTE				*l_pInputBmpData;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* Get head information of the bmp image */
	l_pInputBmpFileHeader = (BITMAPFILEHEADER *) pInputBmpImage;
	l_pInputBmpInfo = (BITMAPINFO *) (pInputBmpImage + sizeof(BITMAPFILEHEADER));
	l_pInputBmpData = pInputBmpImage + l_pInputBmpFileHeader->bfOffBits;

	/* Check if the file is acceptable */
	if((l_pInputBmpInfo->bmiHeader.biBitCount != 24) || (l_pInputBmpInfo->bmiHeader.biCompression != BI_RGB))
	{
		TRACE("Invalid bmp image\n");
		return NULL;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long				l_lCroppedWidth = 4 * ((lCroppedWidth + 3) / 4);
	long				l_lCroppedHeight = 4 * ((lCroppedHeight + 3) / 4);
	long				l_CroppedFileSize = l_lCroppedWidth * l_lCroppedHeight * 3 + sizeof(BITMAPFILEHEADER) + sizeof
		(BITMAPINFOHEADER);
	BITMAPFILEHEADER	bmfhCropped;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	memset(&bmfhCropped, 0, sizeof(BITMAPFILEHEADER));
	bmfhCropped.bfType = ((WORD) ('M' << 8) | 'B');
	bmfhCropped.bfSize = l_CroppedFileSize;
	bmfhCropped.bfReserved1 = 0;
	bmfhCropped.bfReserved2 = 0;
	bmfhCropped.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BITMAPINFOHEADER	bmihCropped;	/* Info header (40 bytes) */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	memset(&bmihCropped, 0, sizeof(BITMAPINFOHEADER));
	bmihCropped.biSize = sizeof(BITMAPINFOHEADER);
	bmihCropped.biPlanes = 1;
	bmihCropped.biBitCount = 24;
	bmihCropped.biCompression = BI_RGB; /* TODO: From Parameter */
	bmihCropped.biWidth = l_lCroppedWidth;
	bmihCropped.biHeight = l_lCroppedHeight;
	bmihCropped.biSizeImage = 0;
	bmihCropped.biXPelsPerMeter = 0;
	bmihCropped.biYPelsPerMeter = 0;
	bmihCropped.biClrUsed = 0;
	bmihCropped.biClrImportant = 0;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BYTE	*l_pCroppedImage = new BYTE[l_CroppedFileSize];
	long	l_lWriteIndex = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	memcpy((void *) (l_pCroppedImage + l_lWriteIndex), &bmfhCropped, sizeof(BITMAPFILEHEADER));
	l_lWriteIndex += sizeof(BITMAPFILEHEADER);

	memcpy((void *) (l_pCroppedImage + l_lWriteIndex), &bmihCropped, sizeof(BITMAPINFOHEADER));
	l_lWriteIndex += sizeof(BITMAPINFOHEADER);

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long	l_cropX = (l_pInputBmpInfo->bmiHeader.biWidth - lCroppedWidth) / 2;
	long	l_cropY = (l_pInputBmpInfo->bmiHeader.biHeight - lCroppedHeight) / 2;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for(long y = l_cropY; y < l_lCroppedHeight + l_cropY; y++)
	{
		memcpy
		(
			(void *) (l_pCroppedImage + l_lWriteIndex),
			y * l_pInputBmpInfo->bmiHeader.biWidth * 3 + l_pInputBmpData + l_cropX * 3,
			l_lCroppedWidth * 3
		);
		l_lWriteIndex += l_lCroppedWidth * 3;
	}

	return l_pCroppedImage;
}
