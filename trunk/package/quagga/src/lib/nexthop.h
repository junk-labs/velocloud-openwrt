/*
 * Nexthop structure definition.
 * Copyright (C) 1997, 98, 99, 2001 Kunihiro Ishiguro
 * Copyright (C) 2013 Cumulus Networks, Inc.
 *
 * This file is part of Quagga.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _LIB_NEXTHOP_H
#define _LIB_NEXTHOP_H

#include "prefix.h"

/* Maximum next hop string length - gateway + ifindex */
#define NEXTHOP_STRLEN (INET6_ADDRSTRLEN + 30)

/* Nexthop label structure. */
struct nexthop_label
{
  u_int8_t num_labels;
  u_int8_t reserved[3];
};

extern int zebra_rnh_ip_default_route;
extern int zebra_rnh_ipv6_default_route;

static inline int
nh_resolve_via_default(int family)
{
  if (((family == AF_INET) && zebra_rnh_ip_default_route) ||
      ((family == AF_INET6) && zebra_rnh_ipv6_default_route))
    return 1;
  else
    return 0;
}

struct nexthop *nexthop_new (void);
void nexthop_add (struct nexthop **target, struct nexthop *nexthop);

void copy_nexthops (struct nexthop **tnh, struct nexthop *nh);
void nexthop_free (struct nexthop *nexthop);
void nexthops_free (struct nexthop *nexthop);

extern const char *nexthop_type_to_str (enum nexthop_types_t nh_type);
extern int nexthop_same_no_recurse (struct nexthop *next1, struct nexthop *next2);

extern const char * nexthop2str (struct nexthop *nexthop, char *str, int size);
#endif /*_LIB_NEXTHOP_H */
