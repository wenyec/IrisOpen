/*************************************************************
*
* The file, IrisProc.cpp is used to manipulate iris image.
*
*
**************************************************************/
#include "StdAfx.h"

#include "IrisProc.h"
#include "VISIrisApp.h"

/****************************************************************************
*  add the methods for the iris image processing:
*  - getting an iris image quality;
*  - creating an iris image template
*  - match an template by an iris image
*
***************************************************************************/
long    GetIrisImageQuality(BYTE* pImage)
{
	/*~~~~~~~~~~~~~~~*/
	//BYTE	*pRawImage = m_capturedImage.m_pRightIrisImage;
	long	lWidth = 640;
	long	lHeight = 480;
	long	lSize = 640 * 480;
	long	lResult = 0;
	/*~~~~~~~~~~~~~~~*/


#if 0 //disable the croppe image
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long	lCropTopLeftX;
	long	lCropTopLeftY;
	long	lCroppedWidth;
	long	lCroppedHeight;
	BYTE	*l_lpbyCroppedImage = NULL;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	lResult = ::iisCropIrisImage
		(
		CROP_TYPE_CROPPED,
		m_capturedImage.m_pRightIrisImage/*pRawImage*/,
		lWidth,
		lHeight,
		&lCropTopLeftX,
		&lCropTopLeftY,
		&lCroppedWidth,
		&lCroppedHeight,
		&l_lpbyCroppedImage
		);

	if (lResult != 0)
	{
		TCHAR	tmpMsg[100];
		if (lResult == IAIRIS_LICENCE_EXPIRED)
			_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("iisCropIrisImage failed with error %#x"), lResult);
		MessageBox(tmpMsg, TEXT(""), MB_OK);
		//			OnBnClickedBtnImagingClear();
		return;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long	cropImageWidthAdj = ((lCroppedWidth + 3) / 4) * 4;
	long	cropImageHeightAdj = ((lCroppedHeight + 3) / 4) * 4;
	BYTE	*pCroppedImageAdj = new BYTE[cropImageWidthAdj * cropImageHeightAdj];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for (int i = 0; i < lCroppedHeight; i++)
	{
		memcpy((pCroppedImageAdj + i * cropImageWidthAdj), (l_lpbyCroppedImage + i * lCroppedWidth), lCroppedWidth);
	}
	/* //display a cropped image. it's disabled right now.
	m_pCVCppSampleDlg->DisplayRawImage
	(
	m_picImagingCropped,
	pCroppedImageAdj,
	cropImageWidthAdj * cropImageHeightAdj,
	cropImageWidthAdj,
	cropImageHeightAdj
	);
	delete[] pCroppedImageAdj;
	*/
#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long					lQuality;
	IrisImageQualityInfo	stIrisImageQualityInfo;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	std::wstring	message;
	TCHAR			tmpMsg[100];
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	lResult = ::iisGetImageQuality(IRIS_IMAGE_RECT, pImage, lWidth, lHeight, &lQuality, &stIrisImageQualityInfo);

	if (1 && (lResult != 0))
	{
		TCHAR	tmpMsg[100];
		if (lResult == IAIRIS_LICENCE_EXPIRED)
			_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("iisGetImageQuality returned the information code %#x"), lResult);
		MessageBox(ghwndAppMain, tmpMsg, TEXT(""), MB_OK);
		//OnBnClickedBtnImagingClear();//should be double check this function -wcheng
		//return 0;
	}
	//else
	{
		_stprintf(tmpMsg, TEXT("~~~~~~    Quality Score:  \t%d     ~~~~~~\r\n"), lQuality);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("1.  IrisRadius:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.IrisRadius.value, stIrisImageQualityInfo.IrisRadius.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("2.  IrisFocus:   \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.IrisFocus.value, stIrisImageQualityInfo.IrisFocus.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("3.  IrisVisibility:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.IrisVisibility.value, stIrisImageQualityInfo.IrisVisibility.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("4.  ScleraIrisContrast:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.ScleraIrisContrast.value, stIrisImageQualityInfo.ScleraIrisContrast.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("5.  IrisPupilContrast:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.IrisPupilContrast.value, stIrisImageQualityInfo.IrisPupilContrast.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("6.  PupilIrisDiameterRatio:   \t%.0f\t%#x\r\n"), stIrisImageQualityInfo.PupilIrisDiameterRatio.value, stIrisImageQualityInfo.PupilIrisDiameterRatio.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("7.  IrisTexture:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.IrisTexture.value, stIrisImageQualityInfo.IrisTexture.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("8.  Enroll bits:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.EnrollBits.value, stIrisImageQualityInfo.EnrollBits.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("9.  GLSV:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.GLSV.value, stIrisImageQualityInfo.GLSV.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("10. SNR:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.SNR.value, stIrisImageQualityInfo.SNR.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("11. HorizontalMarginScore:  \t%.0f\t%#x\r\n"), stIrisImageQualityInfo.HorizontalMarginScore.value, stIrisImageQualityInfo.HorizontalMarginScore.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("12. VerticalMarginScore:   \t%.0f\t%#x\r\n"), stIrisImageQualityInfo.VerticalMarginScore.value, stIrisImageQualityInfo.VerticalMarginScore.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("13. MotionBlur:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.MotionBlur.value, stIrisImageQualityInfo.MotionBlur.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("14. Eyeness:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.Eyeness.value, stIrisImageQualityInfo.Eyeness.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("15. EyeType:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.EyeType.value, stIrisImageQualityInfo.EyeType.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("16. GazeAngle:  \t\t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.GazeAngle.value, stIrisImageQualityInfo.GazeAngle.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("17. GazeScore:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.GazeScore.value, stIrisImageQualityInfo.GazeScore.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("18. Magnification:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.Magnification.value, stIrisImageQualityInfo.Magnification.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("19. HeadRotationAngle:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.HeadRotationAngle.value, stIrisImageQualityInfo.HeadRotationAngle.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("20. HeadRotationScore:  \t%.0f\t%#x\r\n"), stIrisImageQualityInfo.HeadRotationScore.value, stIrisImageQualityInfo.HeadRotationScore.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("21. InterlaceArtifact:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.InterlaceArtifact.value, stIrisImageQualityInfo.InterlaceArtifact.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("22. ContactLensScore*:  \t%.0f\t%#x\r\n"), stIrisImageQualityInfo.ContactLensScore.value, stIrisImageQualityInfo.ContactLensScore.error_code);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("23. PupilShape*:  \t\t%.0f\t%#x\r\n"), stIrisImageQualityInfo.PupilShape.value, stIrisImageQualityInfo.PupilShape.error_code);
		message.append(tmpMsg);

		//		_stprintf(tmpMsg, TEXT("Cropped (W x H):  %d x %d\r\n"), lCroppedWidth, lCroppedHeight);
		//		message.append(tmpMsg);

		//	}
		//To Be uncommented when GetContactLensInformation is supported in the SDK
		/*
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		long					lContactLensType = 0;
		long					lContactLensScore = 0;
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		lResult = ::iisGetContactLensInformation(IRIS_IMAGE_RECT, pRawImage, lWidth, lHeight, &lContactLensType, &lContactLensScore);
		if(lResult != 0 && IAIRIS_ERROR_CONTACT_LENS != lResult)
		{
		TCHAR	tmpMsg[100];
		if(lResult == IAIRIS_LICENCE_EXPIRED)
		_stprintf(tmpMsg, TEXT("License expired!"));
		else
		_stprintf(tmpMsg, TEXT("iisGetContactLensInformation failed with error %#x"), lResult);
		MessageBox(tmpMsg, TEXT(""), MB_OK);
		//OnBnClickedBtnImagingClear();
		//return;
		}
		else
		{
		_stprintf(tmpMsg, TEXT("Contact Lens Type:  %ld\n"), lContactLensType);
		message.append(tmpMsg);
		_stprintf(tmpMsg, TEXT("Contact Lens Score:   %ld\n"), lContactLensScore);
		message.append(tmpMsg);
		}*/
	}
	//m_picCtrlResults.SetWindowText(message.c_str());
	//m_lQuality = lQuality;
	return lQuality;
}

void    CreateIrisTemplate(BYTE* pImage)
{
	/*~~~~~~~~~~~~~~~*/
	//BYTE	*pRawImage; //pImage
	long	lWidth = gcap.stillWidth;//640;
	long	lHeight = gcap.stillHeight;//480;
	long	lSize = lWidth*lHeight;// 640 * 480;
	long	lResult = 0;
	/*~~~~~~~~~~~~~~~*/

	// TODO: Add your control notification handler code here
	//if(NULL !=m_pProcessedImageBytes) delete[]m_pProcessedImageBytes;
	//if (m_pCVCppSampleDlg->LoadIrisImage(&pRawImage, &lSize, &lWidth, &lHeight) != 0)
	//	return;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	BYTE		pIrisCodeGallery[512] = { 0 };
	ImageInfo	stImageInfoGallery;
	long		lQualityProbe = 0;
	BYTE		*pProcessedImageGallery = new BYTE[lSize];
	BYTE		*pIrisCodeGalleryCreate;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	pIrisCodeGalleryCreate = &pIrisCodeGallery[0];
#if 1  //create a short iris code
	lResult = iisCreateIrisCode
		(
		IRIS_IMAGE_RECT,
		pImage,
		lWidth,
		lHeight,
		pIrisCodeGalleryCreate,
		pProcessedImageGallery,
		&lQualityProbe,
		&stImageInfoGallery
		);
#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	std::wstring	message;
	TCHAR			tmpMsg[100];
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	if (lResult != 0)
	{
		if (pProcessedImageGallery != NULL)
		{
			delete[] pProcessedImageGallery;
		}
		if (lResult == IAIRIS_LICENCE_EXPIRED)
			MessageBox(ghwndAppMain, L"Iris SDK License expired!", L"Fail Message", MB_OK); //_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("CreateIrisCode failed with error %#x"), lResult);//MessageBox(ghwndAppMain, L"reateIrisCode failed with error %#x", L"Fail Message", MB_OK); 
		//message.append(tmpMsg);
		//SetWindowText(ghwndAppMain, message.c_str());
		MessageBox(ghwndAppMain, tmpMsg, L"Fail Message", MB_OK);
		return;
	}
	else //display the successful result and save the iris code into the template gallery
	{
		_stprintf(tmpMsg, TEXT("CreateIrisCode successful\r\n"));

		/* save the iris code into the template gallery file */
		HANDLE hFile;
		wchar_t fname2[128];
		unsigned long dwBytesWritten;
		swprintf(fname2, L"..\\template\\templateTest.dat");

		hFile = CreateFile(fname2,
			GENERIC_WRITE, // open for writing
			0, // do not share
			NULL, // default security
			OPEN_ALWAYS, // open or create
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL); // no attribute template

		if (hFile == INVALID_HANDLE_VALUE)
		{
			_stprintf(tmpMsg, TEXT("Could not open to save!\r\n"));
			message.append(tmpMsg);
		}

		WriteFile(hFile, pIrisCodeGalleryCreate, 512, &dwBytesWritten, NULL);

		if (CloseHandle(hFile) == 0)
		{
			_stprintf(tmpMsg, TEXT("File handle is not closed!\r\n"));
			message.append(tmpMsg);
		}
		if (dwBytesWritten != 0)
		{
			_stprintf(tmpMsg, TEXT("IrisCode Saved:  SUCCESSFULLY\r\n"));
			//MessageBox(tmpMsg, TEXT(""), MB_OK);
			message.append(tmpMsg);
		}

	}
	//	MessageBox(tmpMsg, TEXT(""), MB_OK); //disable the message window.
#if 0 //disable the cropping image
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long	lCropTopLeftX;
	long	lCropTopLeftY;
	long	lCroppedWidth;
	long	lCroppedHeight;
	BYTE	*l_lpbyCroppedImage = NULL;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	lResult = ::iisCropIrisImage
		(
		CROP_TYPE_CROPPED,
		pProcessedImageGallery, //replace the raw image with the processed image
		m_lProcessedImageWidth,
		m_lProcessedImageHeight,
		&lCropTopLeftX,
		&lCropTopLeftY,
		&lCroppedWidth,
		&lCroppedHeight,
		&l_lpbyCroppedImage
		);
	if (lResult != 0)
	{
		if (pProcessedImageGallery != NULL)
		{
			delete[] pProcessedImageGallery;
		}
		TCHAR	tmpMsg[100];
		if (lResult == IAIRIS_LICENCE_EXPIRED)
			_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("iisCropIrisImage failed with error %#x"), lResult);
		message.append(tmpMsg);
		m_ProcessedIrisCode.SetWindowText(message.c_str());
		return;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	m_lProcessedImageWidth = ((lCroppedWidth + 3) / 4) * 4;
	m_lProcessedImageHeight = ((lCroppedHeight + 3) / 4) * 4;
	m_pProcessedImageBytes = new BYTE[m_lProcessedImageWidth * m_lProcessedImageHeight];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	for (int i = 0; i < lCroppedHeight; i++)
	{
		memcpy((m_pProcessedImageBytes + i * m_lProcessedImageWidth), (l_lpbyCroppedImage + i * lCroppedWidth), lCroppedWidth);
	}

	m_pCVCppSampleDlg->DisplayRawImage
		(
		m_picProcessedImage,
		m_pProcessedImageBytes,
		m_lProcessedImageWidth * m_lProcessedImageHeight,
		m_lProcessedImageWidth,
		m_lProcessedImageHeight
		);
#endif
	if (NULL != pProcessedImageGallery /*&& NULL != m_pProcessedImageBytes*/)
	{
		_stprintf(tmpMsg, TEXT("Image Quality:  %ld\r\n"), lQualityProbe);
		message.append(tmpMsg);

		//_stprintf(tmpMsg, TEXT("IrisCode Response:  SUCCESSFULL\r\n"));
		//message.append(tmpMsg);
	}
	else
	{
		delete[] pProcessedImageGallery;
		pProcessedImageGallery = NULL;
		//delete[] m_pProcessedImageBytes;
		//m_pProcessedImageBytes = NULL;
		//OnBnClickedbtnclearprocessedimage();
		return;
	}

	if (pProcessedImageGallery != NULL)
	{
		delete[] pProcessedImageGallery;
	}
	SetWindowText(ghwndAppMain, message.c_str());
}

void    MatchIirisImage(BYTE* pImage)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	long		lResult = 0;
	long		lWidth = 640;
	long		lHeight = 480;
	long		lSize = 640 * 480;
	BYTE		*pIrisCodeGallery = new BYTE[512];
	ImageInfo	stImageInfoGallery;
	long		lQualityProbe = 0;
	BYTE		*pProcessedImageGallery = NULL;//new BYTE[640 * 480];

	/*~~~~~~~~~~~~~~~~~~~~~~~~*/
	std::wstring	message;
	TCHAR			tmpMsg[100];
	/*~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/* load template gallery */
	//TODO loading template gallery file.
	HANDLE hFile;
	wchar_t fname2[128];
	unsigned long dwBytesWritten;
	swprintf(fname2, L"..\\template\\templateTest1.dat");

	hFile = CreateFile(fname2,
		GENERIC_READ, // open for writing
		0, // do not share
		NULL, // default security
		OPEN_ALWAYS, // open or create
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL); // no attribute template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		_stprintf(tmpMsg, TEXT("Could not open to save!\r\n"));
		message.append(tmpMsg);
	}

	ReadFile(hFile, pIrisCodeGallery, 512, &dwBytesWritten, NULL);

	if (CloseHandle(hFile) == 0)
	{
		_stprintf(tmpMsg, TEXT("File handle is not closed!\r\n"));
		message.append(tmpMsg);
	}
	if (dwBytesWritten != 0)
	{
		_stprintf(tmpMsg, TEXT("IrisCode Saved:  SUCCESSFULLY\r\n"));
		MessageBox(ghwndAppMain, tmpMsg, TEXT(""), MB_OK);
		message.append(tmpMsg);
	}
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	ImageInfo	stImageInfoProbe;
	BYTE		*pIrisCodeProbe = new BYTE[1024];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	lResult = iisCreateLongIrisCode
		(
		IRIS_IMAGE_RECT,
		pImage,
		lWidth,
		lHeight,
		pIrisCodeProbe,
		&stImageInfoProbe
		);

	if (lResult != 0)
	{
		if (pIrisCodeGallery != NULL)
		{
			delete[] pIrisCodeGallery;
			pIrisCodeGallery = NULL;
		}

		if (pIrisCodeProbe != NULL)
		{
			delete[] pIrisCodeProbe;
			pIrisCodeProbe = NULL;
		}

		if (lResult == IAIRIS_LICENCE_EXPIRED)
			_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("iisCreateIrisCode failed with error %#x"), lResult);
		MessageBox(ghwndAppMain, tmpMsg, TEXT(""), MB_OK);
		//m_picCtrlResults.SetWindowText(message.c_str());
		return;
	}

	/*~~~~~~~~~~~~~~~~~~~~~*/
	long	matchedIndex = 0;
	float	hd = -1.00;
	/*~~~~~~~~~~~~~~~~~~~~~*/

	lResult = iisMatchByLongIrisCode
		(
		MATCHING_MODE_STANDARD,
		pIrisCodeProbe,
		pIrisCodeGallery,
		1,
		0.32f,
		&matchedIndex,
		&hd
		);
	if (lResult != 0 && lResult != IAIRIS_NO_MATCH_FOUND)
	{
		if (pIrisCodeGallery != NULL)
		{
			delete[] pIrisCodeGallery;
			pIrisCodeGallery = NULL;
		}

		if (pIrisCodeProbe != NULL)
		{
			delete[] pIrisCodeProbe;
			pIrisCodeProbe = NULL;
		}
		TCHAR	tmpMsg[100];
		if (lResult == IAIRIS_LICENCE_EXPIRED)
			_stprintf(tmpMsg, TEXT("License expired!"));
		else
			_stprintf(tmpMsg, TEXT("iisMatchByLongIrisCode failed with error %#x"), lResult);
		MessageBox(ghwndAppMain, tmpMsg, TEXT(""), MB_OK);
		return;
	}


	_stprintf(tmpMsg, TEXT("Matched Index:  %d\r\n"), matchedIndex);
	message.append(tmpMsg);
	_stprintf(tmpMsg, TEXT("Hamming Distance:  %f\r\n"), hd);
	message.append(tmpMsg);

	if (matchedIndex < 0)
	{
		_stprintf(tmpMsg, TEXT("MATCH:  FAILED\r\n"));
		message.append(tmpMsg);
	}
	else
	{
		_stprintf(tmpMsg, TEXT("MATCH:  SUCCESSFULL\r\n"));
		message.append(tmpMsg);
	}

	if (pIrisCodeGallery != NULL)
	{
		delete[] pIrisCodeGallery;
		pIrisCodeGallery = NULL;
	}

	if (pIrisCodeProbe != NULL)
	{
		delete[] pIrisCodeProbe;
		pIrisCodeProbe = NULL;
	}
	//_stprintf(tmpMsg, TEXT("Image Quality:  %d\r\n"), m_lQuality);
	message.append(tmpMsg);
	//m_lQuality = NULL;//clean the quality value.
	//m_picCtrlResults.SetWindowText(message.c_str());
}
