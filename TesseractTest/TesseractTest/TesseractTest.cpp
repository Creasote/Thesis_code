// TesseractTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <baseapi.h>
#include <allheaders.h>
#include <iostream>

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	tesseract::TessBaseAPI api;
	api.Init("", "eng", tesseract::OEM_DEFAULT);
	api.SetPageSegMode(static_cast<tesseract::PageSegMode>(7));
	api.SetOutputName("out");

	cout<<"File name:";
	char image[256];
	cin>>image;
	PIX   *pixs = pixRead(image);

	string text_out;
	api.ProcessPages(image, NULL, 0, &text_out);

	cout<<text_out.string();

	return 0;
}

