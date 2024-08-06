#pragma once

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <re.h>
#include <cJSON.h>


// Macros
#define internal static
#define global static
#define local_persist static

#define ArrayCount(array) sizeof(array)/sizeof(array[0])
