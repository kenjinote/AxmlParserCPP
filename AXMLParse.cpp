#include "AXMLParse.h"

AXMLParse::AXMLParse()
{
}

AXMLParse::~AXMLParse()
{
	closeParse();
}

void AXMLParse::startParseString(void* read_string)
{

}
void AXMLParse::startParseFile(const char* read_path)
{
	initFile(read_path);
	parseFileHearder();
	parseStringChunk();
	parseResourceIDChunk();
	parseStartNamespaceChunk();
	parseXmlContent();
}

void AXMLParse::closeParse()
{
	freeFile();
	freeMainfestXML();
}

void AXMLParse::initFile(const char* read_path)
{
	printf("open file:%s\n", read_path);
	errno_t error = fopen_s(&fp, read_path, "rb");
	if (error != 0) {
		puts("Can't open file!\n");
		exit(1);
	}
}

void AXMLParse::writeFormatXmlToFile(const char* write_path)
{
	std::string xmlstr;

	xmlstr.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	xmlstr.append("\n");

	XMlContentChunk* p = mainXML.xmlContentChunk.next;
	while (NULL != p)
	{
		if (p->type == TAG_START)
		{
			StartTagChunk* tagChunk = (StartTagChunk*)p->TagChunk;
			if (tagChunk->name != -1)
			{
				if (getStringItem(mainXML.stringChunk.stringPool + tagChunk->name) == "manifest")
				{
					xmlstr.append("<manifest xmlns:");
					xmlstr.append(getStringItem(mainXML.stringChunk.stringPool + mainXML.startNamespaceChunk.prefix) + "=");
					xmlstr.append("\"" + getStringItem(mainXML.stringChunk.stringPool + mainXML.startNamespaceChunk.uri) + "\"");
					xmlstr.append("\n");
				}
				else
				{
					xmlstr.append("<" + getStringItem(mainXML.stringChunk.stringPool + tagChunk->name));
					xmlstr.append("\n");
				}
			}
			for (int i = 0; i < tagChunk->atrrCount; ++i)
			{
				AttrItem* attrItem = tagChunk->attrs + i;
				if (attrItem->spaceUri != -1)
				{
					xmlstr.append("android:" + getStringItem(mainXML.stringChunk.stringPool + attrItem->name)
						+ "=");
				}
				else
				{
					xmlstr.append(getStringItem(mainXML.stringChunk.stringPool + attrItem->name) + "=");
				}
				if (attrItem->type >> 24 == ATTR_STRING)
				{
					xmlstr.append("\"" + getStringItem(mainXML.stringChunk.stringPool + attrItem->data) + "\"");
				}
				else
				{
					xmlstr.append("\"" + getAttributeData(attrItem) + "\"");
				}
				xmlstr.append("\n");
			}
			xmlstr.append(">");
			xmlstr.append("\n");
		}
		else
		{
			EndTagChunk* tagEndChunk = (EndTagChunk*)p->TagChunk;
			xmlstr.append("</" + getStringItem(mainXML.stringChunk.stringPool + tagEndChunk->name) + ">");
			xmlstr.append("\n");
		}
		p = p->next;
	}
	FILE* file;
	errno_t error = fopen_s(&file, write_path, "wb");
	if (error == NULL)
	{
		fwrite(xmlstr.c_str(), xmlstr.length(), 1, file);
		fclose(file);
	}
	return;
}

void AXMLParse::parseFileHearder()
{
	printf("\n-------------------------File Header-----position:%ld\n", ftell(fp));
	if (fread(&mainXML.header, sizeof(FileHeader), 1, fp) == 1) {
		printf("magicNum:%-8x   fileSize:%-8x\n", mainXML.header.magicNumber, mainXML.header.fileSize);
	}
	else {
		puts("header read error!");
	}
}

void AXMLParse::parseXmlContent()
{
	mainXML.xmlContentChunk.prev = NULL;
	mainXML.xmlContentChunk.next = NULL;

	int isEnd = 0;
	int32_t magicNum;
	while (!isEnd) {
		fread(&magicNum, 4, 1, fp);
		fseek(fp, -4, SEEK_CUR);
		switch (magicNum) {
		case 0x00100102:
			parseStartTagChunk();
			break;
		case 0x00100103:
			parseEndTagChunk();
			break;
		case 0x00100101:
			parseEndNamespaceChunk();
			isEnd = 1;
			break;
		default:
			printf("magic is %-8x unknown\n", magicNum);
		}
	}
	printf("\n------------------parse end!---------------finish at %ld\n", ftell(fp));
}

void AXMLParse::parseStringChunk()
{
	printf("\n-------------------------String chunk----position:%ld\n", ftell(fp));
	if (fread(&mainXML.stringChunk, sizeof(int32_t), 7, fp) == 7) {
		printf("chunk type:%-8x   chunk size:%-8x  string count:%-8x\n", mainXML.stringChunk.chunkType, mainXML.stringChunk.chunkSize, mainXML.stringChunk.stringCount);
		printf("style count:%-8x  unkonwn:%-8x     sting pool offset:%-8x\n", mainXML.stringChunk.styleCount, mainXML.stringChunk.unknown, mainXML.stringChunk.stringPoolOffset);
		printf("style pool offset:%x\n", mainXML.stringChunk.stylePoolOffset);
	}
	else {
		puts("string chunk error!");
	}
	mainXML.stringChunk.stringOffsets = (int32_t*)malloc(4 * mainXML.stringChunk.stringCount);
	if (mainXML.stringChunk.stringCount != 0)
		if (fread(mainXML.stringChunk.stringOffsets, 4, mainXML.stringChunk.stringCount, fp) == mainXML.stringChunk.stringCount) {
			printf("String offsets:\n");
			int i = 0;
			for (i = 0; i < mainXML.stringChunk.stringCount; ++i) {
				printf("%-8x", *(mainXML.stringChunk.stringOffsets + i));
			}
			puts("");
		}
		else {
			puts("sting chunk : stringOffsets error!");
		}
	else {
		puts("no string offset");
	}
	mainXML.stringChunk.styleOffsets = (int32_t*)malloc(4 * mainXML.stringChunk.styleCount);
	if (mainXML.stringChunk.styleCount != 0)
		if (fread(mainXML.stringChunk.styleOffsets, 4, mainXML.stringChunk.styleCount, fp) == mainXML.stringChunk.styleCount) {
			printf("Style offsets:\n");
			int i;
			for (i = 0; i < mainXML.stringChunk.styleCount; ++i) {
				printf("%-8x", *(mainXML.stringChunk.styleOffsets + i));
				printf("\n");
			}
		}
		else {
			puts("sting chunk : styleOffsets error!\n");
		}
	else
		puts("no style offset");
	printf("String pool<<<<<<<<position:%ld\n", ftell(fp));
	mainXML.stringChunk.stringPool = (StringItem*)malloc(sizeof(StringItem) * mainXML.stringChunk.stringCount);
	for (int i = 0; i < mainXML.stringChunk.stringCount; ++i) {
		StringItem* item = (mainXML.stringChunk.stringPool + i);
		fread(&item->charNum, 2, 1, fp);
		item->charNum = (unsigned char)item->charNum;
		item->chars = (int16_t*)malloc(item->charNum + 1);
		fread(item->chars, 1, item->charNum + 1, fp);
		printStringItem(item);
	}
	int32_t emptyByte = 0;
	while (emptyByte != 0x80)
		fread(&emptyByte, 1, 1, fp);
	fseek(fp, -1, SEEK_CUR);
}

void AXMLParse::parseResourceIDChunk()
{
	printf("\n--------------------------ResouceID chunk----position:%ld\n", ftell(fp));
	if (fread(&mainXML.resourceIDChunk, 4, 2, fp) == 2) {
		printf("chunk type:%-8x  chunk size:%-8x\n", mainXML.resourceIDChunk.chunkType, mainXML.resourceIDChunk.chunkSize);
	}
	else {
		puts("resouceIDChunk Header read error");
	}
	mainXML.resourceIDChunk.resourceIDs = (int32_t*)malloc(mainXML.resourceIDChunk.chunkSize - 8);
	if (fread(mainXML.resourceIDChunk.resourceIDs, 4, mainXML.resourceIDChunk.chunkSize / 4 - 2, fp) == (mainXML.resourceIDChunk.chunkSize / 4 - 2)) {
		int i;
		for (i = 0; i < (mainXML.resourceIDChunk.chunkSize / 4 - 2); ++i)
			printf("id:%-10d hex:%-8x\n", *(mainXML.resourceIDChunk.resourceIDs + i), *(mainXML.resourceIDChunk.resourceIDs + i));
	}
	else {
		puts("resource id read error ,in parseResourceIDChunk()");
	}
}

void AXMLParse::parseStartNamespaceChunk()
{
	printf("\n--------------------------StartNamespaceChunk----position:%ld\n", ftell(fp));
	if (fread(&mainXML.startNamespaceChunk, 4, 6, fp) == 6) {
		printf("chunkType:%-8x chunkSize:%-8x lineNum:%-8x unknown:%-8x\n", mainXML.startNamespaceChunk.chunkType, mainXML.startNamespaceChunk.chunkSize, mainXML.startNamespaceChunk.lineNum, mainXML.startNamespaceChunk.unknown);
		printf("prefix:%-8x", mainXML.startNamespaceChunk.prefix);
		printStringItem(mainXML.stringChunk.stringPool + mainXML.startNamespaceChunk.prefix);
		printf("uri:%-8x   ", mainXML.startNamespaceChunk.uri);
		printStringItem(mainXML.stringChunk.stringPool + mainXML.startNamespaceChunk.uri);
	}
	else {
		puts("startNameSpace chunk read error!");
	}
}

void AXMLParse::parseEndNamespaceChunk()
{
	printf("\n--------------------------EndNamespaceChunk----position:%ld\n", ftell(fp));
	if (fread(&mainXML.endNamespaceChunk, 4, 6, fp) == 6) {
		printf("chunkType:%-8x chunkSize:%-8x lineNum:%-8x unknown:%-8x\n", mainXML.endNamespaceChunk.chunkType, mainXML.endNamespaceChunk.chunkSize, mainXML.startNamespaceChunk.lineNum, mainXML.startNamespaceChunk.unknown);
		printf("prefix:%-8x", mainXML.endNamespaceChunk.prefix);
		printStringItem(mainXML.stringChunk.stringPool + mainXML.endNamespaceChunk.prefix);
		printf("uri:%-8x   ", mainXML.endNamespaceChunk.uri);
		printStringItem(mainXML.stringChunk.stringPool + mainXML.endNamespaceChunk.uri);
	}
	else {
		puts("endNameSpace chunk read error!");
	}
}

void AXMLParse::parseStartTagChunk()
{
	printf("\n--------------------------StartTagChunk----------position:%ld\n", ftell(fp));
	StartTagChunk* chunk = (StartTagChunk*)malloc(sizeof(StartTagChunk));
	if (fread(chunk, 4, 9, fp) == 9) {
		printf("chunkType:%-8x  chunkSize:%-8x  lineNum:%-8x unknown:%-8x\n", chunk->chunkType, chunk->chunkSize, chunk->lineNum, chunk->unknown);
		printf("spaceUri:%-8x   name:%-8x <----> ", chunk->nameSpaceUri, chunk->name);
		if (chunk->name != -1)
			printStringItem(mainXML.stringChunk.stringPool + chunk->name);
		else
			puts("null");
		printf("flag:%-8x       attrCount:%-8x   classAttr:%-8x\n", chunk->flags, chunk->atrrCount, chunk->classAtrr);
	}
	else {
		puts("start tag chunk read error");
	}
	printf("Attributes<<<<<<<<position:%ld\n", ftell(fp));
	chunk->attrs = (AttrItem*)malloc(chunk->atrrCount * sizeof(AttrItem));
	int i;
	for (i = 0; i < chunk->atrrCount; ++i) {
		AttrItem* item = (chunk->attrs + i);
		if (fread(item, 4, 5, fp) == 5) {
			printf("spaceUri:%-8x name:%-8x valueStr:%-8x type:%-8x data:%-8x\n", item->spaceUri, item->name, item->valueString, item->type, item->data);
			printf("namespaceUri-->");
			if (item->spaceUri != -1)
				printStringItem(mainXML.stringChunk.stringPool + item->spaceUri);
			else
				puts("null");
			printf("name-->");
			if (item->name != -1)
				printStringItem(mainXML.stringChunk.stringPool + item->name);
			else
				puts("null");
			printf("valueStr-->");
			if (item->valueString != -1)
				printStringItem(mainXML.stringChunk.stringPool + item->valueString);
			else
				puts("null");
			printf("type-->%s\n", getAttrType(item->type));
			printf("data-->");
			if (item->type >> 24 == ATTR_STRING) {
				if (item->data != -1)
				{
					printStringItem(mainXML.stringChunk.stringPool + item->data);
				} else {
					puts("null");
				}
			}
			else {
				printf("%d\n", item->data);
			}
		}
	}
	addTagChunk(TAG_START, chunk);
}

void AXMLParse::parseEndTagChunk()
{
	printf("\n-------------------------End Tag Chunk--------position:%ld\n", ftell(fp));
	EndTagChunk* chunk = (EndTagChunk*)malloc(sizeof(EndTagChunk));
	if (fread(chunk, 4, 6, fp) == 6) {
		printf("chunkType:%-8x chunkSize:%-8x    lineNum:%-8x\n", chunk->chunkType, chunk->chunkSize, chunk->lineNum);
		printf("unknown:%-8x\n", chunk->unknown);
		printf("nameSpaceUri:%-8x <----> ", chunk->nameSpaceUri);
		if (chunk->nameSpaceUri != -1)
			printStringItem(mainXML.stringChunk.stringPool + chunk->nameSpaceUri);
		else
			puts("null");
		printf("name:%-8x <----> ", chunk->name);
		if (chunk->name != -1)
			printStringItem(mainXML.stringChunk.stringPool + chunk->name);
		else
			puts("null");
	}
	else
		puts("read end Tag Chunk error");
	addTagChunk(TAG_END, chunk);
}

void AXMLParse::addTagChunk(int type, void* address)
{
	XMlContentChunk* chunk = (XMlContentChunk*)malloc(sizeof(XMlContentChunk));
	XMlContentChunk* targetChunk = &mainXML.xmlContentChunk;
	while (targetChunk->next != NULL) {
		targetChunk = targetChunk->next;
	}
	targetChunk->next = chunk;
	chunk->type = type;
	chunk->TagChunk = address;
	chunk->next = NULL;
	chunk->prev = targetChunk;
}

void AXMLParse::printStringItem(StringItem* item)
{
	char* chars = (char*)item->chars;
	int i;
	for (i = 0; i < item->charNum; ++i) {
		char c = *(chars + i);
		if (c != 0)
			putchar(c);
	}
	puts("");
}

std::string AXMLParse::getStringItem(StringItem* item)
{
	char* chars = (char*)item->chars;
	char* p = new char[item->charNum + 1];
	int i;
	for (i = 0; i < item->charNum; ++i) {
		char c = *(chars + i);
		if (c != 0)
			sprintf_s(p + i, 2, "%c", c);
	}
	p[item->charNum] = 0;
	std::string str1(p);
	delete[] p;
	return str1;
}

std::string AXMLParse::getAttributeData(AttrItem* data)
{
	if (data->type >> 24 == ATTR_STRING) {
		return "";
	}
	if (data->type >> 24 == ATTR_ATTRIBUTE) {
		return FormatString("?%s%08X", getPackage(data->data).c_str(), data->data);
	}
	if (data->type >> 24 == ATTR_REFERENCE) {
		return FormatString("@%s%08X", getPackage(data->data).c_str(), data->data);
	}
	if (data->type >> 24 == ATTR_FLOAT) {
		return FormatString("%f", data->data);
	}
	if (data->type >> 24 == ATTR_HEX) {
		return FormatString("0x%08X", data->data);
	}
	if (data->type >> 24 == ATTR_BOOLEAN) {
		return data->data != 0 ? "true" : "false";
	}
	if (data->type >> 24 == ATTR_DIMENSION) {
		return FormatString("%f", complexToFloat(data->data)) +
			DIMENSION_UNITS[data->data & COMPLEX_UNIT_MASK];
	}
	if (data->type >> 24 == ATTR_FRACTION) {
		return FormatString("%f", complexToFloat(data->data)) +
			FRACTION_UNITS[data->data & COMPLEX_UNIT_MASK];
	}
	if (data->type >> 24 >= ATTR_FIRSTCOLOR && data->type >> 24 <= ATTR_LASTCOLOR) {
		return FormatString("#%08X", data->data);
	}
	if (data->type >> 24 >= ATTR_FIRSTINT && data->type >> 24 <= ATTR_LASTINT) {
		return FormatString("%d", data->data);
	}
	return FormatString("<0x%X, type 0x%02X>", data->data, data->type);
}

float AXMLParse::complexToFloat(int complex)
{
	return (float)(complex & 0xFFFFFF00) * RADIX_MULTS[(complex >> 4) & 3];
}

std::string AXMLParse::FormatString(const char* lpcszFormat, ...)
{
	char* pszStr = NULL;
	if (NULL != lpcszFormat)
	{
		va_list marker = NULL;
		va_start(marker, lpcszFormat);
		size_t nLength = _vscprintf(lpcszFormat, marker) + 1;
		pszStr = new char[nLength];
		memset(pszStr, '\0', nLength);
		_vsnprintf_s(pszStr, nLength, nLength, lpcszFormat, marker);
		va_end(marker);
	}
	std::string strResult(pszStr);
	delete[]pszStr;
	return strResult;
}

std::string AXMLParse::getPackage(int32_t id)
{
	if (id >> 24 == 1) {
		return "android:";
	}
	return "";
}

const char* AXMLParse::getAttrType(int32_t type)
{
	type = type >> 24;
	switch (type) {
	case ATTR_NULL:
		return "ATTR_NULL";
	case ATTR_REFERENCE:
		return "ATTR_REFERENCE";
	case ATTR_ATTRIBUTE:
		return "ATTR_ATTRIBUTE";
	case ATTR_STRING:
		return "ATTR_STRING";
	case ATTR_FLOAT:
		return "ATTR_FLOAT";
	case ATTR_DIMENSION:
		return "ATTR_DIMENSION";
	case ATTR_FRACTION:
		return "ATTR_FRACTION";
	case ATTR_FIRSTINT:
		return "ATTR_FIRSTINT";
	case ATTR_HEX:
		return "ATTR_HEX";
	case ATTR_BOOLEAN:
		return "ATTR_BOOLEAN";
	case ATTR_FIRSTCOLOR:
		return "ATTR_FIRSTCOLOR";
	case ATTR_RGB8:
		return "ATTR_RGB8";
	case ATTR_ARGB4:
		return "ATTR_ARGB4";
	case ATTR_RGB4:
		return "ATTR_RGB4";
	}
	return "";
}

void AXMLParse::freeMainfestXML()
{
	free(mainXML.stringChunk.stringOffsets);
	free(mainXML.stringChunk.styleOffsets);
	int i;
	for (i = 0; i < mainXML.stringChunk.stringCount; ++i) {
		StringItem* stringPool = (mainXML.stringChunk.stringPool + i);
		free(stringPool->chars);
	}
	free(mainXML.stringChunk.stringPool);
	free(mainXML.resourceIDChunk.resourceIDs);
	XMlContentChunk* freeChunk = mainXML.xmlContentChunk.next;
	while (freeChunk->next != NULL) {
		XMlContentChunk* nextChunk = freeChunk->next;
		freeXMlContentChunk(freeChunk);
		freeChunk = nextChunk;
	}
	freeXMlContentChunk(freeChunk);
}

void AXMLParse::freeXMlContentChunk(XMlContentChunk* chunk)
{
	if (chunk->type == TAG_START) {
		free(((StartTagChunk*)(chunk->TagChunk))->attrs);
	}
	free(chunk->TagChunk);
	free(chunk);
}

void AXMLParse::freeFile()
{
	if (fclose(fp) == EOF) {
		puts("Close file error!\n");
	}
}