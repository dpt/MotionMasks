/* codes.c -- Motion Mask byte encoding */

#include "codes.h"

const char *MMCodeNames[MMID_LIMIT] =
{
  "MMCopy",
  "MMBlendConst",
  "MMBlendArray",
  "MMCopyLong",
  "MMBlendConstLong",
  "MMBlendArrayLong",
  "MMSetSource",
  "MMStop",
};
