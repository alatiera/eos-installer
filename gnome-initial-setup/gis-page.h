/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2012 Red Hat
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
 *
 * Written by:
 *     Jasper St. Pierre <jstpierre@mecheye.net>
 */

#ifndef __GIS_PAGE_H__
#define __GIS_PAGE_H__

#include "gnome-initial-setup.h"

G_BEGIN_DECLS

#define GIS_TYPE_PAGE               (gis_page_get_type ())
#define GIS_PAGE(obj)                           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIS_TYPE_PAGE, GisPage))
#define GIS_PAGE_CLASS(klass)                   (G_TYPE_CHECK_CLASS_CAST ((klass),  GIS_TYPE_PAGE, GisPageClass))
#define GIS_IS_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIS_TYPE_PAGE))
#define GIS_IS_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GIS_TYPE_PAGE))
#define GIS_PAGE_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj),  GIS_TYPE_PAGE, GisPageClass))

typedef struct _GisPage        GisPage;
typedef struct _GisPageClass   GisPageClass;
typedef struct _GisAssistantPagePrivate GisAssistantPagePrivate;

typedef void (* GisPageApplyCallback) (GisPage *page,
                                       gboolean valid,
                                       gpointer user_data);

struct _GisPage
{
  GtkBin parent;

  GisDriver *driver;
  GtkBuilder *builder;

  GisAssistantPagePrivate *assistant_priv;
};

struct _GisPageClass
{
  GtkBinClass parent_class;
  gboolean hide_forward_button;
  gboolean hide_backward_button;
  gboolean hide_window_controls;
  const char *page_id;

  GtkBuilder * (*get_builder) (GisPage *page);
  void         (*locale_changed) (GisPage *page);
  GtkAccelGroup * (*get_accel_group) (GisPage *page);
  gboolean     (*apply) (GisPage *page,
                         GCancellable *cancellable);
  void         (*save_data) (GisPage *page);
  void         (*shown) (GisPage *page);
};

GType gis_page_get_type (void);

char *       gis_page_get_title (GisPage *page);
void         gis_page_set_title (GisPage *page, const char *title);
const char *       gis_page_get_forward_text (GisPage *page);
void         gis_page_set_forward_text (GisPage *page, const char *text);
gboolean     gis_page_get_complete (GisPage *page);
void         gis_page_set_complete (GisPage *page, gboolean complete);
GtkAccelGroup *  gis_page_get_accel_group (GisPage *page);
void         gis_page_locale_changed (GisPage *page);
void         gis_page_apply_begin (GisPage *page, GisPageApplyCallback callback, gpointer user_data);
void         gis_page_apply_cancel (GisPage *page);
void         gis_page_apply_complete (GisPage *page, gboolean valid);
gboolean     gis_page_get_applying (GisPage *page);
void         gis_page_save_data (GisPage *page);
void         gis_page_shown (GisPage *page);
gboolean     gis_page_get_hide_forward_button (GisPage *page);
gboolean     gis_page_get_hide_backward_button (GisPage *page);
gboolean     gis_page_get_hide_window_controls (GisPage *page);

G_END_DECLS

#endif /* __GIS_PAGE_H__ */
