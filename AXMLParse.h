#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>

typedef struct FileHeader {
	int32_t magicNumber;
	int32_t fileSize;
} FileHeader;

typedef struct StringItem {
	int16_t charNum;
	int16_t* chars;
}StringItem;

typedef struct StringChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t stringCount;
	int32_t styleCount;
	int32_t unknown;
	int32_t stringPoolOffset;
	int32_t stylePoolOffset;
	int32_t* stringOffsets;
	int32_t* styleOffsets;
	StringItem* stringPool;
}StringChunk;

typedef struct ResourceIDChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t* resourceIDs;
}ResourceIDChunk;

typedef struct StartNamespaceChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t lineNum;
	int32_t unknown;
	int32_t prefix;
	int32_t uri;
}StartNamespaceChunk;

typedef struct EndNamespaceChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t lineNum;
	int32_t unknown;
	int32_t prefix;
	int32_t uri;
}EndNamespaceChunk;

typedef struct AttrItem {
	int32_t spaceUri;
	int32_t name;
	int32_t valueString;
	int32_t type;
	int32_t data;
}AttrItem;

typedef struct StartTagChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t lineNum;
	int32_t unknown;
	int32_t nameSpaceUri;
	int32_t name;
	int32_t flags;
	int32_t atrrCount;
	int32_t classAtrr;
	AttrItem* attrs;
}StartTagChunk;

typedef struct EndTagChunk {
	int32_t chunkType;
	int32_t chunkSize;
	int32_t lineNum;
	int32_t unknown;
	int32_t nameSpaceUri;
	int32_t name;
}EndTagChunk;

typedef struct XMlContentChunk {
	int type;
	void* TagChunk;
	struct XMlContentChunk* next;
	struct XMlContentChunk* prev;
}XMlContentChunk;

typedef struct _MainfestXML {
	FileHeader header;
	StringChunk stringChunk;
	ResourceIDChunk resourceIDChunk;
	StartNamespaceChunk startNamespaceChunk;
	XMlContentChunk xmlContentChunk;
	EndNamespaceChunk endNamespaceChunk;
}MainfestXML;

class AXMLParse
{
public:
	AXMLParse();
	~AXMLParse();

	void startParseString(void* read_string);
	void startParseFile(const char* read_path);
	void writeFormatXmlToFile(const char* write_path);
	void closeParse();

private:

	void initFile(const char* read_path);
	void parseFileHearder();
	void parseStringChunk();

	void parseResourceIDChunk();

	void parseStartNamespaceChunk();
	void parseEndNamespaceChunk();

	void parseStartTagChunk();
	void parseEndTagChunk();

	void addTagChunk(int, void*);

	void parseXmlContent();

	void freeFile();
	void freeMainfestXML();
	void freeXMlContentChunk(XMlContentChunk*);

	void printStringItem(StringItem*);
	std::string getStringItem(StringItem* item);
	std::string getAttributeData(AttrItem* data);
	std::string FormatString(const char* lpcszFormat, ...);
	std::string getPackage(int32_t id);
	const char* getAttrType(int32_t type);
	float complexToFloat(int complex);

	enum AttrType {
		ATTR_NULL = 0,
		ATTR_REFERENCE = 1,
		ATTR_ATTRIBUTE = 2,
		ATTR_STRING = 3,
		ATTR_FLOAT = 4,
		ATTR_DIMENSION = 5,
		ATTR_FRACTION = 6,
		ATTR_FIRSTINT = 16,
		ATTR_DEC = 16,
		ATTR_HEX = 17,
		ATTR_BOOLEAN = 18,
		ATTR_FIRSTCOLOR = 28,
		ATTR_ARGB8 = 28,
		ATTR_RGB8 = 29,
		ATTR_ARGB4 = 30,
		ATTR_RGB4 = 31,
		ATTR_LASTCOLOR = 31,
		ATTR_LASTINT = 31,
	};

	enum TagType {
		TAG_START, TAG_END,
	};

	int
		COMPLEX_UNIT_PX = 0,
		COMPLEX_UNIT_DIP = 1,
		COMPLEX_UNIT_SP = 2,
		COMPLEX_UNIT_PT = 3,
		COMPLEX_UNIT_IN = 4,
		COMPLEX_UNIT_MM = 5,
		COMPLEX_UNIT_SHIFT = 0,
		COMPLEX_UNIT_MASK = 15,
		COMPLEX_UNIT_FRACTION = 0,
		COMPLEX_UNIT_FRACTION_PARENT = 1,
		COMPLEX_RADIX_23p0 = 0,
		COMPLEX_RADIX_16p7 = 1,
		COMPLEX_RADIX_8p15 = 2,
		COMPLEX_RADIX_0p23 = 3,
		COMPLEX_RADIX_SHIFT = 4,
		COMPLEX_RADIX_MASK = 3,
		COMPLEX_MANTISSA_SHIFT = 8,
		COMPLEX_MANTISSA_MASK = 0xFFFFFF;

	float RADIX_MULTS[4] = { 0.00390625F,3.051758E-005F,1.192093E-007F,4.656613E-010F };
	std::string DIMENSION_UNITS[8] = { "px","dip","sp","pt","in","mm","","" };
	std::string FRACTION_UNITS[8] = { "%","%p","","","","","","" };

	MainfestXML mainXML;
	FILE* fp;
};