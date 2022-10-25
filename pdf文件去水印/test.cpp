#pragma once
//#define PODOFO_HAVE_BOOST
#include <podofo/podofo.h>

#include <iostream>
#include <stack>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cstdio>
#include "PdfContentsGraph.h"
using namespace std;
using namespace PoDoFo;

int main(int argc, char* argv[])
{
	bool all_pages = false;
	int firstPageNo = 0;
	string inputFileName;
	inputFileName = "../../../test.pdf";

	try
	{
		PdfMemDocument doc(inputFileName.c_str());
		if (!doc.GetPageCount())
		{
			std::cerr << "This document contains no page!" << std::endl;
			return 1;
		}

		int toPage = all_pages ? doc.GetPageCount() : firstPageNo + 1;
		for (int i = firstPageNo; i < toPage; ++i)
		{
			cout << "Processing page " << setw(6) << (i + 1) << "..." << std::flush;
			PdfPage* page = doc.GetPage(i);
			PODOFO_RAISE_LOGIC_IF(!page, "Got null page pointer within valid page range");

			PdfContentsTokenizer tokenizer(page);
			PdfContentsGraph grapher(tokenizer);
			grapher.WriteToStdErr();

			cout << " - page ok" << endl;
		}
	}
	catch (const PdfError& e)
	{
		e.PrintErrorMsg();
		return e.GetError();
	}

	cout << endl;
	return 0;
}
