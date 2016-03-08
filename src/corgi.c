#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

int main(int argc, char **argv) {
    // For now, we just want the path to the file to
    // compile, and nothing else.
    if (argc != 2) {
        printf("Usage: %s source-file.cg\n", argv[0]);
        return 0;
    }

	long num_tokens = 0;
	struct Token *tokens = lexer_scan(argv[1], &num_tokens);
	if(tokens == NULL) {
		printf("Failed to read source file\n");
		return 1;
	}

	for(long i = 0; i < num_tokens; i++) {
        struct Token t = tokens[i];
        printf("%d", t.type);

        if(t.value != NULL) {
            printf(" (%s)", t.value);
            free(t.value);
        }
        printf("\n");
    }

	free(tokens);

	return 0;
}
