#pragma once
#include "root.unity.h"

#define INDENT_AMOUNT 2

int main(int argc, char *argv[]) {
	(void)argc;
	const u8 **args = (const u8 **)argv;

	// Hard-limit of 500MiB per JSON file
	//
	// TODO: this is assuming 4096 bytes per page. Page size is
	// architecture-dependent though. For instance, on Arm-based Macs, the page
	// size is 16kb.
	u64 fileBufferPages = 128000;
	u8 *fileBuffer = allocPages(fileBufferPages);
	// TODO: add this back later to check if we're out of range
	// char *maxFillBufferAddress = fileBuffer + (fileBufferPages * PAGE_SIZE) - 1;
	u8 *startPos = fileBuffer;

	s64 fd = open_(args[1], O_RDONLY, 0);

	u64 numBytesRead;
	for (u64 i = 0; i < fileBufferPages; i++) {
		numBytesRead = read_(fd, fileBuffer, PAGE_SIZE);
		if (!numBytesRead) {
			break;
		}

		fileBuffer += numBytesRead;
	}
	String wholeBufferStr = {.buffer = startPos, .len = fileBuffer - startPos};
	JsonStringReader r = newJsonStringReader(wholeBufferStr);
	JsonToken t;

	JsonTokenType lastToken = 0;
	u8 indentLevel = 0;
	while (true) {
		t = jsonNext(&r);

		switch (t.tokenType) {
		case TOKEN_TYPE_STRING:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("\"");
			printRefString(wholeBufferStr, t.data);
			printChar("\"");
			break;
		case TOKEN_TYPE_NUMBER:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printRefString(wholeBufferStr, t.data);
			break;
		case TOKEN_TYPE_TRUE:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("true");
			break;
		case TOKEN_TYPE_FALSE:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("false");
			break;
		case TOKEN_TYPE_NULL:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("null");
			break;
		case TOKEN_TYPE_OBJECT_START:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("{");
			indentLevel++;
			break;
		case TOKEN_TYPE_OBJECT_END:
			indentLevel--;
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("}");
			break;
		case TOKEN_TYPE_OBJECT_KEY:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("\"");
			printRefString(wholeBufferStr, t.data);
			printChar("\": ");
			break;
		case TOKEN_TYPE_ARRAY_START:
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("[");
			indentLevel++;
			break;
		case TOKEN_TYPE_ARRAY_END:
			indentLevel--;
			printWhitespace(INDENT_AMOUNT, indentLevel, t.tokenType, lastToken);
			printChar("]");
			break;
		case TOKEN_TYPE_EOF:
			// 2 EOFs in a row means we really hit the end of the file
			if (lastToken == TOKEN_TYPE_EOF) {
				goto afterLoop;
			}
			printChar("\n");
			lastToken = t.tokenType;
			resetJsonStringReader(r);
			break;
		case TOKEN_TYPE_ERROR:
			printChar("\nparse error\n");
			printFlush();
			return 1;

		default:
			break;
		}
		lastToken = t.tokenType;
	}
afterLoop:
	printFlush();
	return 0;
}
