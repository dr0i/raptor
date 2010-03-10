/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_option.c - Class options
 *
 * Copyright (C) 2004-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#endif


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


static const struct
{
  raptor_option option;
  raptor_option_area area;
  raptor_option_value_type value_type;
  const char *name;
  const char *label;
} raptor_options_list[RAPTOR_OPTION_LAST + 1] = {
  { RAPTOR_OPTION_SCANNING,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "scanForRDF",
    "RDF/XML parser scans for rdf:RDF in XML content"
  },
  { RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowNonNsAttributes",
    "RDF/XML parser allows bare 'name' rather than namespaced 'rdf:name'"
  },
  { RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowOtherParsetypes",
    "RDF/XML parser allows user-defined rdf:parseType values"
  },
  { RAPTOR_OPTION_ALLOW_BAGID,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowBagID",
    "RDF/XML parser allows rdf:bagID"
  },
  { RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "allowRDFtypeRDFlist",
    "RDF/XML parser generates the collection rdf:type rdf:List triple"
  },
  { RAPTOR_OPTION_NORMALIZE_LANGUAGE,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "normalizeLanguage",
    "RDF/XML parser normalizes xml:lang values to lowercase"
  },
  { RAPTOR_OPTION_NON_NFC_FATAL,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "nonNFCfatal",
    "RDF/XML parser makes non-NFC literals a fatal error"
  },
  { RAPTOR_OPTION_WARN_OTHER_PARSETYPES,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "warnOtherParseTypes",
    "RDF/XML parser warns about unknown rdf:parseType values"
  },
  { RAPTOR_OPTION_CHECK_RDF_ID,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "checkRdfID",
    "RDF/XML parser checks rdf:ID values for duplicates"
  },
  { RAPTOR_OPTION_RELATIVE_URIS,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "relativeURIs",
    "Serializers write relative URIs wherever possible."
  },
  { RAPTOR_OPTION_WRITER_AUTO_INDENT,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "autoIndent",
    "Turtle and XML Writer automatically indent elements."
  },
  { RAPTOR_OPTION_WRITER_AUTO_EMPTY,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "autoEmpty",
    "Turtle and XML Writer automatically detect and abbreviate empty elements."
  },
  { RAPTOR_OPTION_WRITER_INDENT_WIDTH,
    (raptor_option_area)(RAPTOR_OPTION_AREA_XML_WRITER | RAPTOR_OPTION_AREA_TURTLE_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "indentWidth",
    "Turtle and XML Writer use as number of spaces to indent."
  },
  { RAPTOR_OPTION_WRITER_XML_VERSION,
    (raptor_option_area)(RAPTOR_OPTION_AREA_SERIALIZER | RAPTOR_OPTION_AREA_XML_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "xmlVersion",
    "Serializers and XML Writer use as XML version to write."
  },
  { RAPTOR_OPTION_WRITER_XML_DECLARATION,
    (raptor_option_area)(RAPTOR_OPTION_AREA_SERIALIZER | RAPTOR_OPTION_AREA_XML_WRITER),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "xmlDeclaration",
    "Serializers and XML Writer write XML declaration."
  },
  { RAPTOR_OPTION_NO_NET,
    (raptor_option_area)(RAPTOR_OPTION_AREA_PARSER | RAPTOR_OPTION_AREA_SAX2),
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "noNet",
    "Parsers and SAX2 XML Parser deny network requests."
  },
  { RAPTOR_OPTION_RESOURCE_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "resourceBorder",
    "DOT serializer resource border color"
  },
  { RAPTOR_OPTION_LITERAL_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "literalBorder",
    "DOT serializer literal border color"
  },
  { RAPTOR_OPTION_BNODE_BORDER,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "bnodeBorder",
    "DOT serializer blank node border color"
  },
  { RAPTOR_OPTION_RESOURCE_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "resourceFill",
    "DOT serializer resource fill color"
  },
  { RAPTOR_OPTION_LITERAL_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "literalFill",
    "DOT serializer literal fill color"
  },
  { RAPTOR_OPTION_BNODE_FILL,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "bnodeFill",
    "DOT serializer blank node fill color"
  },
  { RAPTOR_OPTION_HTML_TAG_SOUP,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "htmlTagSoup",
    "GRDDL parser uses a lax HTML parser"
  },
  { RAPTOR_OPTION_MICROFORMATS,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "microformats",
    "GRDDL parser looks for microformats"
  },
  { RAPTOR_OPTION_HTML_LINK,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "htmlLink",
    "GRDDL parser looks for <link type=\"application/rdf+xml\">"
  },
  { RAPTOR_OPTION_WWW_TIMEOUT,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_INT,
    "wwwTimeout",
    "Parser WWW request retrieval timeout"
  },
  { RAPTOR_OPTION_WRITE_BASE_URI,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "writeBaseURI",
    "Serializers write a base URI directive @base / xml:base"
  },
  { RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwHttpCacheControl",
    "Parser WWW request HTTP Cache-Control: header value"
  },
  { RAPTOR_OPTION_WWW_HTTP_USER_AGENT,
    RAPTOR_OPTION_AREA_PARSER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "wwwHttpUserAgent",
    "Parser WWW request HTTP User-Agent: header value"
  },
  { RAPTOR_OPTION_JSON_CALLBACK,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "jsonCallback",
    "JSON serializer callback function name"
  },
  { RAPTOR_OPTION_JSON_EXTRA_DATA,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "jsonExtraData",
    "JSON serializer callback data parameter"
  },
  { RAPTOR_OPTION_RSS_TRIPLES,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_STRING,
    "rssTriples",
    "Atom and RSS serializers write extra RDF triples"
  },
  { RAPTOR_OPTION_ATOM_ENTRY_URI,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_URI,
    "atomEntryUri",
    "Atom serializer writes an atom:entry with this URI (otherwise atom:feed)"
  },
  { RAPTOR_OPTION_PREFIX_ELEMENTS,
    RAPTOR_OPTION_AREA_SERIALIZER,
    RAPTOR_OPTION_VALUE_TYPE_BOOL,
    "prefixElements",
    "Atom and RSS serializers write namespace-prefixed elements"
  }
};


static const char * const raptor_option_uri_prefix = "http://feature.librdf.org/raptor-";
/* NOTE: this is strlen(raptor_option_uri_prefix) */
static const int raptor_option_uri_prefix_len = 33;


/*
 * raptor_world_options_enumerate_common:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 * @area: area to match.
 * 
 * Internal: Get list of syntax options.
 *
 * If @uri is not NULL, a pointer toa new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
static int
raptor_world_options_enumerate_common(raptor_world* world,
                                      const raptor_option option,
                                      const char **name, 
                                      raptor_uri **uri, const char **label,
                                      raptor_option_area area)
{
  int i;

  for(i = 0; i <= RAPTOR_OPTION_LAST; i++)
    if(raptor_options_list[i].option == option &&
       (raptor_options_list[i].area & area)) {
      if(name)
        *name = raptor_options_list[i].name;
      
      if(uri) {
        raptor_uri *base_uri;
        base_uri = raptor_new_uri_from_counted_string(world,
                                                      (const unsigned char*)raptor_option_uri_prefix,
                                                      raptor_option_uri_prefix_len);
        if(!base_uri)
          return -1;
        
        *uri = raptor_new_uri_from_uri_local_name(world,
                                                  base_uri,
                                                  (const unsigned char*)raptor_options_list[i].name);
        raptor_free_uri(base_uri);
      }
      if(label)
        *label = raptor_options_list[i].label;
      return 0;
    }

  return 1;
}



/**
 * raptor_world_enumerate_parser_options:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 *
 * Get list of syntax options.
 * 
 * If uri is not NULL, a pointer to a new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
int
raptor_world_enumerate_parser_options(raptor_world* world,
                                      const raptor_option option,
                                      const char **name, 
                                      raptor_uri **uri, const char **label)
{
  return raptor_world_options_enumerate_common(world, option, name, uri, label,
                                               RAPTOR_OPTION_AREA_PARSER);
}


/**
 * raptor_world_enumerate_serializer_options:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 *
 * Get list of serializer options.
 * 
 * If uri is not NULL, a pointer toa new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
int
raptor_world_enumerate_serializer_options(raptor_world* world,
                                          const raptor_option option,
                                          const char **name, 
                                          raptor_uri **uri, const char **label)
{
  return raptor_world_options_enumerate_common(world, option, name, uri, label,
                                               RAPTOR_OPTION_AREA_SERIALIZER);
}


/**
 * raptor_world_enumerate_sax2_options:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 *
 * Get list of SAX2 options.
 * 
 * If uri is not NULL, a pointer to a new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
int
raptor_world_enumerate_sax2_options(raptor_world* world,
                                    const raptor_option option,
                                    const char **name, 
                                    raptor_uri **uri, const char **label)
{
  return raptor_world_options_enumerate_common(world, option, name, uri, label,
                                               RAPTOR_OPTION_AREA_SAX2);
}


/**
 * raptor_world_enumerate_turtle_writer_options:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 *
 * Get list of turtle_writer options.
 * 
 * If uri is not NULL, a pointer to a new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
int
raptor_world_enumerate_turtle_writer_options(raptor_world* world,
                                             const raptor_option option,
                                             const char **name, 
                                             raptor_uri **uri,
                                             const char **label)
{
  return raptor_world_options_enumerate_common(world, option, name, uri, label,
                                               RAPTOR_OPTION_AREA_TURTLE_WRITER);
}


/**
 * raptor_world_enumerate_xml_writer_options:
 * @world: raptor_world object
 * @option: option enumeration (0+)
 * @name: pointer to store option short name (or NULL)
 * @uri: pointer to store option URI (or NULL)
 * @label: pointer to option label (or NULL)
 *
 * Get list of xml_writer options.
 * 
 * If uri is not NULL, a pointer to a new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if option is unknown
 **/
int
raptor_world_enumerate_xml_writer_options(raptor_world* world,
                                          const raptor_option option,
                                          const char **name, 
                                          raptor_uri **uri, const char **label)
{
  return raptor_world_options_enumerate_common(world, option, name, uri, label,
                                               RAPTOR_OPTION_AREA_XML_WRITER);
}


/**
 * raptor_option_get_value_type
 * @option: raptor serializer or parser option
 *
 * Get the type of a options.
 *
 * Most options are boolean or integer values and use
 * raptor_parser_set_option() and raptor_parser_get_option()
 * (raptor_serializer_set_option() and raptor_serializer_get_option() )
 *
 * String and URI value options use raptor_parser_set_option_string() and
 * raptor_parser_get_option_string()
 * ( raptor_serializer_set_option_string()
 * and raptor_serializer_get_option_string() )
 *
 * Return value: the type of the option or < 0 if @option is unknown
 */
raptor_option_value_type
raptor_option_get_value_type(const raptor_option option)
{
  if(option > RAPTOR_OPTION_LAST)
    return (raptor_option_value_type)-1;
  return raptor_options_list[option].value_type;
}


int
raptor_option_is_valid_for_area(const raptor_option option,
                                raptor_option_area area)
{
  if(option > RAPTOR_OPTION_LAST)
    return 0;
  return (raptor_options_list[option].area & area) != 0;
}


int
raptor_option_value_is_numeric(const raptor_option option)
{
  raptor_option_value_type t = raptor_options_list[option].value_type;
  
  return t == RAPTOR_OPTION_VALUE_TYPE_BOOL ||
         t == RAPTOR_OPTION_VALUE_TYPE_INT;
}


/**
 * raptor_world_get_option_from_uri:
 * @world: raptor_world instance
 * @uri: option URI
 *
 * Get a option ID from a URI
 * 
 * The allowed option URIs are available via raptor_options_enumerate().
 *
 * Return value: < 0 if the option is unknown
 **/
raptor_option
raptor_world_get_option_from_uri(raptor_world* world, raptor_uri *uri)
{
  unsigned char *uri_string;
  int i;
  raptor_option option = (raptor_option)-1;
  
  if(!uri)
    return option;
  
  uri_string = raptor_uri_as_string(uri);
  if(strncmp((const char*)uri_string, raptor_option_uri_prefix,
             raptor_option_uri_prefix_len))
    return option;

  uri_string += raptor_option_uri_prefix_len;

  for(i = 0; i <= RAPTOR_OPTION_LAST; i++)
    if(!strcmp(raptor_options_list[i].name, (const char*)uri_string)) {
      option = (raptor_option)i;
      break;
    }

  return option;
}


/**
 * raptor_parser_get_option_count:
 *
 * Get the count of options defined.
 *
 * This is prefered to the compile time-only symbol #RAPTOR_OPTION_LAST
 * and returns a count of the number of options which is
 * #RAPTOR_OPTION_LAST + 1.
 *
 * Return value: count of options in the #raptor_option enumeration
 **/
unsigned int
raptor_parser_get_option_count(void)
{
  return RAPTOR_OPTION_LAST + 1;
}


const char* const
raptor_option_value_type_labels[RAPTOR_OPTION_VALUE_TYPE_URI + 1] = {
  "boolean",
  "integer",
  "string",
  "uri"
};


/**
 * raptor_option_get_value_type_label:
 * @type: value type
 *
 * Get a label for a value type
 *
 * Return value: label for type or NULL for invalid type
 */
const char*
raptor_option_get_value_type_label(const raptor_option_value_type type)
{
  if(type > RAPTOR_OPTION_VALUE_TYPE_LAST)
    return NULL;
  return raptor_option_value_type_labels[type];
}


void
raptor_object_options_copy_state(raptor_object_options* to,
                                 raptor_object_options* from)
{
  int i;
  
  for(i = 0; i <= RAPTOR_OPTION_LAST; i++) {
    to->options[i] = from->options[i];
  }
}


void
raptor_object_options_init(raptor_object_options* options,
                           raptor_option_area area)
{
  options->area = area;
}

