#pragma once
#include "root.unity.h"

typedef enum {
	PARSE_STATE_START,
	PARSE_STATE_SCALAR,
	PARSE_STATE_COMMA,
	PARSE_STATE_COLON,
	PARSE_STATE_ARRAY_START,
	PARSE_STATE_ARRAY_END,
	PARSE_STATE_OBJECT_START,
	PARSE_STATE_OBJECT_KEY,
	PARSE_STATE_OBJECT_END,
	PARSE_STATE_END,
} JsonParseState;

typedef enum {
	TOKEN_TYPE_ERROR,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_TRUE,
	TOKEN_TYPE_FALSE,
	TOKEN_TYPE_NULL,
	TOKEN_TYPE_OBJECT_START,
	TOKEN_TYPE_OBJECT_END,
	TOKEN_TYPE_OBJECT_KEY,
	TOKEN_TYPE_ARRAY_START,
	TOKEN_TYPE_ARRAY_END,
	TOKEN_TYPE_EOF,
} JsonTokenType;

typedef enum {
	CONTAINER_NONE,
	CONTAINER_ARRAY,
	CONTAINER_OBJECT,
} Container;

// clang-format off

/// Table mapping the current state of the parser with valid new states. `255`
/// is a sentinel value indicating that we don't need to iterate any more.
static u8 validNewStates[115] = {
    // Valid states from PARSE_STATE_START, CONTAINER_NONE
	PARSE_STATE_SCALAR,
	PARSE_STATE_ARRAY_START,
	PARSE_STATE_OBJECT_START,
	255,
    // Valid states from PARSE_STATE_SCALAR, CONTAINER_NONE
	PARSE_STATE_END,
	255,
    0,
    0,
    0,
    0,
    0, // 10
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    // Valid states from PARSE_STATE_ARRAY_END, CONTAINER_NONE
    PARSE_STATE_END, // 20
    255,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 30
    0,
    // Valid states from PARSE_STATE_OBJECT_END, CONTAINER_NONE
    PARSE_STATE_END,
    255,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 40
    0,
    0,
    0,
    // Valid states from PARSE_STATE_SCALAR, CONTAINER_ARRAY
    PARSE_STATE_ARRAY_END,
    PARSE_STATE_COMMA,
    255,
    0,
    // Valid states from PARSE_STATE_COMMA, CONTAINER_ARRAY
    PARSE_STATE_SCALAR,
    PARSE_STATE_ARRAY_START,
    PARSE_STATE_OBJECT_START, // 50
    255,
    0,
    0,
    0,
    0,
    // Valid states from PARSE_STATE_ARRAY_START, CONTAINER_ARRAY
    PARSE_STATE_SCALAR,
    PARSE_STATE_ARRAY_START,
    PARSE_STATE_OBJECT_START,
    PARSE_STATE_ARRAY_END,
    // Valid states from PARSE_STATE_ARRAY_END, CONTAINER_ARRAY
    PARSE_STATE_COMMA, // 60
    PARSE_STATE_ARRAY_END,
    255,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 70
    0,
    // Valid states from PARSE_STATE_OBJECT_END, CONTAINER_ARRAY
    PARSE_STATE_COMMA,
    PARSE_STATE_ARRAY_END,
    255,
    0,
    0,
    0,
    0,
    0,
    0, // 80
    0,
    0,
    0,
    // Valid states from PARSE_STATE_SCALAR, CONTAINER_OBJECT
    PARSE_STATE_COMMA,
    PARSE_STATE_OBJECT_END,
    255,
    0,
    // Valid states from PARSE_STATE_COMMA, CONTAINER_OBJECT
    PARSE_STATE_OBJECT_KEY,
    255,
    0, // 90
    0,
    // Valid states from PARSE_STATE_COLON, CONTAINER_OBJECT
    PARSE_STATE_SCALAR,
    PARSE_STATE_ARRAY_START,
    PARSE_STATE_OBJECT_START,
    255,
    0,
    0,
    0,
    0,
    // Valid states from PARSE_STATE_ARRAY_END, CONTAINER_OBJECT
    PARSE_STATE_COMMA, // 100
    PARSE_STATE_OBJECT_END,
    255,
    0,
    // Valid states from PARSE_STATE_OBJECT_START, CONTAINER_OBJECT
    PARSE_STATE_OBJECT_KEY,
    PARSE_STATE_OBJECT_END,
    255,
    0,
    // Valid states from PARSE_STATE_OBJECT_KEY, CONTAINER_OBJECT
    PARSE_STATE_COLON,
    255,
    0, // 110
    0,
    // Valid states from PARSE_STATE_OBJECT_END, CONTAINER_OBJECT
    PARSE_STATE_COMMA,
    PARSE_STATE_OBJECT_END,
    255
};

// clang-format on

u8 getValidNewStateIndex(JsonParseState jsonParseState, Container container) {
	return ((container * 10) + jsonParseState) * 4;
}

typedef struct {
	RefString data;
	JsonTokenType tokenType;
} JsonToken;

typedef struct {
	String string;
	u64 stringPos;
	u64 stringLineNumber;
	// packed bits - 0 means array, 1 means object. Max depth is 127
	u8 containerStack[16];
	// -1 is initial position
	JsonParseState parseState;
	s8 containerStackPos;
} JsonStringReader;

JsonStringReader newJsonStringReader(String s) {
	JsonStringReader r = {
	    .string = s,
	    .stringPos = 0,
	    .parseState = PARSE_STATE_START,
	    .containerStackPos = -1,
	};
	return r;
}

Container getCurrentContainer(JsonStringReader const *r) {
	if (r->containerStackPos == -1) {
		return CONTAINER_NONE;
	} else {
		u8 byteNum = r->containerStackPos / 8;
		u8 inBytePos = r->containerStackPos % 8;
		return ((r->containerStack[byteNum] & (1 << inBytePos)) >> inBytePos) + 1;
	}
}

u8 isValidNewState(JsonStringReader const *r, JsonParseState newState) {
	Container currentContainer = getCurrentContainer(r);

	u64 i = getValidNewStateIndex(r->parseState, currentContainer);
	for (u8 j = 0; j < 4; j++) {
		if (validNewStates[i + j] == 255) {
			break;
		} else if (validNewStates[i + j] == newState) {
			return true;
		}
	}

	return false;
}

// findEndQuoteIndex finds the index of the closing quoteation mark character.
// If there is some error, this returns 0.
u64 findEndQuoteIndex(String s, u64 startIndex) {
	u64 i = startIndex;
	while (i < s.len) {
		u8 character = s.buffer[i];

		if (i == startIndex) {
			if (character == '"') {
				i++;
				continue;
			} else {
				return 0;
			}
		}

		if (character == '"') {
			u64 possibleEnd = i;
			while (1) {
				u8 previousIsSlash = s.buffer[i - 1] == '\\';
				if (previousIsSlash && i - 2 != startIndex && s.buffer[i - 2] == '\\') {
					i -= 2;
				} else if (previousIsSlash) {
					i = possibleEnd;
					break;
				} else {
					return possibleEnd;
				}
			}
		}

		i++;
	}

	return 0;
}

// findEndNumberIndex finds the index of the last character in a JSON number.
// This will eventually need to be replaced by a function which validates a
// number
u64 findEndNumberIndex(String s, u64 startIndex) {
	u64 i = startIndex;
	while (i < s.len) {
		u8 c = s.buffer[i];
		if (c >= '0' && c <= '9') {
			i++;
			continue;
		} else if (c == '.') {
			i++;
			continue;
		} else if (c == '-') {
			i++;
			continue;
		} else if (c == '+') {
			i++;
			continue;
		} else if (c == 'e') {
			i++;
			continue;
		} else if (c == 'E') {
			i++;
			continue;
		} else {
			break;
		}
	}

	return i;
}

u8 charArraysAreEqual(u8 *a, char *b, u64 len) {
	for (u64 i = 0; i < len; i++) {
		if (a[i] != b[i]) {
			return 0;
		}
	}
	return 1;
}

JsonToken jsonNext(JsonStringReader *r) {
	// TODO: give error details

	for (; r->stringPos < r->string.len; r->stringPos++) {
		u8 c = r->string.buffer[r->stringPos];

		switch (c) {
		case '{':
			if (r->containerStackPos == 127) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			JsonParseState newState = PARSE_STATE_OBJECT_START;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			r->containerStackPos++;
			u8 byteNum = r->containerStackPos / 8;
			u8 inBytePos = r->containerStackPos % 8;
			r->containerStack[byteNum] = r->containerStack[byteNum] | (1 << inBytePos);

			r->stringPos++;
			return (JsonToken){.tokenType = TOKEN_TYPE_OBJECT_START};
		case '}':
			newState = PARSE_STATE_OBJECT_END;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			r->containerStackPos--;

			r->stringPos++;
			return (JsonToken){.tokenType = TOKEN_TYPE_OBJECT_END};
		case '[':
			if (r->containerStackPos == 127) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			newState = PARSE_STATE_ARRAY_START;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			r->containerStackPos++;
			byteNum = r->containerStackPos / 8;
			inBytePos = r->containerStackPos % 8;
			r->containerStack[byteNum] = r->containerStack[byteNum] & ~(1 << inBytePos);

			r->stringPos++;
			return (JsonToken){.tokenType = TOKEN_TYPE_ARRAY_START};
		case ']':
			newState = PARSE_STATE_ARRAY_END;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			r->containerStackPos--;

			r->stringPos++;
			return (JsonToken){.tokenType = TOKEN_TYPE_ARRAY_END};
		case ':':
			newState = PARSE_STATE_COLON;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;
			break;
		case ',':
			newState = PARSE_STATE_COMMA;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;
			break;
		case '"':
			if (r->parseState == PARSE_STATE_OBJECT_START ||
			    (r->parseState == PARSE_STATE_COMMA && getCurrentContainer(r) == CONTAINER_OBJECT)) {
				newState = PARSE_STATE_OBJECT_KEY;
			} else {
				newState = PARSE_STATE_SCALAR;
			}

			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			u64 endQuoteIndex = findEndQuoteIndex(r->string, r->stringPos);
			if (endQuoteIndex == 0) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			u64 startPos = r->stringPos;
			r->stringPos = endQuoteIndex + 1;

			JsonTokenType stringTokenType;
			if (r->parseState == PARSE_STATE_OBJECT_KEY) {
				stringTokenType = TOKEN_TYPE_OBJECT_KEY;
			} else {
				stringTokenType = TOKEN_TYPE_STRING;
			}

			return (JsonToken){.tokenType = stringTokenType,
					   .data =
					       (RefString){.start = startPos + 1, .len = endQuoteIndex - startPos - 1}};
		case 't':
		case 'n':
			if (r->string.len < r->stringPos + 4) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}

			u8 *b = r->string.buffer + r->stringPos;
			JsonToken token;
			if (charArraysAreEqual(b, "true", 4)) {
				r->stringPos += 4;
				token = (JsonToken){.tokenType = TOKEN_TYPE_TRUE};
			} else if (charArraysAreEqual(b, "null", 4)) {
				r->stringPos += 4;
				token = (JsonToken){.tokenType = TOKEN_TYPE_NULL};
			} else {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}

			newState = PARSE_STATE_SCALAR;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			return token;

		case 'f':
			if (r->string.len < r->stringPos + 5) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}

			b = r->string.buffer + r->stringPos;
			if (charArraysAreEqual(b, "false", 5)) {
				r->stringPos += 5;
				newState = PARSE_STATE_SCALAR;
				if (!isValidNewState(r, newState)) {
					return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
				}
				r->parseState = newState;
				return (JsonToken){.tokenType = TOKEN_TYPE_FALSE};
			} else {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
		case '-':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			newState = PARSE_STATE_SCALAR;
			if (!isValidNewState(r, newState)) {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}
			r->parseState = newState;

			u64 endNumIndex = findEndNumberIndex(r->string, r->stringPos);

			// no starting numbers with 0 except if the number is
			// literally "0" or a number between 0 and 1
			if (c == '0' && endNumIndex - r->stringPos > 1 && r->string.buffer[r->stringPos + 1] != '.') {
				return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
			}

			startPos = r->stringPos;
			r->stringPos = endNumIndex;
			return (JsonToken){.tokenType = TOKEN_TYPE_NUMBER,
					   .data = (RefString){.start = startPos, .len = endNumIndex - startPos}};
		case ' ':
		case '\r':
		case '\t':
			break;
		case '\n':
			r->stringLineNumber++;
			break;
		default:
			return (JsonToken){.tokenType = TOKEN_TYPE_ERROR};
		}
	}

	return (JsonToken){.tokenType = TOKEN_TYPE_EOF};
}

internal void printWhitespace(u8 indentAmount, s8 indentLevel, JsonTokenType thisToken, JsonTokenType lastToken) {
	switch (lastToken) {
	case TOKEN_TYPE_STRING:
	case TOKEN_TYPE_NUMBER:
	case TOKEN_TYPE_TRUE:
	case TOKEN_TYPE_FALSE:
	case TOKEN_TYPE_NULL:
		if (thisToken != TOKEN_TYPE_ARRAY_END && thisToken != TOKEN_TYPE_OBJECT_END) {
			printChar(",");
		}
	default:
		break;
	}

	if (thisToken != TOKEN_TYPE_ARRAY_START && thisToken != TOKEN_TYPE_OBJECT_START) {
		indentLevel += 1;
	}
	if ((lastToken == TOKEN_TYPE_OBJECT_END || lastToken == TOKEN_TYPE_ARRAY_END) &&
	    thisToken != TOKEN_TYPE_ARRAY_END && thisToken != TOKEN_TYPE_OBJECT_END && indentAmount != 0) {
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
