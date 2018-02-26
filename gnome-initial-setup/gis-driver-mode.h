/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright © 2012 Red Hat
 * Copyright © 2018 Endless Mobile, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __GIS_DRIVER_MODE_H__
#define __GIS_DRIVER_MODE_H__

#include <glib-object.h>

/**
 * GisDriverMode:
 * @GIS_DRIVER_MODE_NEW_USER: running within first-boot experience
 * @GIS_DRIVER_MODE_EXISTING_USER: running within a user session
 */
typedef enum {
  GIS_DRIVER_MODE_NEW_USER,
  GIS_DRIVER_MODE_EXISTING_USER,
} GisDriverMode;

#define GIS_TYPE_DRIVER_MODE (gis_driver_mode_get_type ())

GType gis_driver_mode_get_type (void);

#endif /* __GIS_DRIVER_MODE_H__ */
