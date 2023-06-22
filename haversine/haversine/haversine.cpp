#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

struct Buffer {
    size_t count;
    u8* data;

    bool InBounds(u64 i) { return i < count; }

    u8& operator[](int i) { return data[i]; }
    const u8& operator[](int i) const { return data[i]; }

    bool operator==(const Buffer& other) {
        if (count != other.count) return false;

        for (int i = 0; i < count; i++) {
            if (data[i] != other.data[i]) { return false; }
        }

        return true;
    }
};

Buffer AllocateBuffer(size_t size) {
    Buffer result = {};
    result.data = (u8*)malloc(size);
    if (result.data) {
        result.count = size;
    }
    else {
        fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", size);
    }

    return result;
}

void FreeBuffer(Buffer* buffer) {
    if (buffer->data) {
        free(buffer->data);
    }
    *buffer = {};
}

template <size_t length>
Buffer StringBuffer(const char(&string)[length]) {
    return { .count = length, .data = string };
}

struct JSON {
    Buffer source;
    u64 at;
    bool hadError;

    void Error(JSON_Token token, const char* message) {
        hadError = true;
        fprintf(stderr, "ERROR: \"%.*s\" - %s\n", (u32)token.value.count, (char*)token.value.data, message);
    }
};

struct JSON_Element {
    Buffer label;
    Buffer value;

    JSON_Element* next;
};

struct JSON_Token {
    enum Type {
        EndOfStream,
        Error,

        OpenBrace,
        OpenBracket,
        CloseBrace,
        CloseBracket,
        Comma,
        Colon,
        SemiColon,
        StringLiteral,
        Number,
        True,
        False,
        Null,

        Count
    };

    Type type;
    Buffer value;
};

bool CanParse(JSON* json) {
    return json->source.InBounds(json->at);
}

JSON_Element* ParseJSONList(JSON* json, JSON_Token startToken, JSON_Token::Type endToken, bool hasLabels) {
    JSON_Element* first = {};
    JSON_Element* last = {};

    while (CanParse(json)) {
        Buffer label = {};
        JSON_Token token = GetToken(json);

        if(hasLabels) {
            if (token.type == JSON_Token::StringLiteral) {
                label = token.value;

                JSON_Token colon = GetToken(json);
                if (colon.type == JSON_Token::Colon) {
                    token = GetToken(json);
                }
                else {
                    json->Error(colon, "Expected colon after field name");
                }
            }
            else if (token.type != endToken) {
                json->Error(token, "Unexpected token");
            }
        }

        JSON_Element* element = ParseJSONElement(json, label, token);
        if (element) {
            last = (last ? last->next : first) = element;
        }
        else if (token.type == endToken) {
            break;
        }
        else {
            json->Error(token, "Unexpected token");
        }

        JSON_Token comma = GetToken(json);
        if (comma.type == endToken) {
            break;
        }
        else if (comma.type != JSON_Token::Comma) {
            json->Error(comma, "Unexpected token");
        }
    }

    return first;
}

JSON_Element* ParseJSONElement(JSON* json, Buffer label, JSON_Token token) {
    bool valid = true;

    JSON_Element* next = nullptr;
    if (token.type == JSON_Token::OpenBracket) {
        next = ParseJSONList(json, token, JSON_Token::CloseBracket, false);
    }
    else if (token.type == JSON_Token::OpenBrace) {
        next = ParseJSONList(json, token, JSON_Token::CloseBrace, true);
    }
    else if (  token.type == JSON_Token::StringLiteral
            || token.type == JSON_Token::True
            || token.type == JSON_Token::False
            || token.type == JSON_Token::Null
            || token.type == JSON_Token::Number) {
        // no additional data
    }
    else {
        valid = false;
    }

    JSON_Element* result = nullptr;

    if (valid) {
        result = (JSON_Element*)malloc(sizeof(JSON_Element));
        assert(result);
        *result = {
            .label = label,
            .value = token.value,
            .next = next
        };
    }

    return result;
}

bool IsWhitespace(Buffer source, u64 at) {
    if (source.InBounds(at)) {
        u8 c = source[at];
        return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
    }
}

bool IsDigit(Buffer source, u64 at) {
    if (source.InBounds(at)) {
        u8 c = source[at];
        return (c >= '0') && (c <= '9');
    }
}

void ParseKeyword(Buffer source, u64* at, Buffer remaining, JSON_Token::Type type, JSON_Token* result) {
    if ((source.count - *at) >= remaining.count) {
        Buffer check = { .count = remaining.count, .data = source.data + *at };
        if (remaining == check) {
            result->type = type;
            result->value.count = remaining.count;
            *at += remaining.count;
        }
    }
}

void ParseStringLiteral(Buffer source, u64* at, JSON_Token* result) {
    result->type = JSON_Token::StringLiteral;
    u64 stringStart = *at;

    while (source.InBounds(*at) && source[*at] != '"') {
        if (source.InBounds(*at + 1) && source[*at] == '\\' && source[*at + 1] == '"') {
            (*at)++;
        }
        (*at)++;
    }

    result->value = {
        .count = *at - stringStart,
        .data = source.data + stringStart
    };

    if (source.InBounds(*at)) (*at)++;
}

void ParseNumber(u8 c, Buffer source, u64* at, JSON_Token* result) {
    u64 numberStart = *at - 1;
    result->type = JSON_Token::Number;

    if (c == '-' && source.InBounds(*at)) {
        c = source[(*at)++];
    }

    if (c != '0') {
        while (IsDigit(source, *at)) { (*at)++; }
    }

    if (source.InBounds(*at) && source[*at] == '.') {
        at++;
        while (IsDigit(source, *at)) { (*at)++; }
    }

    // TODO: scientific notation

    result->value.count = *at - numberStart;
}

JSON_Token GetToken(JSON* json) {
    JSON_Token result = {};

    Buffer source = json->source;
    u64 at = json->at;

    while (IsWhitespace(source, at)) { at++; }

    if (source.InBounds(at)) {
        result = {
            .type = JSON_Token::Error,
            .value = {
                .count = 1, 
                .data = source.data + at
            }
        };

        u8 c = source[at++];
        switch (c) {
        case '{': result.type = JSON_Token::OpenBrace;      break;
        case '[': result.type = JSON_Token::OpenBracket;    break;
        case '}': result.type = JSON_Token::CloseBracket;   break;
        case ']': result.type = JSON_Token::CloseBrace;     break;
        case ',': result.type = JSON_Token::Comma;          break;
        case ':': result.type = JSON_Token::Colon;          break;
        case ';': result.type = JSON_Token::SemiColon;      break;

        case 'f': ParseKeyword(source, &at, StringBuffer("alse"), JSON_Token::False, &result);  break;
        case 'n': ParseKeyword(source, &at, StringBuffer("ull"), JSON_Token::Null, &result);    break;
        case 't': ParseKeyword(source, &at, StringBuffer("rue"), JSON_Token::True, &result);    break;
        case '"': ParseStringLiteral(source, &at, &result);                                     break;

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
        case '9': ParseNumber(c, source, &at, &result); break;

        default: break;
        }
    }

    json->at = at;
    return result;
}

JSON_Element* ParseJSON(Buffer input) {
    JSON json = { input, 0 };
    JSON_Element* result = ParseJSONElement(&json, {}, GetToken(&json));
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: haversine_generator [haversine_input.json]");
        return 0;
    }

    FILE* fp = fopen(argv[1], "rb");

    Buffer input = {};

    if (fp) {
        fseek(fp, 0, SEEK_END);
        u64 size = ftell(fp);
        rewind(fp);

        input = AllocateBuffer(size);

        if (input.data) {
            if (fread(input.data, input.count, 1, fp) != 1) {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", argv[1]);
                FreeBuffer(&input);
            }
        }

        fclose(fp);
    }
    else {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", argv[1]);
    }

    JSON_Element* json = ParseJSON(input);

    return 0;
}
