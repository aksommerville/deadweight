#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "serial.h"
#include "fs.h"
#include "image.h"

extern struct g {
  const char *palpath;
  const char **srcpathv;
  int srcpathc,srcpatha;
  uint32_t colorv[64]; // RGBA, R in MSB. A is always 0xff.
  int colorc; // 0..64; if 0 we are not validating specific colors.
} g;

#endif
