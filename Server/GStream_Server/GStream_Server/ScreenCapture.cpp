#include "ScreenCapture.h"

namespace Gstream{
	namespace capture {
		ScreenCapture::ScreenCapture(HWND handle) {
			this->tPix = 0;
			this->TPix = 0;
			this->hDC = GetDC(NULL);
			this->inf = this->hwndTowinInfo(NULL);
			this->pixelData = 0;

			//force 720p
			this->inf.w = 1280;
			this->inf.h = 720;

			//setup frame container
			//this->rgbFrame = new Frame(0,this->inf.w,this->inf.h,3);

			//print screen res
			std::cout << "Image res: " << this->inf.w << "x" << this->inf.h << std::endl;
		}

		void ScreenCapture::setDefaultHeader(){
			this->outputInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			this->outputInfo.bmiHeader.biWidth = this->inf.w;
			this->outputInfo.bmiHeader.biHeight = -this->inf.h;
			this->outputInfo.bmiHeader.biPlanes = 1;
			this->outputInfo.bmiHeader.biBitCount = 24;
			this->outputInfo.bmiHeader.biCompression = BI_RGB;        
			this->outputInfo.bmiHeader.biSizeImage = 0;   
		}

		void ScreenCapture::setupGDC(){
			setDefaultHeader();
			this->hDest = CreateCompatibleDC(this->hDC);
			this->hbDesktop = CreateCompatibleBitmap(this->hDC, this->inf.w, this->inf.h);
			SelectObject(this->hDest, this->hbDesktop);
		}

		// Capture the screen, returns a bitmap
		HBITMAP ScreenCapture::capScreen(winInfo info){
			this->hDC = GetDC(info.handle);
			this->hDest = CreateCompatibleDC(this->hDC);
			HBITMAP hbDesktop = CreateCompatibleBitmap(this->hDC, info.w, info.h);
			SelectObject(this->hDest, hbDesktop);
			BitBlt( this->hDest, info.x, info.y, info.w, info.h, this->hDC, 0, 0, SRCCOPY | BLACKNESS);
			DeleteDC(this->hDest);
			ReleaseDC(info.handle,this->hDC);
			return hbDesktop;
		}

		void ScreenCapture::snap(const char* file){
			SaveToFile(capScreen(this->inf),file);
		}

		// Extracts info from a hwn and returns a winInfo struct
		winInfo ScreenCapture::hwndTowinInfo(HWND handle){
			winInfo info;
			if (handle == NULL)
			{
				info.handle = NULL;
				info.x = 0;
				info.y = 0;
				info.h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
				info.w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				int wBitCount;
				int iBits = GetDeviceCaps(this->hDC, BITSPIXEL) * GetDeviceCaps(this->hDC, PLANES);
				if (iBits <= 1)
					wBitCount = 1;
				else if (iBits <= 4)
					wBitCount = 4;
				else if (iBits <= 8)
					wBitCount = 8;
				else
					wBitCount = 24;
				info.bpp = wBitCount/8;
				info.size = info.w * info.h * info.bpp;
			}
			else
			{
				info.handle = handle;

				RECT rect;
				// Get the windo info
				GetClientRect(handle, &rect);

				std::cout << rect.top << " " << rect.left << " " << rect.right << " " << rect.bottom << " " << std::endl;
				info.x = rect.top;
				info.y = rect.left;
				info.h = rect.bottom;
				info.w = rect.right;
			}
			return info;
		}

		unsigned char* ScreenCapture::screenCapture(Frame* f,bool freeLast){
			BitBlt( this->hDest, this->inf.x, this->inf.y, this->inf.w, this->inf.h, this->hDC, 0, 0, SRCCOPY);

			this->pixelData = (unsigned char*)malloc(this->inf.size);
			GetDIBits( this->hDest , this->hbDesktop , 0 , this->inf.h , this->pixelData , &this->outputInfo , DIB_RGB_COLORS );

			if(f!=NULL){
				f->setData(this->pixelData,freeLast);
			}

			return this->pixelData;
		}

		// Save a bit map to disk
		BOOL ScreenCapture::SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName){
			HDC hDC;
			int iBits;
			WORD wBitCount;
			DWORD dwPaletteSize=0, dwBmBitsSize=0, dwDIBSize=0, dwWritten=0;
			BITMAP Bitmap;
			BITMAPFILEHEADER bmfHdr;
			BITMAPINFOHEADER bi;
			LPBITMAPINFOHEADER lpbi;
			HANDLE fh, hDib, hPal,hOldPal=NULL;

			hDC = CreateDC("DISPLAY", NULL, NULL, NULL);
			iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
			DeleteDC(hDC);
			if (iBits <= 1)
				wBitCount = 1;
			else if (iBits <= 4)
				wBitCount = 4;
			else if (iBits <= 8)
				wBitCount = 8;
			else
				wBitCount = 24;
			GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = Bitmap.bmWidth;
			bi.biHeight = Bitmap.bmHeight;
			bi.biPlanes = 1;
			bi.biBitCount = wBitCount;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = 0;
			bi.biXPelsPerMeter = 0;
			bi.biYPelsPerMeter = 0;
			bi.biClrImportant = 0;
			bi.biClrUsed = 0;
			dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

			hDib = GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
			lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
			*lpbi = bi;

			hPal = GetStockObject(DEFAULT_PALETTE);
			if (hPal)
			{
				hDC = GetDC(NULL);
				hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
				RealizePalette(hDC);
			}


			GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
				+dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

			if (hOldPal)
			{
				SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
				RealizePalette(hDC);
				ReleaseDC(NULL, hDC);
			}

			fh = CreateFile(lpszFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

			if (fh == INVALID_HANDLE_VALUE)
				return FALSE;

			bmfHdr.bfType = 0x4D42; // "BM"
			dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmfHdr.bfSize = dwDIBSize;
			bmfHdr.bfReserved1 = 0;
			bmfHdr.bfReserved2 = 0;
			bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

			WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

			WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			CloseHandle(fh);
			return TRUE;
		}

		COLORREF ScreenCapture::getPixel(HBITMAP bmp,int x,int y){
			COLORREF pixelColor;
			pixelColor = ::GetPixel(this->hDest, x,y);

			DeleteDC(this->hDest);
			ReleaseDC(NULL, hDC);

			return pixelColor;
		}
	}
}