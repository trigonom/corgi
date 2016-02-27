#ifndef lexer_h
#define lexer_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

// A token holds the smallest possible "chunk" of code, such
// as a symbol or a number.
struct Token {

    // The type of the token.
    enum TokenType {
        NUMBER,

        // Symbols
        LPAREN,
        RPAREN,
        EQUALS,
        LESS_THAN,
        GREATER_THAN,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        BITWISE_AND,
        BITWISE_OR,
        BANG,
        DOT,
        LCURLYBRACE,
        RCURLYBRACE,
        MODULO,

        // Keywords
        AND,
        NOT,
        OR,
        IF,
        THEN,
        ELSE,
        ELIF,
        WHILE,
        UNTIL,
        FOR,
        IN,
        DO,
        END,
        VAR,
        LET,
        TYPE,
        IMPORT,
        FUNCTION,
        RETURN,

        // Two hyphens ('-'), followed by any characters
        // until the end of the current line.
        COMMENT,

        // Any string of characters surrounded by
        // double quotes (""). Escape characters
        // are not supported as of yet.
        STRING,

        // An identifier always starts with a letter
        // or an underscore, optionally followed by any
        // number of letters, numbers or underscores.
        IDENTIFIER,

        // A semicolon or newline character.
        END_OF_LINE,

        // The end of the file, or a null character.
        END_OF_FILE,

        // Used for error-checking.
        ERROR_TOKEN
    } TokenType;

    enum TokenType type;

    // A token may store its metadata, or just a pointer to
    // an error message.
    // The absolute offset from the start of the file where
    // the token starts.
    long position;

    // The length of the token.
    long length;

    // An error string, if this is an ERROR_TOKEN.
    const char *error;

    // Value for strings and identifiers
    char *value;
};

struct Token build_token(enum TokenType type, long position, long length);
struct Token build_token_string(enum TokenType type, long position, long length, char *value);
struct Token error_token(const char *message, long position, long length);

struct Token* lexer_scan(const char *path, long *num_tokens);

#endif
