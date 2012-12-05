/* regdata.h */

#ifndef SPAN_REGISTRY_REGDATA_H
#define SPAN_REGISTRY_REGDATA_H

#include "utils/array.h"

#include "plot/span-bgrx8888.h"
#include "plot/span-rgbx8888.h"
#include "plot/span-xbgr8888.h"

/* FIXME: Register these spans at runtime to avoid linking all referenced
 * spans into the binary. */
static const span_t *spans[] =
{
  &span_bgrx8888,
  &span_rgbx8888,
  &span_xbgr8888,
};

static const int nspans = NELEMS(spans);

#endif /* SPAN_REGISTRY_REGDATA_H */
