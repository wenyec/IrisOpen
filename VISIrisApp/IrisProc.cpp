/*************************************************************
*
* The file, IrisProc.cpp is used to manipulate iris image.
*
*
**************************************************************/
#include "StdAfx.h"

#include "IrisProc.h"
#include "VISIrisApp.h"

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