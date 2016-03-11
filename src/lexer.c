#include "lexer.h"

char *get_token_string(enum TokenType type) {
    switch(type) {
        case NUMBER: return "number";
        case LPAREN: return "lparen";
        case RPAREN: return "rparen";
        case EQUALS: return "equals";
        case LESS_THAN: return "less than";
        case GREATER_THAN: return "greater than";
        case PLUS: return "plus";
        case MINUS: return "minus";
        case MULTIPLY: return "multiply";
        case DIVIDE: return "divide";
        case BITWISE_AND: return "bitwise and";
        case BITWISE_OR: return "bitwise or";
        case BANG: return "bang";
        case DOT: return "dot";
        case LCURLYBRACE: return "lcurlybrace";
        case RCURLYBRACE: return "rcurlybrace";
        case MODULO: return "modulo";
        case AND: return "and";
        case NOT: return "not";
        case OR: return "or";
        case IF: return "if";
        case THEN: return "then";
        case ELSE: return "else";
        case ELIF: return "elif";
        case WHILE: return "while";
        case UNTIL: return "until";
        case FOR: return "for";
        case IN: return "in";
        case DO: return "do";
        case END: return "end";
        case VAR: return "var";
        case LET: return "let";
        case TYPE: return "type";
        case IMPORT: return "import";
        case FUNCTION: return "function";
        case RETURN: return "return";
        case COMMENT: return "comment";
        case STRING: return "string";
        case IDENTIFIER: return "identifier";
        case END_OF_LINE: return "end of line";
        case END_OF_FILE: return "end of file";
        case ERROR_TOKEN: return "error token";
        default: return "unknown";
    }
}

// Construct a token.
struct Token build_token(enum TokenType type, long position, long length) {
    struct Token t = { .type = type, .position = position, .length = length, .error = NULL, .value = NULL };
    return t;
}

// Construct a token with a string.
struct Token build_token_string(enum TokenType type, long position, long length, char *value) {
    struct Token t = { .type = type, .position = position, .length = length, .error = NULL, .value = strdup(value) };
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
    while (next == ' ' || next == '\t') {
        position = ftell(file);
        next = fgetc(file);
    }

    // If the next two characters are hyphens ('-'), we
    // should include the rest of the line in a token,
    // except for the newline character.
    //
    // We don't include the newline character because it may
    // terminate a statement preceding the comment.
    if (next == '-') {

        char after_next = fgetc(file);
        if (after_next == '-') {

            // Skip to the end of the line or EOF.
            long length = 2;
            while (next != '\n' && next != EOF) {
                length++;
                next = fgetc(file);
            }

            return build_token(COMMENT, position, length);

        } else {

            // Return to where we were otherwise.
            ungetc(after_next, file);
        }
    }

    // Possible contents for SYMBOL tokens.
    static const char *SYMBOLS = "()[]=<>+-*/&|!.{}%";

    // Buffer for identifiers / strings
    char buffer[256] = {0};
    // Buffer location / pointer
    int bp = 0;

    // Check for single-character tokens, then symbols.
    if (next == EOF || next == '\0') {
        return build_token(END_OF_FILE, position, 1);
    } else if (next == ';' || next == '\n') {
        return build_token(END_OF_LINE, position, 1);
    } else if (strchr(SYMBOLS, next) != NULL) {
        #define case_sym(character, type) case character: return build_token(type, position, 1); break;
        switch (next) {
            case_sym('(', LPAREN)
            case_sym(')', RPAREN)
            case_sym('=', EQUALS)
            case_sym('<', LESS_THAN)
            case_sym('>', GREATER_THAN)
            case_sym('+', PLUS)
            case_sym('-', MINUS)
            case_sym('*', MULTIPLY)
            case_sym('/', DIVIDE)
            case_sym('&', BITWISE_AND)
            case_sym('|', BITWISE_OR)
            case_sym('!', BANG)
            case_sym('.', DOT)
            case_sym('{', LCURLYBRACE)
            case_sym('}', RCURLYBRACE)
            case_sym('%', MODULO)
        }
        #undef case_sym
    }

    // Strings
    if (next == '"') {

        next = fgetc(file);
        while (next != '"' && next != EOF) {
            buffer[bp++] = next;
            next = fgetc(file);
        }

        // If we encountered an EOF while searching for
        // the matching quotation mark, we have an
        // unterminated string.
        long end = ftell(file);
        if (next == EOF) {
            return error_token("unterminated string", position, end - position);
        } else {
            return build_token_string(STRING, position, end - position, buffer);
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

    // Keywords are specific, predefined strings that have
    // a corresponding token type.
    #define check_keyword(string, type) if (strcmp(word, string) == 0) { return build_token(type, position, length); }
        check_keyword("and", AND)
        else check_keyword("not", NOT)
        else check_keyword("or",  OR)
        else check_keyword("if", IF)
        else check_keyword("then", THEN)
        else check_keyword("else", ELSE)
        else check_keyword("elif", ELIF)
        else check_keyword("while", WHILE)
        else check_keyword("until", UNTIL)
        else check_keyword("for", FOR)
        else check_keyword("in", IN)
        else check_keyword("do", DO)
        else check_keyword("end", END)
        else check_keyword("var", VAR)
        else check_keyword("let", LET)
        else check_keyword("type", TYPE)
        else check_keyword("import", IMPORT)
        else check_keyword("function", FUNCTION)
        else check_keyword("return", RETURN)
    #undef check_keyword

    // Identifiers start with a letter or an underscore,
    // optionally followed by letters, digits and underscores.
    if (word[0] == '_' || isalpha(word[0])) {
        buffer[0] = word[0];

        // Ensure that there are no invalid characters in the
        // identifier.
        for (int i = 1; i < length; i++) {
            buffer[i] = word[i];
            if (word[i] != '_' && !isalpha(word[i]) && !isdigit(word[i])) {
                return error_token("identifier contains an invalid character", position, length);
            }
        }

        return build_token_string(IDENTIFIER, position, length, buffer);
    }

    return error_token("unidentified token", position, length);
}

struct Token* lexer_scan(const char *path, long *num_tokens) {

    // Attempt to open the file.
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error while opening '%s': %s\n", path, strerror(errno));
        return NULL;
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

    *num_tokens = 0;
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
        } else if (t.type == COMMENT) {

            // Ignore comments completely.
            i--;
            continue;
        } else {
            tokens[i] = t;
            (*num_tokens)++;

            if (t.type == END_OF_FILE) {
                break;
            }
        }
    }

    fclose(file);

    // Reallocate the array to the real number of tokens.
    tokens = realloc(tokens, (*num_tokens) * sizeof(struct Token));

    return tokens;
}
