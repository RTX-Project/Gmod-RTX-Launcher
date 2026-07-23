// JsonValue.h - минимальный JSON-парсер без внешних зависимостей.
// Достаточен для разбора ответов GitHub REST API (releases/latest).
// Поддерживает: object, array, string, number, bool, null.
#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <cctype>

class JsonValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Type type = Type::Null;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;

    bool isObject() const { return type == Type::Object; }
    bool isArray()  const { return type == Type::Array; }
    bool isString() const { return type == Type::String; }

    // Доступ к полю объекта; возвращает пустой JsonValue(Null), если не найдено.
    const JsonValue& operator[](const std::string& key) const {
        static JsonValue nullVal;
        if (type != Type::Object) return nullVal;
        auto it = objectValue.find(key);
        if (it == objectValue.end()) return nullVal;
        return it->second;
    }

    std::string asString(const std::string& def = "") const {
        return type == Type::String ? stringValue : def;
    }
    long long asInt64(long long def = 0) const {
        return type == Type::Number ? static_cast<long long>(numberValue) : def;
    }

    static JsonValue parse(const std::string& text) {
        size_t pos = 0;
        JsonValue v = parseValue(text, pos);
        return v;
    }

private:
    static void skipWs(const std::string& s, size_t& pos) {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) pos++;
    }

    static JsonValue parseValue(const std::string& s, size_t& pos) {
        skipWs(s, pos);
        if (pos >= s.size()) throw std::runtime_error("JSON: unexpected end");
        char c = s[pos];
        if (c == '{') return parseObject(s, pos);
        if (c == '[') return parseArray(s, pos);
        if (c == '"') return parseString(s, pos);
        if (c == 't' || c == 'f') return parseBool(s, pos);
        if (c == 'n') { pos += 4; JsonValue v; v.type = Type::Null; return v; }
        return parseNumber(s, pos);
    }

    static JsonValue parseObject(const std::string& s, size_t& pos) {
        JsonValue v; v.type = Type::Object;
        pos++; // '{'
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == '}') { pos++; return v; }
        while (true) {
            skipWs(s, pos);
            JsonValue key = parseString(s, pos);
            skipWs(s, pos);
            if (pos >= s.size() || s[pos] != ':') throw std::runtime_error("JSON: expected ':'");
            pos++;
            JsonValue val = parseValue(s, pos);
            v.objectValue[key.stringValue] = val;
            skipWs(s, pos);
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            if (pos < s.size() && s[pos] == '}') { pos++; break; }
            throw std::runtime_error("JSON: expected ',' or '}'");
        }
        return v;
    }

    static JsonValue parseArray(const std::string& s, size_t& pos) {
        JsonValue v; v.type = Type::Array;
        pos++; // '['
        skipWs(s, pos);
        if (pos < s.size() && s[pos] == ']') { pos++; return v; }
        while (true) {
            JsonValue val = parseValue(s, pos);
            v.arrayValue.push_back(val);
            skipWs(s, pos);
            if (pos < s.size() && s[pos] == ',') { pos++; continue; }
            if (pos < s.size() && s[pos] == ']') { pos++; break; }
            throw std::runtime_error("JSON: expected ',' or ']'");
        }
        return v;
    }

    static JsonValue parseString(const std::string& s, size_t& pos) {
        JsonValue v; v.type = Type::String;
        if (s[pos] != '"') throw std::runtime_error("JSON: expected '\"'");
        pos++;
        std::string out;
        while (pos < s.size() && s[pos] != '"') {
            char c = s[pos];
            if (c == '\\' && pos + 1 < s.size()) {
                char n = s[pos + 1];
                switch (n) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case 'b': out += '\b'; break;
                    case 'f': out += '\f'; break;
                    case 'u': {
                        // упрощённая обработка \uXXXX -> берём только low byte (ASCII-safe для наших полей)
                        if (pos + 5 < s.size()) {
                            std::string hex = s.substr(pos + 2, 4);
                            unsigned int code = static_cast<unsigned int>(std::stoul(hex, nullptr, 16));
                            if (code < 0x80) out += static_cast<char>(code);
                            else out += '?';
                            pos += 4;
                        }
                        break;
                    }
                    default: out += n; break;
                }
                pos += 2;
            } else {
                out += c;
                pos++;
            }
        }
        if (pos >= s.size()) throw std::runtime_error("JSON: unterminated string");
        pos++; // closing '"'
        v.stringValue = out;
        return v;
    }

    static JsonValue parseBool(const std::string& s, size_t& pos) {
        JsonValue v; v.type = Type::Bool;
        if (s.compare(pos, 4, "true") == 0) { v.boolValue = true; pos += 4; }
        else if (s.compare(pos, 5, "false") == 0) { v.boolValue = false; pos += 5; }
        else throw std::runtime_error("JSON: invalid literal");
        return v;
    }

    static JsonValue parseNumber(const std::string& s, size_t& pos) {
        JsonValue v; v.type = Type::Number;
        size_t start = pos;
        if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) pos++;
        while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])) ||
               s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E' || s[pos] == '-' || s[pos] == '+')) pos++;
        v.numberValue = std::stod(s.substr(start, pos - start));
        return v;
    }
};
