#pragma once
class	CImageConvert
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	CImageConvert(void);
	~			CImageConvert(void);

	static BYTE *BmpToRaw8BitByteArray(BYTE *pBmpImage, long *pRawImageWidth, long *pRawImageHeight);
	static BYTE *Raw8BitByteArrayToBmp(BYTE *pRawImage, long lRawImageWidth, long lRawImageHeight);
	static BYTE *CropBmp24FromCenter(BYTE *pInputBmpImage, long lCroppedWidth, long lCroppedHeight);
	static BYTE *RawByteArrayToBmp(BYTE *pRawImage, long lRawImageWidth, long lRawImageHeight);

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
private:

	/* Bitmap Info structure for an iris image */
	struct IRISBMPINFO
	{
		BITMAPINFOHEADER	bmiHeader;
		RGBQUAD				bmiColors[256];
	};
};
