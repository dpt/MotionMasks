/* regdata.h */

#ifndef SPAN_REGISTRY_REGDATA_H
#define SPAN_REGISTRY_REGDATA_H

#include "plot/span-rgbx8888.h"
#include "plot/span-xbgr8888.h"

/* FIXME: Register these spans at runtime to avoid linking all referenced
 * spans into the binary. */
static const span_t *spans[] =
{
  &span_rgbx8888,
  &span_xbgr8888
};

static const int nspans = 2;

#endif /* SPAN_REGISTRY_REGDATA_H */
