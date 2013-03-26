/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* -*- encoding: utf8 -*- */
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

#include "config.h"

#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gis-assistant.h"
#include "gis-assistant-private.h"
#include "gis-center-container.h"

G_DEFINE_TYPE (GisAssistant, gis_assistant, GTK_TYPE_BOX)

#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GIS_TYPE_ASSISTANT, GisAssistantPrivate))

enum {
  PROP_0,
  PROP_TITLE,
  PROP_LAST,
};

static GParamSpec *obj_props[PROP_LAST];

enum {
  PREPARE,
  NEXT_PAGE,
  LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

struct _GisAssistantPrivate
{
  GtkWidget *frame;
  GtkWidget *forward;
  GtkWidget *back;
  GtkWidget *progress_indicator;
  GtkWidget *main_layout;
  GtkWidget *action_area;
  GtkWidget *page_action_widget_area;

  GList *pages;
  GisPage *current_page;
};

struct _GisAssistantPagePrivate
{
  GList *link;
};

static void
widget_destroyed (GtkWidget    *widget,
                  GisAssistant *assistant)
{
  GisPage *page = GIS_PAGE (widget);
  GisAssistantPrivate *priv = assistant->priv;

  priv->pages = g_list_delete_link (priv->pages, page->assistant_priv->link);
  if (page == priv->current_page)
    priv->current_page = NULL;

  g_slice_free (GisAssistantPagePrivate, page->assistant_priv);
  page->assistant_priv = NULL;
}

static void
gis_assistant_switch_to (GisAssistant          *assistant,
                         GisAssistantDirection  direction,
                         GisPage               *page)
{
  GIS_ASSISTANT_GET_CLASS (assistant)->switch_to (assistant, direction, page);
}

void
gis_assistant_next_page (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  g_signal_emit (assistant, signals[NEXT_PAGE], 0,
                 priv->current_page);
}

static inline gboolean
should_show_page (GList *l)
{
  return l != NULL && gtk_widget_get_visible (GTK_WIDGET (l->data));
}

static GisPage *
find_next_page (GisPage *page)
{
  GList *l = page->assistant_priv->link->next;
  while (!should_show_page (l)) {
    l = l->next;
  }
  return GIS_PAGE (l->data);
}

static void
gis_assistant_real_next_page (GisAssistant *assistant,
                              GisPage      *page)
{
  gis_assistant_switch_to (assistant, GIS_ASSISTANT_NEXT, find_next_page (page));
}

static GisPage *
find_prev_page (GisPage *page)
{
  GList *l = page->assistant_priv->link->prev;
  while (!should_show_page (l)) {
    l = l->prev;
  }
  return GIS_PAGE (l->data);
}

void
gis_assistant_previous_page (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  g_return_if_fail (priv->current_page != NULL);
  gis_assistant_switch_to (assistant, GIS_ASSISTANT_PREV, find_prev_page (priv->current_page));
}

static void
remove_from_page_action_area (GtkWidget *widget,
                              gpointer   user_data)
{
  GisAssistantPrivate *priv = user_data;
  gtk_container_remove (GTK_CONTAINER (priv->page_action_widget_area), widget);
}

static void
update_action_widget (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  GtkWidget *action;

  gtk_container_foreach (GTK_CONTAINER (priv->page_action_widget_area),
                         remove_from_page_action_area, priv);

  action = gis_page_get_action_widget (priv->current_page);
  if (action)
    gtk_container_add (GTK_CONTAINER (priv->page_action_widget_area),
                       action);
}

static void
remove_from_progress_indicator (GtkWidget *widget,
                                gpointer   user_data)
{
  GisAssistantPrivate *priv = user_data;
  gtk_container_remove (GTK_CONTAINER (priv->progress_indicator), widget);
}

static void
update_progress_indicator (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  GList *l;

  gtk_container_foreach (GTK_CONTAINER (priv->progress_indicator),
                         remove_from_progress_indicator, priv);

  for (l = priv->pages; l != NULL; l = l->next)
    {
      GisPage *page = GIS_PAGE (l->data);
      GtkWidget *label = gtk_label_new ("•");

      if (page != priv->current_page)
        {
          GtkStyleContext *context = gtk_widget_get_style_context (label);
          gtk_style_context_add_class (context, "dim-label");
        }

      gtk_container_add (GTK_CONTAINER (priv->progress_indicator), label);
      gtk_widget_show (label);
    }
}

static void
update_navigation_buttons (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  GisPage *page = priv->current_page;
  GisAssistantPagePrivate *page_priv;
  gboolean is_last_page;

  if (page == NULL)
    return;

  page_priv = page->assistant_priv;

  is_last_page = (page_priv->link->next == NULL);

  if (is_last_page)
    {
      gtk_widget_hide (priv->back);
      gtk_widget_hide (priv->forward);
    }
  else
    {
      gboolean can_go_forward, is_first_page;

      gtk_widget_show (priv->back);
      gtk_widget_show (priv->forward);

      is_first_page = (page_priv->link->prev == NULL);
      gtk_widget_set_sensitive (priv->back, !is_first_page);

      can_go_forward = gis_page_get_complete (page);
      gtk_widget_set_sensitive (priv->forward, can_go_forward);
    }
}

static void
gis_assistant_real_prepare (GisAssistant *assistant,
                            GisPage      *page)
{
  update_action_widget (assistant);
  update_navigation_buttons (assistant);
  update_progress_indicator (assistant);
}

static void
page_notify (GisPage      *page,
             GParamSpec   *pspec,
             GisAssistant *assistant)
{
  if (page != assistant->priv->current_page)
    return;

  if (strcmp (pspec->name, "title") == 0)
    g_object_notify_by_pspec (G_OBJECT (assistant), obj_props[PROP_TITLE]);
  else
    update_navigation_buttons (assistant);
}

void
gis_assistant_add_page (GisAssistant *assistant,
                        GisPage      *page)
{
  GisAssistantPrivate *priv = assistant->priv;
  GList *link;

  g_return_if_fail (page->assistant_priv == NULL);

  page->assistant_priv = g_slice_new0 (GisAssistantPagePrivate);
  priv->pages = g_list_append (priv->pages, page);
  link = page->assistant_priv->link = g_list_last (priv->pages);

  g_signal_connect (page, "destroy", G_CALLBACK (widget_destroyed), assistant);
  g_signal_connect (page, "notify", G_CALLBACK (page_notify), assistant);

  GIS_ASSISTANT_GET_CLASS (assistant)->add_page (assistant, page);

  if (priv->current_page->assistant_priv->link == link->prev)
    update_navigation_buttons (assistant);

  update_progress_indicator (assistant);
}

GisPage *
gis_assistant_get_current_page (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  return priv->current_page;
}

GList *
gis_assistant_get_all_pages (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  return priv->pages;
}

static void
go_forward (GtkWidget    *button,
            GisAssistant *assistant)
{
  gis_assistant_next_page (assistant);
}

static void
go_backward (GtkWidget    *button,
             GisAssistant *assistant)
{
  gis_assistant_previous_page (assistant);
}

gchar *
gis_assistant_get_title (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  if (priv->current_page != NULL)
    return gis_page_get_title (priv->current_page);
  else
    return "";
}

GtkWidget *
_gis_assistant_get_frame (GisAssistant *assistant)
{
  return assistant->priv->frame;
}

void
_gis_assistant_current_page_changed (GisAssistant *assistant,
                                     GisPage      *page)
{
  GisAssistantPrivate *priv = assistant->priv;

  if (priv->current_page != page) {
    priv->current_page = page;
    g_object_notify_by_pspec (G_OBJECT (assistant), obj_props[PROP_TITLE]);
    g_signal_emit (assistant, signals[PREPARE], 0, page);
  }
}

void
gis_assistant_locale_changed (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = assistant->priv;
  GList *l;

  gtk_button_set_label (GTK_BUTTON (priv->forward), _("_Next"));
  gtk_button_set_label (GTK_BUTTON (priv->back), _("_Back"));

  for (l = priv->pages; l != NULL; l = l->next)
    gis_page_locale_changed (l->data);
}

static void
gis_assistant_init (GisAssistant *assistant)
{
  GisAssistantPrivate *priv = GET_PRIVATE (assistant);
  GtkWidget *navigation;
  assistant->priv = priv;

  priv->main_layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
  gtk_box_pack_start (GTK_BOX (assistant), priv->main_layout, TRUE, TRUE, 0);

  priv->frame = gtk_frame_new ("");
  gtk_frame_set_shadow_type (GTK_FRAME (priv->frame), GTK_SHADOW_NONE);
  gtk_box_pack_start (GTK_BOX (priv->main_layout), priv->frame, TRUE, TRUE, 0);

  priv->page_action_widget_area = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  navigation = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_set_homogeneous (GTK_BOX (navigation), TRUE);
  gtk_widget_set_halign (navigation, GTK_ALIGN_END);
  gtk_widget_set_hexpand (navigation, TRUE);

  priv->forward = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (priv->forward),
                        gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON));
  gtk_button_set_use_underline (GTK_BUTTON (priv->forward), TRUE);
  gtk_widget_set_can_default (priv->forward, TRUE);

  priv->back = gtk_button_new ();
  gtk_button_set_image (GTK_BUTTON (priv->back),
                        gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON));
  gtk_button_set_use_underline (GTK_BUTTON (priv->back), TRUE);

  gtk_box_pack_start (GTK_BOX (navigation), priv->back, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (navigation), priv->forward, FALSE, FALSE, 0);

  g_signal_connect (priv->forward, "clicked", G_CALLBACK (go_forward), assistant);
  g_signal_connect (priv->back, "clicked", G_CALLBACK (go_backward), assistant);

  priv->progress_indicator = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign (priv->progress_indicator, GTK_ALIGN_CENTER);

  priv->action_area = gis_center_container_new (priv->page_action_widget_area,
                                                priv->progress_indicator,
                                                navigation);

  gis_assistant_locale_changed (assistant);
  gtk_box_pack_start (GTK_BOX (priv->main_layout), priv->action_area, FALSE, TRUE, 0);

  gtk_widget_show_all (GTK_WIDGET (assistant));
}

static void
gis_assistant_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  GisAssistant *assistant = GIS_ASSISTANT (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, gis_assistant_get_title (assistant));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gis_assistant_class_init (GisAssistantClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (GisAssistantPrivate));

  gobject_class->get_property = gis_assistant_get_property;

  klass->prepare = gis_assistant_real_prepare;
  klass->next_page = gis_assistant_real_next_page;

  obj_props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "", "",
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, PROP_LAST, obj_props);

  /**
   * GisAssistant::next-page:
   * @assistant: the #GisAssistant
   * @page: the page we're leaving
   *
   * The ::next-page signal is emitted when we're leaving
   * a page, allowing a page to do something when it's left.
   */
  signals[NEXT_PAGE] =
    g_signal_new ("next-page",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GisAssistantClass, next_page),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, GIS_TYPE_PAGE);

  /**
   * GisAssistant::prepare:
   * @assistant: the #GisAssistant
   * @page: the current page
   *
   * The ::prepare signal is emitted when a new page is set as the
   * assistant's current page, before making the new page visible.
   *
   * A handler for this signal can do any preparations which are
   * necessary before showing @page.
   */
  signals[PREPARE] =
    g_signal_new ("prepare",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GisAssistantClass, prepare),
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1, GIS_TYPE_PAGE);

}
