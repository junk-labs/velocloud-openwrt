/*
 * VRF related header.
 * Copyright (C) 2014 6WIND S.A.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ZEBRA_VRF_H
#define _ZEBRA_VRF_H

#include "prefix.h"
#include "table.h"
#include "queue.h"

union g_addr {
  struct in_addr ipv4;
#ifdef HAVE_IPV6
  struct in6_addr ipv6;
#endif /* HAVE_IPV6 */
};


/* The default VRF ID */
#define VRF_DEFAULT 0
#define VRF_UNKNOWN UINT16_MAX

/*
 * VRF hooks
 */

#define VRF_NEW_HOOK        0   /* a new VRF is just created */
#define VRF_DELETE_HOOK     1   /* a VRF is to be deleted */
#define VRF_ENABLE_HOOK     2   /* a VRF is ready to use */
#define VRF_DISABLE_HOOK    3   /* a VRF is to be unusable */

/* Routing table instance.  */
struct vrf
{
  /* Identifier.  This is same as routing table vector index.  */
  u_int32_t id;

  /* Routing table name.  */
  char *name;

  /* Description.  */
  char *desc;

  /* FIB identifier.  */
  u_char fib_id;

  /* Routing table.  */
  struct route_table *table[AFI_MAX][SAFI_MAX];

  /* Static route configuration.  */
  struct route_table *stable[AFI_MAX][SAFI_MAX];
  
  /* Master list of interfaces belonging to this VRF */
  struct list *iflist;

  /* Zebra internal VRF status */
  u_char status;
#define VRF_ACTIVE     (1 << 0)
  
  /* User data */
  void *info;
};

/*
 * Add a specific hook to VRF module.
 * @param1: hook type
 * @param2: the callback function
 *          - param 1: the VRF ID
 *          - param 2: the address of the user data pointer (the user data
 *                     can be stored in or freed from there)
 */
extern void vrf_add_hook (int, int (*)(vrf_id_t, const char *, void **));
extern void vrf_init (void);
extern struct list *
vrf_iflist (vrf_id_t vrf_id);
extern void *
vrf_info_lookup (vrf_id_t vrf_id);
extern void
vrf_iflist_create (vrf_id_t vrf_id);
extern void
vrf_iflist_terminate (vrf_id_t vrf_id);
extern struct vrf *vrf_lookup (u_int32_t);


#endif /*_ZEBRA_VRF_H*/

