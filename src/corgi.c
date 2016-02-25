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
        SYMBOL,
        KEYWORD,

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
};

// Possible contents for KEYWORD tokens.
#define NUM_KEYWORDS 15
const char *KEYWORDS[NUM_KEYWORDS] = {
    "and", "or", "not",
    "if", "else", "elif", "end",
    "while", "import", "function",
    "until", "type", "return", "var", "let"
};

// Possible contents for SYMBOL tokens.
const char *SYMBOLS = "()[]=<>;+-*/&|!";

// Construct a token.
struct Token build_token(enum TokenType type, long position, long length) {
    struct Token t = { .type = type, .position = position, .length = length, .error = NULL };
    return t;
}

// Construct an error token.
struct Token error_token(const char *message, long position, long length) {
    struct Token t = { .type = ERROR_TOKEN, .position = position, .length = length, .error = message };
    return t;
}

// Read the next token from a file handle and return it.
// The file handle will be moved to after the end of
// the token, unless it's EOF.
struct Token read_token(FILE *file) {

    // Remember the position of the next character before
    // calling fgetc().
    long position = ftell(file);
    char next = fgetc(file);

    // Ignore whitespace, but not new-line characters.
    while (next == ' ') {
        position = ftell(file);
        next = fgetc(file);
    }

    // Single-character tokens
    if (next == EOF || next == '\0') {
        return build_token(END_OF_FILE, position, 1);
    } else if (next == ';' || next == '\n') {
        return build_token(END_OF_LINE, position, 1);
    } else if (strchr(SYMBOLS, next) != NULL) {
        return build_token(SYMBOL, position, 1);
    }

    // Strings
    if (next == '"') {

        next = fgetc(file);
        while (next != '"' && next != EOF) {
            next = fgetc(file);
        }

        // If we encountered an EOF while searching for
        // the matching quotation mark, we have an
        // unterminated string.
        long end = ftell(file);
        if (next == EOF) {
            return error_token("unterminated string", position, end - position);
        } else {
            return build_token(STRING, position, end - position);
        }
    }

    // Tokens that consist of multiple characters (such as numbers)
    // are terminated by whitespace, symbols, or EOF.
    //
    // Determine where the token ends by reading characters
    // repeatedly until we reach one of these delimiters.
    while (next != EOF && !isspace(next) && strchr(SYMBOLS, next) == NULL) {
        next = fgetc(file);
    }

    // Remember where the last character was positioned.
    long end = ftell(file) - 1;

    // Determine the type of the token by reading the text into a
    // string, and checking its contents.
    long length = end - position;
    fseek(file, position, SEEK_SET);
    char word[length + 1];
    fread(word, sizeof(char), length, file);
    word[length] = '\0';

    // Numbers always start with a digit.
    if (isdigit(word[0])) {
        return build_token(NUMBER, position, length);
    }

    // Keywords are always in the KEYWORDS array.
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(KEYWORDS[i], word) == 0) {
            return build_token(KEYWORD, position, length);
        }
    }

    // Identifiers start with a letter or an underscore,
    // optionally followed by letters, digits and underscores.
    if (word[0] == '_' || isalpha(word[0])) {

        // Ensure that there are no invalid characters in the
        // identifier.
        for (int i = 1; i < length; i++) {
            if (word[i] != '_' && !isalpha(word[i]) && !isdigit(word[i])) {
                return error_token("identifier contains an invalid character", position, length);
            }
        }

        return build_token(IDENTIFIER, position, length);
    }

    return error_token("unidentified token", position, length);
}

int main(int argc, char **argv)  {

    // For now, we just want the path to the file to
    // compile, and nothing else.
    if (argc != 2) {
        printf("Usage: %s source-file.cg\n", argv[0]);
        return 0;
    }

    // Attempt to open the file.
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error while opening '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    // Build a list of tokens from its contents.
    // Since there is no way to know how many tokens there are, we will
    // assume that the file was filled with single-character tokens,
    // which means that the number of tokens would be equal to the size of
    // the file.
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    struct Token *tokens = malloc(size * sizeof(struct Token));

    long num_tokens = 0;
    for (long i = 0; i < size; i++) {
        struct Token t = read_token(file);

        // Print errors, but don't stop looking for tokens.
        // There might be more errors, in which case it would
        // be easier for the user to fix multiple errors at
        // once.
        // Of course, the existence of an error in the source code
        // may cause more as we read on, which is why it may
        // be good to exit here.
        if (t.type == ERROR_TOKEN) {

            // Read the token from the file, for a better message.
            char text[t.length + 1];
            fseek(file, t.position, SEEK_SET);
            fread(text, sizeof(char), t.length, file);
            text[t.length] = '\0';

            // Report the error.
            printf(" Error: %s\n", t.error);
            printf("  while reading %li: '%s'\n", t.position, text);
        } else {
            tokens[i] = t;
            num_tokens++;

            if (t.type == END_OF_FILE) {
                break;
            }
        }
    }

    // Reallocate the array to the real number of tokens.
    tokens = realloc(tokens, num_tokens * sizeof(struct Token));

    // Clean up and return.
    free(tokens);
    fclose(file);

    return 0;
}
