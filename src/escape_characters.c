#include "server.h"

static char* HTTP_DecodeURL(char* url) {
    char* decoded_url = malloc(strlen(url)+1);
    memset(decoded_url, 0, strlen(url)+1);

    int url_index = 0;
    int decoded_index = 0;
    for (;url_index < strlen(url); url_index++, decoded_index++) {
        if (url[url_index] == '%') {
            char escape_character;
            char escape_code[3] = {0};
            escape_code[0] = url[url_index+1];
            escape_code[1] = url[url_index+2];
            url_index += 2;

            // Matching escape character
            if (!strcmp(escape_code, "20")) {
                escape_character = ' ';
            }
            else if (!strcmp(escape_code, "3C")) {
                escape_character = '<';
            }
            else if (!strcmp(escape_code, "3E")) {
                escape_character = '>';
            }
            else if (!strcmp(escape_code, "23")) {
                escape_character = '#';
            }
            else if (!strcmp(escape_code, "25")) {
                escape_character = '%';
            }
            else if (!strcmp(escape_code, "2B")) {
                escape_character = '+';
            }
            else if (!strcmp(escape_code, "7B")) {
                escape_character = '{';
            }
            else if (!strcmp(escape_code, "7D")) {
                escape_character = '}';
            }
            else if (!strcmp(escape_code, "7C")) {
                escape_character = '|';
            }
            else if (!strcmp(escape_code, "5C")) {
                escape_character = '\\';
            }
            else if (!strcmp(escape_code, "5E")) {
                escape_character = '^';
            }
            else if (!strcmp(escape_code, "7E")) {
                escape_character = '~';
            }
            else if (!strcmp(escape_code, "5B")) {
                escape_character = '[';
            }
            else if (!strcmp(escape_code, "5D")) {
                escape_character = ']';
            }
            else if (!strcmp(escape_code, "60")) {
                escape_character = '‘';
            }
            else if (!strcmp(escape_code, "3B")) {
                escape_character = ';';
            }
            else if (!strcmp(escape_code, "2F")) {
                escape_character = '/';
            }
            else if (!strcmp(escape_code, "3F")) {
                escape_character = '?';
            }
            else if (!strcmp(escape_code, "3A")) {
                escape_character = ':';
            }
            else if (!strcmp(escape_code, "40")) {
                escape_character = '@';
            }
            else if (!strcmp(escape_code, "3D")) {
                escape_character = '=';
            }
            else if (!strcmp(escape_code, "26")) {
                escape_character = '&';
            }
            else if (!strcmp(escape_code, "24")) {
                escape_character = '$';
            }

            decoded_url[decoded_index] = escape_character;
        }
        else {
            decoded_url[decoded_index] = url[url_index];
        }
    }


    return decoded_url;
}