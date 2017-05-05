//
// Created by zwy on 17-5-5.
//

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void error(const char * err){
    perror(err);
    exit(1);
}
