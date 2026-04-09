#pragma once
#include "root.unity.h"
#include <stdio.h>
#include <sys/time.h>

#define INDENT_AMOUNT 2

unsigned long long currentMilliseconds() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}

internal void printWhitespace(u8 indentAmount, u8 indentLevel, JsonTokenType thisToken, JsonTokenType lastToken) {
	switch (lastToken) {
	case TOKEN_TYPE_STRING:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_TRUE:
	case TOKEN_TYPE_FALSE:
	case TOKEN_TYPE_NULL:
		if (thisToken != TOKEN_TYPE_ARRAY_END && thisToken != TOKEN_TYPE_OBJECT_END && indentLevel != 0) {
			printChar(",");
		}
	default:
		break;
	}

	if ((lastToken == TOKEN_TYPE_OBJECT_END || lastToken == TOKEN_TYPE_ARRAY_END) &&
	    thisToken != TOKEN_TYPE_ARRAY_END && thisToken != TOKEN_TYPE_OBJECT_END && indentLevel != 0) {
		printChar(",");
	}

	if (lastToken != TOKEN_TYPE_OBJECT_KEY) {
		int emptyObject = (lastToken == TOKEN_TYPE_OBJECT_START && thisToken == TOKEN_TYPE_OBJECT_END);
		int emptyArray = (lastToken == TOKEN_TYPE_ARRAY_START && thisToken == TOKEN_TYPE_ARRAY_END);
		int veryStart = (lastToken == 0);

		if (!(emptyObject || emptyArray || veryStart)) {
			printChar("\n");

			for (size_t i = 0; i < (u8)(indentLevel); i++) {
				for (size_t j = 0; j < indentAmount; j++) {
					printChar(" ");
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
	(void)argc;
	const u8 **args = (const u8 **)argv;

	// Hard-limit of 500MiB per JSON file
	//
	// TODO: this is assuming 4096 bytes per page. Page size is
	// architecture-dependent though. For instance, on Arm-based Macs, the page
	// size is 16kb.
	// unsigned long long start = currentMilliseconds();
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

	// unsigned long long end = currentMilliseconds();
	// printf("%llu\n", end - start);
	// start = end;
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
			printChar("\n");
			lastToken = t.tokenType;
			resetJsonStringReader(r);
			goto afterLoop;
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
	// end = currentMilliseconds();
	// printf("%llu\n", end - start);
	return 0;
}
