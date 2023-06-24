#pragma once

struct JSON_Element {
    Buffer label;
    Buffer value;
    JSON_Element* firstSubElement;
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

struct JSON {
    Buffer source;
    u64 at;
    bool hadError;

    void Error(JSON_Token token, const char* message) {
        hadError = true;
        fprintf(stderr, "ERROR: \"%.*s\" - %s\n", (u32)token.value.count, (char*)token.value.data, message);
    }
};

bool CanParse(JSON* json) {
    return json->source.InBounds(json->at);
}

bool IsWhitespace(Buffer source, u64 at) {
    if (source.InBounds(at)) {
        u8 c = source[at];
        return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
    }
    return false;
}

bool IsDigit(Buffer source, u64 at) {
    if (source.InBounds(at)) {
        u8 c = source[at];
        return (c >= '0') && (c <= '9');
    }
    return false;
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
        (*at)++;
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
        case '}': result.type = JSON_Token::CloseBrace;     break;
        case ']': result.type = JSON_Token::CloseBracket;   break;
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

JSON_Element* ParseJSONElement(JSON* json, Buffer label, JSON_Token token);
JSON_Element* ParseJSONList(JSON* json, JSON_Token startToken, JSON_Token::Type endToken, bool hasLabels) {
    JSON_Element* first = {};
    JSON_Element* last = {};

    while (CanParse(json)) {
        Buffer label = {};
        JSON_Token token = GetToken(json);

        if (hasLabels) {
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

    JSON_Element* subElement = nullptr;
    if (token.type == JSON_Token::OpenBracket) {
        subElement = ParseJSONList(json, token, JSON_Token::CloseBracket, false);
    }
    else if (token.type == JSON_Token::OpenBrace) {
        subElement = ParseJSONList(json, token, JSON_Token::CloseBrace, true);
    }
    else if (token.type == JSON_Token::StringLiteral
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
            .firstSubElement = subElement,
            .next = nullptr
        };
    }

    return result;
}

JSON_Element* ParseJSON(Buffer input) {
    JSON json = { input, 0 };
    JSON_Element* result = ParseJSONElement(&json, {}, GetToken(&json));
    return result;
}

void FreeJSON(JSON_Element* element) {
    while (element) {
        JSON_Element* freeElement = element;
        element = element->next;

        FreeJSON(freeElement->firstSubElement);
        free(freeElement);
    }
}

JSON_Element* LookupElement(JSON_Element* object, Buffer label) {
    JSON_Element* result = nullptr;

    if (object) {
        for (JSON_Element* search = object->firstSubElement; search; search = search->next) {
            if (search->label == label) {
                result = search;
                break;
            }
        }
    }

    return result;
}

f64 ConvertElementToF64(JSON_Element* object, Buffer label) {
    f64 result = 0.0;

    JSON_Element* element = LookupElement(object, label);
    if (element) {
        Buffer source = element->value;
        u64 at = 0;

        f64 sign = 1.0;
        if (source.InBounds(at) && source[at] == '-') {
            sign = -1.0;
            at++;
        }

        f64 number = 0.0;
        while (source.InBounds(at)) {
            u8 Char = source[at] - (u8)'0';
            if (Char >= 0 && Char < 10) {
                number = 10.0 * number + (f64)Char;
                at++;
            }
            else {
                break;
            }
        }

        if (source.InBounds(at) && source[at] == '.') {
            at++;
            f64 C = 1.0 / 10.0;
            while (source.InBounds(at)) {
                u8 Char = source[at] - (u8)'0';
                if (Char >= 0 && Char < 10) {
                    number = number + C * (f64)Char;
                    C *= 1.0 / 10.0;
                    at++;
                }
                else {
                    break;
                }
            }
        }

        result = sign * number;
    }

    return result;
}