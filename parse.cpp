//
// Created by zwy on 17-5-5.
//

#include <string.h>
#include <http_parser.h>
#include "parse.h"

int onUrlParsed(http_parser * parser, const char *at, size_t len){
    char *url = (char *)parser->data;
    strncpy(url, at, len > URL_LEN ? URL_LEN : len);
    url[len] = '\0';
    return 0;
}

int getUrl(char *buf, int n, char *url){
    http_parser_settings settings;
    http_parser headParser;

    http_parser_settings_init(&settings);
    http_parser_init(&headParser, HTTP_REQUEST);
    settings.on_url = onUrlParsed;
    headParser.data = url;

    http_parser_execute(&headParser, &settings, buf, n);

    return strlen(url);
}
