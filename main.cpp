#include <iostream>
#include "AXMLParse.h"

int main(int argc, char* argv[])
{
	if (argc == 3) {
		AXMLParse* axmlparse = new AXMLParse();
		axmlparse->startParseFile(argv[1]);
		axmlparse->writeFormatXmlToFile(argv[2]);
		delete axmlparse;
	}
	else
	{
		printf("Usage: AXMLPARSECPP <binary xml file> <output xml file>");
	}
	return 0;
}
