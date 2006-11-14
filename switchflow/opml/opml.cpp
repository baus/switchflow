//
// Copyright 2003-2006 Christopher Baus. http://baus.net/
// Read the LICENSE file for more information.

/**
 * @file opml.c generic OPML 1.0 support
 * 
 * Copyright (C) 2003, 2004 Lars Lindner <lars.lindner@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "opml.h"

/**
#include "support.h"
#include "conf.h"
#include "common.h"
#include "feed.h"
#include "item.h"
#include "opml.h"
#include "callbacks.h"
#include <string>
*/

/* you can find the OPML specification at Userland:

   http://www.opml.org/
   
 */
 
/* this is a generic subtag list for directory, channel and format description tags */
#define OPML_TITLE    0
#define OPML_CREATED    1
#define OPML_MODIFIED   2
#define OPML_OWNERNAME    3
#define OPML_OWNEREMAIL   4
#define OPML_MAX_TAG    5

/* note: the tag order has to correspond with the OCS_* defines in the header file */
static char *opml_tag_list[] = {  "title",
        "date_created",
        "date_modified",
        "owner_name",
        "owner_email",
        NULL
            };


/* ---------------------------------------------------------------------------- */
/* OPML parsing and HTML output             */
/* ---------------------------------------------------------------------------- */
            
/* retruns a HTML string containing the text and attributes of the outline */

static void get_outline_contents(xml_node_ptr cur,
                               i_opml_handler* p_opml_handler)
{
  //  char    *buffer = NULL;
  xml_char* value;
  char    *tmp, *tmp2;
  xml_attr_ptr  attr;

  attr = cur->properties;
  while(NULL != attr) {
    /* get prop value */
    value = xml_get_prop(cur, attr->name);
    if(NULL != value) {
      if(!xml_strcmp(attr->name, BAD_CAST"text")) {    

      } else if(!xml_strcmp(attr->name, BAD_CAST"is_comment")) {
        /* don't output anything */

      } else if(!xml_strcmp(attr->name, BAD_CAST"type")) {
        /* don't output anything */

      } else if(!xml_strcmp(attr->name, BAD_CAST"url")) {
        
      } else if(!xml_strcmp(attr->name, BAD_CAST"html_url") ||
                !xml_strcmp(attr->name, BAD_CAST"htmlurl")) {
                
      } else if(!xml_strcmp(attr->name, BAD_CAST"xml_url") ||
                !xml_strcmp(attr->name, BAD_CAST"xmlurl")) {
        p_opml_handler->feed_url((char*)value);
      } else {
      }

      free(value);
    }
    attr = attr->next;
  }
  /* check for <outline> subtags */

  if(NULL != cur->xml_children_node) {
    cur = cur->xml_children_node;
    while(NULL != cur) {
      if(!xml_strcmp(cur->name, BAD_CAST"outline")) {
        get_outline_contents(cur, p_opml_handler);
        //add_to_hTMLBuffer_fast(&buffer, tmp);
        //g_free(tmp);
        //g_free(tmp2);
      }
      cur = cur->next;
    }
  }
  return;
}

void opml_parse(xml_doc_ptr doc,
                xml_node_ptr cur,
                i_opml_handler* p_opml_handler)
{
  xml_node_ptr  child;
  xml_char* tmp;
  char    *buffer, *line;
  char    *head_tags[OPML_MAX_TAG];
  int     i, error = 0;

  do {

    if(!xml_strcmp(cur->name, BAD_CAST"opml") ||
       !xml_strcmp(cur->name, BAD_CAST"oml") ||
       !xml_strcmp(cur->name, BAD_CAST"outline_document")) {
        /* nothing */
    } else {
      printf("Could not find OPML header!\n");
      xml_free_doc(doc);
      error = 1;
      break;      
    }

    cur = cur->xml_children_node;
    while (cur && xml_is_blank_node(cur)) {
      cur = cur->next;
    }

    memset(head_tags, 0, sizeof(char *)*OPML_MAX_TAG);   
    while (cur != NULL) {
      if(!xml_strcmp(cur->name, BAD_CAST"head")) {
        /* check for <head> tags */
        child = cur->xml_children_node;
        while(child != NULL) {
          for(i = 0; i < OPML_MAX_TAG; i++) {
            if(!xml_strcmp(child->name, (const xml_char *)opml_tag_list[i])) {
              tmp = xml_node_list_get_string(doc, child->xml_children_node, 1);           
              if(NULL != tmp) {
                free(head_tags[i]);
                head_tags[i] = (char*)tmp;
              }
            }   
          }
          child = child->next;
        }
      }
      
      if(!xml_strcmp(cur->name, BAD_CAST"body")) {
        /* process all <outline> tags */
        child = cur->xml_children_node;
        while(child != NULL) {
          if(!xml_strcmp(child->name, BAD_CAST"outline")) {
            get_outline_contents(child, p_opml_handler);

            tmp = xml_get_prop(child, BAD_CAST"text");
            if(NULL == tmp)
              tmp = xml_get_prop(child, BAD_CAST"title");
            free(tmp);
          }
          child = child->next;
        }
      }
      
      cur = cur->next;
    }
#ifdef ZED
    /* after parsing we fill in the infos into the feed_ptr structure */   
    feed_add_items(fp, items);
    feed_set_update_interval(fp, -1);
    if(NULL == (fp->title = head_tags[OPML_TITLE]))
      fp->title = g_strdup(fp->source);
    
    if(0 == error) {
      /* prepare HTML output */
      buffer = NULL;
      add_to_hTMLBuffer(&buffer, HEAD_START); 
      
      line = g_strdup_printf(HEAD_LINE, _("Feed:"), fp->title);
      add_to_hTMLBuffer(&buffer, line);
      g_free(line);

      if(NULL != fp->source) {
        tmp = g_strdup_printf("<a href=\"%s\">%s</a>", fp->source, fp->source);
        line = g_strdup_printf(HEAD_LINE, _("Source:"), tmp);
        g_free(tmp);
        add_to_hTMLBuffer(&buffer, line);
        g_free(line);
      }

      add_to_hTMLBuffer(&buffer, HEAD_END); 

      add_to_hTMLBuffer(&buffer, FEED_FOOT_TABLE_START);
      FEED_FOOT_WRITE(buffer, "title",    head_tags[OPML_TITLE]);
      FEED_FOOT_WRITE(buffer, "creation date",  head_tags[OPML_CREATED]);
      FEED_FOOT_WRITE(buffer, "last modified",  head_tags[OPML_MODIFIED]);
      FEED_FOOT_WRITE(buffer, "owner name",   head_tags[OPML_OWNERNAME]);
      FEED_FOOT_WRITE(buffer, "owner email",    head_tags[OPML_OWNEREMAIL]);
      add_to_hTMLBuffer(&buffer, FEED_FOOT_TABLE_END);
      
      feed_set_description(fp, buffer);
      g_free(buffer);
      
      feed_set_available(fp, TRUE);
    } else {
      ui_mainwindow_set_status_bar(_("There were errors while parsing this feed!"));
    }
#endif    
    break;
  } while (0);

}

#ifdef ZED
static gboolean opml_format_check(xml_doc_ptr doc, xml_node_ptr cur) {
  if(!xml_strcmp(cur->name, BAD_CAST"opml") ||
     !xml_strcmp(cur->name, BAD_CAST"oml") || 
     !xml_strcmp(cur->name, BAD_CAST"outline_document")) {
    
    return TRUE;
  }
  return FALSE;
}
/* ---------------------------------------------------------------------------- */
/* initialization               */
/* ---------------------------------------------------------------------------- */

feed_handler_ptr opml_init_feed_handler(void) {
  feed_handler_ptr  fhp;
  
  fhp = g_new0(struct feed_handler, 1);
  
  /* prepare feed handler structure */
  fhp->type_str = "opml";
  fhp->icon = ICON_OCS;
  fhp->directory = FALSE;
  fhp->feed_parser = opml_parse;
  fhp->check_format = opml_format_check;
  fhp->merge    = FALSE;
  
  return fhp;
}

#endif

