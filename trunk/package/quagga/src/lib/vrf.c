/*
 * VRF functions.
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

#include <zebra.h>

#include "prefix.h"
#include "table.h"
#include "memory.h"
#include "str.h"
#include "command.h"
#include "if.h"
#include "log.h"
#include "sockunion.h"
#include "linklist.h"
#include "thread.h"
#include "workqueue.h"
#include "prefix.h"
#include "routemap.h"
#include "vrf.h"

#include "zebra/rib.h"

/*
 * Turn on/off debug code
 * for vrf.
 */
int debug_vrf = 0;

/* Vector for routing table.  */
static vector vrf_vector;

/* Holding VRF hooks  */
struct vrf_master
{
  int (*vrf_new_hook) (vrf_id_t, const char *, void **);
  int (*vrf_delete_hook) (vrf_id_t, const char *, void **);
  int (*vrf_enable_hook) (vrf_id_t, const char *, void **);
  int (*vrf_disable_hook) (vrf_id_t, const char *, void **);
} vrf_master = {0,};

/*
 * vrf_table_create
 */
void
vrf_table_create (struct vrf *vrf, afi_t afi, safi_t safi)
{
  rib_table_info_t *info;
  struct route_table *table;

  assert (!vrf->table[afi][safi]);

  table = route_table_init ();
  vrf->table[afi][safi] = table;

  info = XCALLOC (MTYPE_RIB_TABLE_INFO, sizeof (*info));
  info->vrf = vrf;
  info->afi = afi;
  info->safi = safi;
  table->info = info;
}

/* Allocate new VRF.  */
struct vrf *
vrf_alloc (const char *name)
{
  struct vrf *vrf;

  vrf = XCALLOC (MTYPE_VRF, sizeof (struct vrf));

  /* Put name.  */
  if (name)
    vrf->name = XSTRDUP (MTYPE_VRF_NAME, name);

  /* Allocate routing table and static table.  */
  vrf_table_create (vrf, AFI_IP, SAFI_UNICAST);
  vrf_table_create (vrf, AFI_IP6, SAFI_UNICAST);
  vrf->stable[AFI_IP][SAFI_UNICAST] = route_table_init ();
  vrf->stable[AFI_IP6][SAFI_UNICAST] = route_table_init ();
  vrf_table_create (vrf, AFI_IP, SAFI_MULTICAST);
  vrf_table_create (vrf, AFI_IP6, SAFI_MULTICAST);
  vrf->stable[AFI_IP][SAFI_MULTICAST] = route_table_init ();
  vrf->stable[AFI_IP6][SAFI_MULTICAST] = route_table_init ();


  return vrf;
}

/* Look up the data pointer of the specified VRF. */
void *
vrf_info_lookup (vrf_id_t vrf_id)
{
  struct vrf *vrf = vrf_lookup (vrf_id);
  return vrf ? vrf->info : NULL;
}

/* Look up the interface list in a VRF. */
struct list *
vrf_iflist (vrf_id_t vrf_id)
{
   struct vrf * vrf = vrf_lookup (vrf_id);
   return vrf ? vrf->iflist : NULL;
}

/* Create the interface list for the specified VRF, if needed. */
void
vrf_iflist_create (vrf_id_t vrf_id)
{
   struct vrf * vrf = vrf_lookup (vrf_id);
   if (vrf && !vrf->iflist)
     if_init_list (&vrf->iflist);
}

/* Free the interface list of the specified VRF. */
void
vrf_iflist_terminate (vrf_id_t vrf_id)
{
   struct vrf * vrf = vrf_lookup (vrf_id);
   if (vrf && vrf->iflist)
     if_terminate_list (&vrf->iflist);
}

/* Lookup VRF by identifier.  */
struct vrf *
vrf_lookup (u_int32_t id)
{
  return vector_lookup (vrf_vector, id);
}

/* Add a VRF hook. Please add hooks before calling vrf_init(). */
void
vrf_add_hook (int type, int (*func)(vrf_id_t, const char *, void **))
{
  if (debug_vrf)
    zlog_debug ("%s: Add Hook %d to function %p",  __PRETTY_FUNCTION__,
		type, func);

  switch (type) {
  case VRF_NEW_HOOK:
    vrf_master.vrf_new_hook = func;
    break;
  case VRF_DELETE_HOOK:
    vrf_master.vrf_delete_hook = func;
    break;
  case VRF_ENABLE_HOOK:
    vrf_master.vrf_enable_hook = func;
    break;
  case VRF_DISABLE_HOOK:
    vrf_master.vrf_disable_hook = func;
    break;
  default:
    break;
  }
}

/* Initialize VRF.  */
void
vrf_init (void)
{
  struct vrf *default_table;

  /* Allocate VRF vector.  */
  vrf_vector = vector_init (1);

  /* Allocate default main table.  */
  default_table = vrf_alloc ("Default-IP-Routing-Table");

  /* Default table index must be 0.  */
  vector_set_index (vrf_vector, 0, default_table);
}

