/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@zend.com>                             |
  +----------------------------------------------------------------------+
*/
/* $Id$ */

#include "php_soap.h"
#include "libxml/uri.h"

static int schema_simpleType(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpleType, sdlTypePtr cur_type);
static int schema_complexType(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr compType, sdlTypePtr cur_type);
static int schema_list(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr listType, sdlTypePtr cur_type);
static int schema_union(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr unionType, sdlTypePtr cur_type);
static int schema_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpCompType, sdlTypePtr cur_type);
static int schema_restriction_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type, int simpleType);
static int schema_restriction_complexContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type);
static int schema_extension_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type);
static int schema_extension_complexContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type);
static int schema_sequence(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr seqType, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_all(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_choice(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr choiceType, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_group(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr groupType, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_any(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_element(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr element, sdlTypePtr cur_type, sdlContentModelPtr model);
static int schema_attribute(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType, sdlTypePtr cur_type, sdlCtx *ctx);
static int schema_attributeGroup(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType, sdlTypePtr cur_type, sdlCtx *ctx);

static int schema_restriction_var_int(xmlNodePtr val, sdlRestrictionIntPtr *valptr);

static int schema_restriction_var_char(xmlNodePtr val, sdlRestrictionCharPtr *valptr);

static void schema_type_fixup(sdlCtx *ctx, sdlTypePtr type);

static encodePtr create_encoder(sdlPtr sdl, sdlTypePtr cur_type, const char *ns, const char *type)
{
	smart_str nscat = {0};
	encodePtr enc, *enc_ptr;

	if (sdl->encoders == NULL) {
		sdl->encoders = emalloc(sizeof(HashTable));
		zend_hash_init(sdl->encoders, 0, NULL, delete_encoder, 0);
	}
	smart_str_appends(&nscat, ns);
	smart_str_appendc(&nscat, ':');
	smart_str_appends(&nscat, type);
	smart_str_0(&nscat);
	if (zend_hash_find(sdl->encoders, nscat.c, nscat.len + 1, (void**)&enc_ptr) == SUCCESS) {
		enc = *enc_ptr;
		if (enc->details.ns) {
			efree(enc->details.ns);
		}
		if (enc->details.type_str) {
			efree(enc->details.type_str);
		}
	} else {
		enc_ptr = NULL;
		enc = emalloc(sizeof(encode));
	}
	memset(enc, 0, sizeof(encode));

	enc->details.ns = estrdup(ns);
	enc->details.type_str = estrdup(type);
	enc->details.sdl_type = cur_type;
	enc->to_xml = sdl_guess_convert_xml;
	enc->to_zval = sdl_guess_convert_zval;

	if (enc_ptr == NULL) {
		zend_hash_update(sdl->encoders, nscat.c, nscat.len + 1, &enc, sizeof(encodePtr), NULL);
	}
	smart_str_free(&nscat);
	return enc;
}

static encodePtr get_create_encoder(sdlPtr sdl, sdlTypePtr cur_type, const char *ns, const char *type)
{
	encodePtr enc = get_encoder(sdl, ns, type);
	if (enc == NULL) {
		enc = create_encoder(sdl, cur_type, ns, type);
	}
	return enc;
}

static void schema_load_file(sdlCtx *ctx, xmlAttrPtr ns, xmlChar *location, xmlAttrPtr tns, int import TSRMLS_DC) {
	if (location != NULL &&
	    !zend_hash_exists(&ctx->docs, location, strlen(location)+1)) {
		xmlDocPtr doc;
		xmlNodePtr schema;
		xmlAttrPtr new_tns;

		doc = soap_xmlParseFile(location TSRMLS_CC);
		if (doc == NULL) {
			soap_error1(E_ERROR, "Parsing Schema: can't import schema from '%s'", location);
		}
		schema = get_node(doc->children, "schema");
		if (schema == NULL) {
			xmlFreeDoc(doc);
			soap_error1(E_ERROR, "Parsing Schema: can't import schema from '%s'", location);
		}
		new_tns = get_attribute(schema->properties, "targetNamespace");
		if (import) {
			if (ns != NULL && (new_tns == NULL || strcmp(ns->children->content,new_tns->children->content) != 0)) {
				xmlFreeDoc(doc);
				soap_error2(E_ERROR, "Parsing Schema: can't import schema from '%s', unexpected 'targetNamespace'='%s'", location, new_tns->children->content);
			}
			if (ns == NULL && new_tns != NULL) {
				xmlFreeDoc(doc);
				soap_error2(E_ERROR, "Parsing Schema: can't import schema from '%s', unexpected 'targetNamespace'='%s'", location, new_tns->children->content);
			}
		} else {
			new_tns = get_attribute(schema->properties, "targetNamespace");
			if (new_tns == NULL) {
				if (tns != NULL) {
					xmlSetProp(schema, "targetNamespace", tns->children->content);
				}
			} else if (tns != NULL && strcmp(tns->children->content,new_tns->children->content) != 0) {
				xmlFreeDoc(doc);
				soap_error1(E_ERROR, "Parsing Schema: can't include schema from '%s', different 'targetNamespace'", location);
			}
		}
		zend_hash_add(&ctx->docs, location, strlen(location)+1, (void**)&doc, sizeof(xmlDocPtr), NULL);
		load_schema(ctx, schema TSRMLS_CC);
	}
}

/*
2.6.1 xsi:type
2.6.2 xsi:nil
2.6.3 xsi:schemaLocation, xsi:noNamespaceSchemaLocation
*/

/*
<schema
  attributeFormDefault = (qualified | unqualified) : unqualified
  blockDefault = (#all | List of (extension | restriction | substitution))  : ''
  elementFormDefault = (qualified | unqualified) : unqualified
  finalDefault = (#all | List of (extension | restriction))  : ''
  id = ID
  targetNamespace = anyURI
  version = token
  xml:lang = language
  {any attributes with non-schema namespace . . .}>
  Content: ((include | import | redefine | annotation)*, (((simpleType | complexType | group | attributeGroup) | element | attribute | notation), annotation*)*)
</schema>
*/
int load_schema(sdlCtx *ctx, xmlNodePtr schema TSRMLS_DC)
{
	xmlNodePtr trav;
	xmlAttrPtr tns;

	if (!ctx->sdl->types) {
		ctx->sdl->types = emalloc(sizeof(HashTable));
		zend_hash_init(ctx->sdl->types, 0, NULL, delete_type, 0);
	}
	if (!ctx->attributes) {
		ctx->attributes = emalloc(sizeof(HashTable));
		zend_hash_init(ctx->attributes, 0, NULL, delete_attribute, 0);
	}
	if (!ctx->attributeGroups) {
		ctx->attributeGroups = emalloc(sizeof(HashTable));
		zend_hash_init(ctx->attributeGroups, 0, NULL, delete_type, 0);
	}

	tns = get_attribute(schema->properties, "targetNamespace");
	if (tns == NULL) {
		tns = xmlSetProp(schema, "targetNamespace", "");
		xmlNewNs(schema, "", NULL);
	}

	trav = schema->children;
	while (trav != NULL) {
		if (node_is_equal(trav,"include")) {
			xmlAttrPtr location;

			location = get_attribute(trav->properties, "schemaLocation");
			if (location == NULL) {
				soap_error0(E_ERROR, "Parsing Schema: include has no 'schemaLocation' attribute");
			} else {
				xmlChar *uri;
				xmlChar *base = xmlNodeGetBase(trav->doc, trav);

				if (base == NULL) {
			    uri = xmlBuildURI(location->children->content, trav->doc->URL);
				} else {
	    		uri = xmlBuildURI(location->children->content, base);
			    xmlFree(base);
				}
				schema_load_file(ctx, NULL, uri, tns, 0 TSRMLS_CC);
				xmlFree(uri);
			}

		} else if (node_is_equal(trav,"redefine")) {
			xmlAttrPtr location;

			location = get_attribute(trav->properties, "schemaLocation");
			if (location == NULL) {
				soap_error0(E_ERROR, "Parsing Schema: redefine has no 'schemaLocation' attribute");
			} else {
			  xmlChar *uri;
				xmlChar *base = xmlNodeGetBase(trav->doc, trav);

				if (base == NULL) {
			    uri = xmlBuildURI(location->children->content, trav->doc->URL);
				} else {
	    		uri = xmlBuildURI(location->children->content, base);
			    xmlFree(base);
				}
				schema_load_file(ctx, NULL, uri, tns, 0 TSRMLS_CC);
				xmlFree(uri);
				/* TODO: <redefine> support */
			}

		} else if (node_is_equal(trav,"import")) {
			xmlAttrPtr ns, location;
			xmlChar *uri = NULL;

			ns = get_attribute(trav->properties, "namespace");
			location = get_attribute(trav->properties, "schemaLocation");

			if (ns != NULL && tns != NULL && strcmp(ns->children->content,tns->children->content) == 0) {
				if (location) {
					soap_error1(E_ERROR, "Parsing Schema: can't import schema from '%s', namespace must not match the enclosing schema 'targetNamespace'", location->children->content);
				} else {
					soap_error0(E_ERROR, "Parsing Schema: can't import schema. Namespace must not match the enclosing schema 'targetNamespace'");
				}
			}
			if (location) {
				xmlChar *base = xmlNodeGetBase(trav->doc, trav);

				if (base == NULL) {
			    uri = xmlBuildURI(location->children->content, trav->doc->URL);
				} else {
	    		uri = xmlBuildURI(location->children->content, base);
			    xmlFree(base);
				}
			}
			schema_load_file(ctx, ns, uri, tns, 1 TSRMLS_CC);
			if (uri != NULL) {xmlFree(uri);}
		} else if (node_is_equal(trav,"annotation")) {
			/* TODO: <annotation> support */
/* annotation cleanup
			xmlNodePtr tmp = trav;
			trav = trav->next;
			xmlUnlinkNode(tmp);
			xmlFreeNode(tmp);
			continue;
*/
		} else {
			break;
		}
		trav = trav->next;
	}

	while (trav != NULL) {
		if (node_is_equal(trav,"simpleType")) {
			schema_simpleType(ctx->sdl, tns, trav, NULL);
		} else if (node_is_equal(trav,"complexType")) {
			schema_complexType(ctx->sdl, tns, trav, NULL);
		} else if (node_is_equal(trav,"group")) {
			schema_group(ctx->sdl, tns, trav, NULL, NULL);
		} else if (node_is_equal(trav,"attributeGroup")) {
			schema_attributeGroup(ctx->sdl, tns, trav, NULL, ctx);
		} else if (node_is_equal(trav,"element")) {
			schema_element(ctx->sdl, tns, trav, NULL, NULL);
		} else if (node_is_equal(trav,"attribute")) {
			schema_attribute(ctx->sdl, tns, trav, NULL, ctx);
		} else if (node_is_equal(trav,"notation")) {
			/* TODO: <notation> support */
		} else if (node_is_equal(trav,"annotation")) {
			/* TODO: <annotation> support */
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in schema", trav->name);
		}
		trav = trav->next;
	}
	return TRUE;
}

/*
<simpleType
  final = (#all | (list | union | restriction))
  id = ID
  name = NCName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | list | union))
</simpleType>
*/
static int schema_simpleType(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpleType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr name, ns;

	ns = get_attribute(simpleType->properties, "targetNamespace");
	if (ns == NULL) {
		ns = tns;
	}

	name = get_attribute(simpleType->properties, "name");
	if (cur_type != NULL) {
		/* Anonymous type inside <element> or <restriction> */
		sdlTypePtr newType, *ptr;

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));
		newType->kind = XSD_TYPEKIND_SIMPLE;
		if (name != NULL) {
			newType->name = estrdup(name->children->content);
			newType->namens = estrdup(ns->children->content);
		} else {
			newType->name = estrdup(cur_type->name);
			newType->namens = estrdup(cur_type->namens);
		}

		zend_hash_next_index_insert(sdl->types,  &newType, sizeof(sdlTypePtr), (void **)&ptr);

		if (sdl->encoders == NULL) {
			sdl->encoders = emalloc(sizeof(HashTable));
			zend_hash_init(sdl->encoders, 0, NULL, delete_encoder, 0);
		}
		cur_type->encode = emalloc(sizeof(encode));
		memset(cur_type->encode, 0, sizeof(encode));
		cur_type->encode->details.ns = estrdup(newType->namens);
		cur_type->encode->details.type_str = estrdup(newType->name);
		cur_type->encode->details.sdl_type = *ptr;
		cur_type->encode->to_xml = sdl_guess_convert_xml;
		cur_type->encode->to_zval = sdl_guess_convert_zval;
		zend_hash_next_index_insert(sdl->encoders,  &cur_type->encode, sizeof(encodePtr), NULL);

		cur_type =*ptr;

	} else if (name != NULL) {
		sdlTypePtr newType, *ptr;

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));
		newType->kind = XSD_TYPEKIND_SIMPLE;
		newType->name = estrdup(name->children->content);
		newType->namens = estrdup(ns->children->content);

		if (cur_type == NULL) {
			zend_hash_next_index_insert(sdl->types,  &newType, sizeof(sdlTypePtr), (void **)&ptr);
		} else {
			if (cur_type->elements == NULL) {
				cur_type->elements = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
			}
			zend_hash_update(cur_type->elements, newType->name, strlen(newType->name)+1, &newType, sizeof(sdlTypePtr), (void **)&ptr);
		}
		cur_type = (*ptr);

		create_encoder(sdl, cur_type, ns->children->content, name->children->content);
	} else {
		soap_error0(E_ERROR, "Parsing Schema: simpleType has no 'name' attribute");
	}

	trav = simpleType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"restriction")) {
			schema_restriction_simpleContent(sdl, tns, trav, cur_type, 1);
			trav = trav->next;
		} else if (node_is_equal(trav,"list")) {
			cur_type->kind = XSD_TYPEKIND_LIST;
			schema_list(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else if (node_is_equal(trav,"union")) {
			cur_type->kind = XSD_TYPEKIND_UNION;
			schema_union(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in simpleType", trav->name);
		}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: expected <restriction>, <list> or <union> in simpleType");
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in simpleType", trav->name);
	}

	return TRUE;
}

/*
<list
  id = ID
  itemType = QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?))
</list>
*/
static int schema_list(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr listType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr itemType;

	itemType = get_attribute(listType->properties, "itemType");
	if (itemType != NULL) {
		char *type, *ns;
		xmlNsPtr nsptr;

		parse_namespace(itemType->children->content, &type, &ns);
		nsptr = xmlSearchNs(listType->doc, listType, ns);
		if (nsptr != NULL) {
			sdlTypePtr newType, *tmp;

			newType = emalloc(sizeof(sdlType));
			memset(newType, 0, sizeof(sdlType));

			newType->name = estrdup(type);
			newType->namens = estrdup(nsptr->href);

			newType->encode = get_create_encoder(sdl, newType, (char *)nsptr->href, type);

			if (cur_type->elements == NULL) {
				cur_type->elements = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
			}
			zend_hash_next_index_insert(cur_type->elements, &newType, sizeof(sdlTypePtr), (void **)&tmp);
		}
		if (type) {efree(type);}
		if (ns) {efree(ns);}
	}

	trav = listType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL && node_is_equal(trav,"simpleType")) {
		sdlTypePtr newType, *tmp;

		if (itemType != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'itemType' attribute and subtype");
		}

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));

		newType->name = estrdup("anonymous");
		newType->namens = estrdup(tns->children->content);

		if (cur_type->elements == NULL) {
			cur_type->elements = emalloc(sizeof(HashTable));
			zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
		}
		zend_hash_next_index_insert(cur_type->elements, &newType, sizeof(sdlTypePtr), (void **)&tmp);

		schema_simpleType(sdl, tns, trav, newType);
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in list", trav->name);
	}
	return TRUE;
}

/*
<union
  id = ID
  memberTypes = List of QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType*))
</union>
*/
static int schema_union(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr unionType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr memberTypes;

	memberTypes = get_attribute(unionType->properties, "memberTypes");
	if (memberTypes != NULL) {
		char *str, *start, *end, *next;
		char *type, *ns;
		xmlNsPtr nsptr;

		str = estrdup(memberTypes->children->content);
		whiteSpace_collapse(str);
		start = str;
		while (start != NULL && *start != '\0') {
			end = strchr(start,' ');
			if (end == NULL) {
				next = NULL;
			} else {
				*end = '\0';
				next = end+1;
			}

			parse_namespace(start, &type, &ns);
			nsptr = xmlSearchNs(unionType->doc, unionType, ns);
			if (nsptr != NULL) {
				sdlTypePtr newType, *tmp;

				newType = emalloc(sizeof(sdlType));
				memset(newType, 0, sizeof(sdlType));

				newType->name = estrdup(type);
				newType->namens = estrdup(nsptr->href);

				newType->encode = get_create_encoder(sdl, newType, (char *)nsptr->href, type);

				if (cur_type->elements == NULL) {
					cur_type->elements = emalloc(sizeof(HashTable));
					zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
				}
				zend_hash_next_index_insert(cur_type->elements, &newType, sizeof(sdlTypePtr), (void **)&tmp);
			}
			if (type) {efree(type);}
			if (ns) {efree(ns);}

			start = next;
		}
		efree(str);
	}

	trav = unionType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"simpleType")) {
			sdlTypePtr newType, *tmp;

			newType = emalloc(sizeof(sdlType));
			memset(newType, 0, sizeof(sdlType));

			newType->name = estrdup("anonymous");
			newType->namens = estrdup(tns->children->content);

			if (cur_type->elements == NULL) {
				cur_type->elements = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
			}
			zend_hash_next_index_insert(cur_type->elements, &newType, sizeof(sdlTypePtr), (void **)&tmp);

			schema_simpleType(sdl, tns, trav, newType);

		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in union", trav->name);
		}
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in union", trav->name);
	}
	return TRUE;
}

/*
<simpleContent
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | extension))
</simpleContent>
*/
static int schema_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpCompType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;

	trav = simpCompType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav, "restriction")) {
			cur_type->kind = XSD_TYPEKIND_RESTRICTION;
			schema_restriction_simpleContent(sdl, tns, trav, cur_type, 0);
			trav = trav->next;
		} else if (node_is_equal(trav, "extension")) {
			cur_type->kind = XSD_TYPEKIND_EXTENSION;
			schema_extension_simpleContent(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in simpleContent", trav->name);
		}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: expected <restriction> or <extension> in simpleContent");
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in simpleContent", trav->name);
	}

	return TRUE;
}

/*
simpleType:<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern)*)?)
</restriction>
simpleContent:<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern)*)?, ((attribute | attributeGroup)*, anyAttribute?))
</restriction>
*/
static int schema_restriction_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type, int simpleType)
{
	xmlNodePtr trav;
	xmlAttrPtr base;

	base = get_attribute(restType->properties, "base");
	if (base != NULL) {
		char *type, *ns;
		xmlNsPtr nsptr;

		parse_namespace(base->children->content, &type, &ns);
		nsptr = xmlSearchNs(restType->doc, restType, ns);
		if (nsptr != NULL) {
			cur_type->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, type);
		}
		if (type) {efree(type);}
		if (ns) {efree(ns);}
	} else if (!simpleType) {
		soap_error0(E_ERROR, "Parsing Schema: restriction has no 'base' attribute");
	}

	if (cur_type->restrictions == NULL) {
		cur_type->restrictions = emalloc(sizeof(sdlRestrictions));
		memset(cur_type->restrictions, 0, sizeof(sdlRestrictions));
	}

	trav = restType->children;
	if (trav != NULL && node_is_equal(trav, "annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL && node_is_equal(trav, "simpleType")) {
		schema_simpleType(sdl, tns, trav, cur_type);
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav, "minExclusive")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->minExclusive);
		} else if (node_is_equal(trav, "minInclusive")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->minInclusive);
		} else if (node_is_equal(trav, "maxExclusive")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->maxExclusive);
		} else if (node_is_equal(trav, "maxInclusive")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->maxInclusive);
		} else if (node_is_equal(trav, "totalDigits")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->totalDigits);
		} else if (node_is_equal(trav, "fractionDigits")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->fractionDigits);
		} else if (node_is_equal(trav, "length")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->length);
		} else if (node_is_equal(trav, "minLength")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->minLength);
		} else if (node_is_equal(trav, "maxLength")) {
			schema_restriction_var_int(trav, &cur_type->restrictions->maxLength);
		} else if (node_is_equal(trav, "whiteSpace")) {
			schema_restriction_var_char(trav, &cur_type->restrictions->whiteSpace);
		} else if (node_is_equal(trav, "pattern")) {
			schema_restriction_var_char(trav, &cur_type->restrictions->pattern);
		} else if (node_is_equal(trav, "enumeration")) {
			sdlRestrictionCharPtr enumval = NULL;

			schema_restriction_var_char(trav, &enumval);
			if (cur_type->restrictions->enumeration == NULL) {
				cur_type->restrictions->enumeration = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->restrictions->enumeration, 0, NULL, delete_restriction_var_char, 0);
			}
			zend_hash_add(cur_type->restrictions->enumeration, enumval->value, strlen(enumval->value)+1, &enumval, sizeof(sdlRestrictionCharPtr), NULL);
		} else {
			break;
		}
		trav = trav->next;
	}
	if (!simpleType) {
		while (trav != NULL) {
			if (node_is_equal(trav,"attribute")) {
				schema_attribute(sdl, tns, trav, cur_type, NULL);
			} else if (node_is_equal(trav,"attributeGroup")) {
				schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
			} else if (node_is_equal(trav,"anyAttribute")) {
				/* TODO: <anyAttribute> support */
				trav = trav->next;
				break;
			} else {
				soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in restriction", trav->name);
			}
			trav = trav->next;
		}
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in restriction", trav->name);
	}

	return TRUE;
}

/*
<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?))
</restriction>
*/
static int schema_restriction_complexContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type)
{
	xmlAttrPtr base;
	xmlNodePtr trav;

	base = get_attribute(restType->properties, "base");
	if (base != NULL) {
		char *type, *ns;
		xmlNsPtr nsptr;

		parse_namespace(base->children->content, &type, &ns);
		nsptr = xmlSearchNs(restType->doc, restType, ns);
		if (nsptr != NULL) {
			cur_type->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, type);
		}
		if (type) {efree(type);}
		if (ns) {efree(ns);}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: restriction has no 'base' attribute");
	}

	trav = restType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"group")) {
			schema_group(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"all")) {
			schema_all(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"choice")) {
			schema_choice(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"sequence")) {
			schema_sequence(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		}
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"attribute")) {
			schema_attribute(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"attributeGroup")) {
			schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"anyAttribute")) {
			/* TODO: <anyAttribute> support */
			trav = trav->next;
			break;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in restriction", trav->name);
		}
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in restriction", trav->name);
	}

	return TRUE;
}

static int schema_restriction_var_int(xmlNodePtr val, sdlRestrictionIntPtr *valptr)
{
	xmlAttrPtr fixed, value;

	if ((*valptr) == NULL) {
		(*valptr) = emalloc(sizeof(sdlRestrictionInt));
	}
	memset((*valptr), 0, sizeof(sdlRestrictionInt));

	fixed = get_attribute(val->properties, "fixed");
	(*valptr)->fixed = FALSE;
	if (fixed != NULL) {
		if (!strncmp(fixed->children->content, "true", sizeof("true")) ||
			!strncmp(fixed->children->content, "1", sizeof("1")))
			(*valptr)->fixed = TRUE;
	}

	value = get_attribute(val->properties, "value");
	if (value == NULL) {
		soap_error0(E_ERROR, "Parsing Schema: missing restriction value");
	}

	(*valptr)->value = atoi(value->children->content);

	return TRUE;
}

static int schema_restriction_var_char(xmlNodePtr val, sdlRestrictionCharPtr *valptr)
{
	xmlAttrPtr fixed, value;

	if ((*valptr) == NULL) {
		(*valptr) = emalloc(sizeof(sdlRestrictionChar));
	}
	memset((*valptr), 0, sizeof(sdlRestrictionChar));

	fixed = get_attribute(val->properties, "fixed");
	(*valptr)->fixed = FALSE;
	if (fixed != NULL) {
		if (!strncmp(fixed->children->content, "true", sizeof("true")) ||
		    !strncmp(fixed->children->content, "1", sizeof("1"))) {
			(*valptr)->fixed = TRUE;
		}
	}

	value = get_attribute(val->properties, "value");
	if (value == NULL) {
		soap_error0(E_ERROR, "Parsing Schema: missing restriction value");
	}

	(*valptr)->value = estrdup(value->children->content);
	return TRUE;
}

/*
From simpleContent (not supported):
<extension
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((attribute | attributeGroup)*, anyAttribute?))
</extension>
*/
static int schema_extension_simpleContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr base;

	base = get_attribute(extType->properties, "base");
	if (base != NULL) {
		char *type, *ns;
		xmlNsPtr nsptr;

		parse_namespace(base->children->content, &type, &ns);
		nsptr = xmlSearchNs(extType->doc, extType, ns);
		if (nsptr != NULL) {
			cur_type->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, type);
		}
		if (type) {efree(type);}
		if (ns) {efree(ns);}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: extension has no 'base' attribute");
	}

	trav = extType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"attribute")) {
			schema_attribute(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"attributeGroup")) {
			schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"anyAttribute")) {
			/* TODO: <anyAttribute> support */
			trav = trav->next;
			break;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in extension", trav->name);
		}
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in extension", trav->name);
	}
	return TRUE;
}

/*
From complexContent:
<extension
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?)))
</extension>
*/
static int schema_extension_complexContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr base;

	base = get_attribute(extType->properties, "base");
	if (base != NULL) {
		char *type, *ns;
		xmlNsPtr nsptr;

		parse_namespace(base->children->content, &type, &ns);
		nsptr = xmlSearchNs(extType->doc, extType, ns);
		if (nsptr != NULL) {
			cur_type->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, type);
		}
		if (type) {efree(type);}
		if (ns) {efree(ns);}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: extension has no 'base' attribute");
	}

	trav = extType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"group")) {
			schema_group(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"all")) {
			schema_all(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"choice")) {
			schema_choice(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		} else if (node_is_equal(trav,"sequence")) {
			schema_sequence(sdl, tns, trav, cur_type, NULL);
			trav = trav->next;
		}
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"attribute")) {
			schema_attribute(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"attributeGroup")) {
			schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"anyAttribute")) {
			/* TODO: <anyAttribute> support */
			trav = trav->next;
			break;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in extension", trav->name);
		}
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in extension", trav->name);
	}
	return TRUE;
}

/*
<all
  id = ID
  maxOccurs = 1 : 1
  minOccurs = (0 | 1) : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, element*)
</all>
*/
static int schema_all(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr all, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	xmlNodePtr trav;
	xmlAttrPtr attr;
	sdlContentModelPtr newModel;

	newModel = emalloc(sizeof(sdlContentModel));
	newModel->kind = XSD_CONTENT_ALL;
	newModel->u.content = emalloc(sizeof(HashTable));
	zend_hash_init(newModel->u.content, 0, NULL, delete_model, 0);
	if (model == NULL) {
		cur_type->model = newModel;
	} else {
		zend_hash_next_index_insert(model->u.content,&newModel,sizeof(sdlContentModelPtr), NULL);
	}

	newModel->min_occurs = 1;
	newModel->max_occurs = 1;

	attr = get_attribute(all->properties, "minOccurs");
	if (attr) {
		newModel->min_occurs = atoi(attr->children->content);
	}

	attr = get_attribute(all->properties, "maxOccurs");
	if (attr) {
		if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
			newModel->max_occurs = -1;
		} else {
			newModel->max_occurs = atoi(attr->children->content);
		}
	}

	trav = all->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"element")) {
			schema_element(sdl, tns, trav, cur_type, newModel);
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in all", trav->name);
		}
		trav = trav->next;
	}
	return TRUE;
}

/*
<group
  name = NCName
  Content: (annotation?, (all | choice | sequence))
</group>
<group
  name = NCName
  ref = QName>
  Content: (annotation?)
</group>
*/
static int schema_group(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr groupType, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	xmlNodePtr trav;
	xmlAttrPtr attr;
	xmlAttrPtr ns, name, ref = NULL;
	sdlContentModelPtr newModel;

	ns = get_attribute(groupType->properties, "targetNamespace");
	if (ns == NULL) {
		ns = tns;
	}

	name = get_attribute(groupType->properties, "name");
	if (name == NULL) {
		name = ref = get_attribute(groupType->properties, "ref");
	}

	if (name) {
		smart_str key = {0};

		if (ref) {
			char *type, *ns;
			xmlNsPtr nsptr;

			parse_namespace(ref->children->content, &type, &ns);
			nsptr = xmlSearchNs(groupType->doc, groupType, ns);
			if (nsptr != NULL) {
				smart_str_appends(&key, nsptr->href);
				smart_str_appendc(&key, ':');
			}
			smart_str_appends(&key, type);
			smart_str_0(&key);

			newModel = emalloc(sizeof(sdlContentModel));
			newModel->kind = XSD_CONTENT_GROUP_REF;
			newModel->u.group_ref = estrdup(key.c);

			if (type) {efree(type);}
			if (ns) {efree(ns);}
		} else {
			newModel = emalloc(sizeof(sdlContentModel));
			newModel->kind = XSD_CONTENT_SEQUENCE; /* will be redefined */
			newModel->u.content = emalloc(sizeof(HashTable));
			zend_hash_init(newModel->u.content, 0, NULL, delete_model, 0);

			smart_str_appends(&key, ns->children->content);
			smart_str_appendc(&key, ':');
			smart_str_appends(&key, name->children->content);
			smart_str_0(&key);
		}

		if (cur_type == NULL) {
			sdlTypePtr newType;

			newType = emalloc(sizeof(sdlType));
			memset(newType, 0, sizeof(sdlType));

			if (sdl->groups == NULL) {
				sdl->groups = emalloc(sizeof(HashTable));
				zend_hash_init(sdl->groups, 0, NULL, delete_type, 0);
			}
			if (zend_hash_add(sdl->groups, key.c, key.len+1, (void**)&newType, sizeof(sdlTypePtr), NULL) != SUCCESS) {
				soap_error1(E_ERROR, "Parsing Schema: group '%s' already defined", key.c);
			}

			cur_type = newType;
		}
		smart_str_free(&key);

		if (model == NULL) {
			cur_type->model = newModel;
		} else {
			zend_hash_next_index_insert(model->u.content, &newModel, sizeof(sdlContentModelPtr), NULL);
		}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: group has no 'name' nor 'ref' attributes");
	}

	newModel->min_occurs = 1;
	newModel->max_occurs = 1;

	attr = get_attribute(groupType->properties, "minOccurs");
	if (attr) {
		newModel->min_occurs = atoi(attr->children->content);
	}

	attr = get_attribute(groupType->properties, "maxOccurs");
	if (attr) {
		if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
			newModel->max_occurs = -1;
		} else {
			newModel->max_occurs = atoi(attr->children->content);
		}
	}

	trav = groupType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"choice")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: group has both 'ref' attribute and subcontent");
			}
			newModel->kind = XSD_CONTENT_CHOICE;
			schema_choice(sdl, tns, trav, cur_type, newModel);
			trav = trav->next;
		} else if (node_is_equal(trav,"sequence")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: group has both 'ref' attribute and subcontent");
			}
			newModel->kind = XSD_CONTENT_SEQUENCE;
			schema_sequence(sdl, tns, trav, cur_type, newModel);
			trav = trav->next;
		} else if (node_is_equal(trav,"all")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: group has both 'ref' attribute and subcontent");
			}
			newModel->kind = XSD_CONTENT_ALL;
			schema_all(sdl, tns, trav, cur_type, newModel);
			trav = trav->next;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in group", trav->name);
		}
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in group", trav->name);
	}
	return TRUE;
}
/*
<choice
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (element | group | choice | sequence | any)*)
</choice>
*/
static int schema_choice(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr choiceType, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	xmlNodePtr trav;
	xmlAttrPtr attr;
	sdlContentModelPtr newModel;

	newModel = emalloc(sizeof(sdlContentModel));
	newModel->kind = XSD_CONTENT_CHOICE;
	newModel->u.content = emalloc(sizeof(HashTable));
	zend_hash_init(newModel->u.content, 0, NULL, delete_model, 0);
	if (model == NULL) {
		cur_type->model = newModel;
	} else {
		zend_hash_next_index_insert(model->u.content,&newModel,sizeof(sdlContentModelPtr), NULL);
	}

	newModel->min_occurs = 1;
	newModel->max_occurs = 1;

	attr = get_attribute(choiceType->properties, "minOccurs");
	if (attr) {
		newModel->min_occurs = atoi(attr->children->content);
	}

	attr = get_attribute(choiceType->properties, "maxOccurs");
	if (attr) {
		if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
			newModel->max_occurs = -1;
		} else {
			newModel->max_occurs = atoi(attr->children->content);
		}
	}

	trav = choiceType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"element")) {
			schema_element(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"group")) {
			schema_group(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"choice")) {
			schema_choice(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"sequence")) {
			schema_sequence(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"any")) {
			schema_any(sdl, tns, trav, cur_type, newModel);
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in choice", trav->name);
		}
		trav = trav->next;
	}
	return TRUE;
}

/*
<sequence
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (element | group | choice | sequence | any)*)
</sequence>
*/
static int schema_sequence(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr seqType, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	xmlNodePtr trav;
	xmlAttrPtr attr;
	sdlContentModelPtr newModel;

	newModel = emalloc(sizeof(sdlContentModel));
	newModel->kind = XSD_CONTENT_SEQUENCE;
	newModel->u.content = emalloc(sizeof(HashTable));
	zend_hash_init(newModel->u.content, 0, NULL, delete_model, 0);
	if (model == NULL) {
		cur_type->model = newModel;
	} else {
		zend_hash_next_index_insert(model->u.content,&newModel,sizeof(sdlContentModelPtr), NULL);
	}

	newModel->min_occurs = 1;
	newModel->max_occurs = 1;

	attr = get_attribute(seqType->properties, "minOccurs");
	if (attr) {
		newModel->min_occurs = atoi(attr->children->content);
	}

	attr = get_attribute(seqType->properties, "maxOccurs");
	if (attr) {
		if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
			newModel->max_occurs = -1;
		} else {
			newModel->max_occurs = atoi(attr->children->content);
		}
	}

	trav = seqType->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"element")) {
			schema_element(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"group")) {
			schema_group(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"choice")) {
			schema_choice(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"sequence")) {
			schema_sequence(sdl, tns, trav, cur_type, newModel);
		} else if (node_is_equal(trav,"any")) {
			schema_any(sdl, tns, trav, cur_type, newModel);
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in sequence", trav->name);
		}
		trav = trav->next;
	}
	return TRUE;
}

/*
<any 
  id = ID 
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  namespace = ((##any | ##other) | List of (anyURI | (##targetNamespace | ##local)) )  : ##any
  processContents = (lax | skip | strict) : strict
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?)
</any>
*/
static int schema_any(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr anyType, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	if (model != NULL) {
		sdlContentModelPtr newModel;
		xmlAttrPtr attr;

		newModel = emalloc(sizeof(sdlContentModel));
		newModel->kind = XSD_CONTENT_ANY;
		newModel->min_occurs = 1;
		newModel->max_occurs = 1;

		attr = get_attribute(anyType->properties, "minOccurs");
		if (attr) {
			newModel->min_occurs = atoi(attr->children->content);
		}

		attr = get_attribute(anyType->properties, "maxOccurs");
		if (attr) {
			if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
				newModel->max_occurs = -1;
			} else {
				newModel->max_occurs = atoi(attr->children->content);
			}
		}

		zend_hash_next_index_insert(model->u.content, &newModel, sizeof(sdlContentModelPtr), NULL);
	}
	return TRUE;
}

/*
<complexContent
  id = ID
  mixed = boolean
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | extension))
</complexContent>
*/
static int schema_complexContent(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr compCont, sdlTypePtr cur_type)
{
	xmlNodePtr trav;

	trav = compCont->children;
	if (trav != NULL && node_is_equal(trav,"annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav, "restriction")) {
			cur_type->kind = XSD_TYPEKIND_RESTRICTION;
			schema_restriction_complexContent(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else if (node_is_equal(trav, "extension")) {
			cur_type->kind = XSD_TYPEKIND_EXTENSION;
			schema_extension_complexContent(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in complexContent", trav->name);
		}
	} else {
		soap_error0(E_ERROR, "Parsing Schema: <restriction> or <extension> expected in complexContent");
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in complexContent", trav->name);
	}

	return TRUE;
}

/*
<complexType
  abstract = boolean : false
  block = (#all | List of (extension | restriction))
  final = (#all | List of (extension | restriction))
  id = ID
  mixed = boolean : false
  name = NCName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleContent | complexContent | ((group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?))))
</complexType>
*/
static int schema_complexType(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr compType, sdlTypePtr cur_type)
{
	xmlNodePtr trav;
	xmlAttrPtr attrs, name, ns;

	attrs = compType->properties;
	ns = get_attribute(attrs, "targetNamespace");
	if (ns == NULL) {
		ns = tns;
	}

	name = get_attribute(attrs, "name");
	if (cur_type != NULL) {
		/* Anonymous type inside <element> */
		sdlTypePtr newType, *ptr;

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));
		newType->kind = XSD_TYPEKIND_COMPLEX;
		if (name != NULL) {
			newType->name = estrdup(name->children->content);
			newType->namens = estrdup(ns->children->content);
		} else {
			newType->name = estrdup(cur_type->name);
			newType->namens = estrdup(cur_type->namens);
		}

		zend_hash_next_index_insert(sdl->types,  &newType, sizeof(sdlTypePtr), (void **)&ptr);

		if (sdl->encoders == NULL) {
			sdl->encoders = emalloc(sizeof(HashTable));
			zend_hash_init(sdl->encoders, 0, NULL, delete_encoder, 0);
		}
		cur_type->encode = emalloc(sizeof(encode));
		memset(cur_type->encode, 0, sizeof(encode));
		cur_type->encode->details.ns = estrdup(newType->namens);
		cur_type->encode->details.type_str = estrdup(newType->name);
		cur_type->encode->details.sdl_type = *ptr;
		cur_type->encode->to_xml = sdl_guess_convert_xml;
		cur_type->encode->to_zval = sdl_guess_convert_zval;
		zend_hash_next_index_insert(sdl->encoders,  &cur_type->encode, sizeof(encodePtr), NULL);

		cur_type =*ptr;

	} else if (name) {
		sdlTypePtr newType, *ptr;

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));
		newType->kind = XSD_TYPEKIND_COMPLEX;
		newType->name = estrdup(name->children->content);
		newType->namens = estrdup(ns->children->content);

		zend_hash_next_index_insert(sdl->types,  &newType, sizeof(sdlTypePtr), (void **)&ptr);

		cur_type = (*ptr);
		create_encoder(sdl, cur_type, ns->children->content, name->children->content);
	} else {
		soap_error0(E_ERROR, "Parsing Schema: complexType has no 'name' attribute");
		return FALSE;
	}

	trav = compType->children;
	if (trav != NULL && node_is_equal(trav, "annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"simpleContent")) {
			schema_simpleContent(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else if (node_is_equal(trav,"complexContent")) {
			schema_complexContent(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else {
			if (node_is_equal(trav,"group")) {
				schema_group(sdl, tns, trav, cur_type, NULL);
				trav = trav->next;
			} else if (node_is_equal(trav,"all")) {
				schema_all(sdl, tns, trav, cur_type, NULL);
				trav = trav->next;
			} else if (node_is_equal(trav,"choice")) {
				schema_choice(sdl, tns, trav, cur_type, NULL);
				trav = trav->next;
			} else if (node_is_equal(trav,"sequence")) {
				schema_sequence(sdl, tns, trav, cur_type, NULL);
				trav = trav->next;
			}
			while (trav != NULL) {
				if (node_is_equal(trav,"attribute")) {
					schema_attribute(sdl, tns, trav, cur_type, NULL);
				} else if (node_is_equal(trav,"attributeGroup")) {
					schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
				} else if (node_is_equal(trav,"anyAttribute")) {
					/* TODO: <anyAttribute> support */
					trav = trav->next;
					break;
				} else {
					soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in complexType", trav->name);
				}
				trav = trav->next;
			}
		}
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in complexType", trav->name);
	}
	return TRUE;
}
/*
<element
  abstract = boolean : false
  block = (#all | List of (extension | restriction | substitution))
  default = string
  final = (#all | List of (extension | restriction))
  fixed = string
  form = (qualified | unqualified)
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  name = NCName
  nillable = boolean : false
  ref = QName
  substitutionGroup = QName
  type = QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((simpleType | complexType)?, (unique | key | keyref)*))
</element>
*/
static int schema_element(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr element, sdlTypePtr cur_type, sdlContentModelPtr model)
{
	xmlNodePtr trav;
	xmlAttrPtr attrs, attr, ns, name, type, ref = NULL;

	attrs = element->properties;
	ns = get_attribute(attrs, "targetNamespace");
	if (ns == NULL) {
		ns = tns;
	}

	name = get_attribute(attrs, "name");
	if (name == NULL) {
		name = ref = get_attribute(attrs, "ref");
	}

	if (name) {
		HashTable *addHash;
		sdlTypePtr newType;
		smart_str key = {0};

		newType = emalloc(sizeof(sdlType));
		memset(newType, 0, sizeof(sdlType));

		if (ref) {
			smart_str nscat = {0};
			char *type, *ns;
			xmlNsPtr nsptr;

			parse_namespace(ref->children->content, &type, &ns);
			nsptr = xmlSearchNs(element->doc, element, ns);
			if (nsptr != NULL) {
				smart_str_appends(&nscat, nsptr->href);
				smart_str_appendc(&nscat, ':');
				newType->namens = estrdup(nsptr->href);
			}
			smart_str_appends(&nscat, type);
			newType->name = estrdup(type);
			smart_str_0(&nscat);
			if (type) {efree(type);}
			if (ns) {efree(ns);}
			newType->ref = estrdup(nscat.c);
			smart_str_free(&nscat);
		} else {
			newType->name = estrdup(name->children->content);
			newType->namens = estrdup(ns->children->content);
		}

		newType->nillable = FALSE;

		if (cur_type == NULL) {
			if (sdl->elements == NULL) {
				sdl->elements = emalloc(sizeof(HashTable));
				zend_hash_init(sdl->elements, 0, NULL, delete_type, 0);
			}
			addHash = sdl->elements;
			smart_str_appends(&key, newType->namens);
			smart_str_appendc(&key, ':');
			smart_str_appends(&key, newType->name);
		} else {
			if (cur_type->elements == NULL) {
				cur_type->elements = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->elements, 0, NULL, delete_type, 0);
			}
			addHash = cur_type->elements;
			smart_str_appends(&key, newType->name);
		}

		smart_str_0(&key);
		if (zend_hash_add(addHash, key.c, key.len + 1, &newType, sizeof(sdlTypePtr), NULL) != SUCCESS) {
			if (cur_type == NULL) {
				soap_error1(E_ERROR, "Parsing Schema: element '%s' already defined", key.c);
			} else {
				zend_hash_next_index_insert(addHash, &newType, sizeof(sdlTypePtr), NULL);
			}
		}
		smart_str_free(&key);

		if (model != NULL) {
			sdlContentModelPtr newModel = emalloc(sizeof(sdlContentModel));

			newModel->kind = XSD_CONTENT_ELEMENT;
			newModel->u.element = newType;
			newModel->min_occurs = 1;
			newModel->max_occurs = 1;

			attr = get_attribute(attrs, "maxOccurs");
			if (attr) {
				if (!strncmp(attr->children->content, "unbounded", sizeof("unbounded"))) {
					newModel->max_occurs = -1;
				} else {
					newModel->max_occurs = atoi(attr->children->content);
				}
			}

			attr = get_attribute(attrs, "minOccurs");
			if (attr) {
				newModel->min_occurs = atoi(attr->children->content);
			}

			zend_hash_next_index_insert(model->u.content, &newModel, sizeof(sdlContentModelPtr), NULL);
		}
		cur_type = newType;
	} else {
		soap_error0(E_ERROR, "Parsing Schema: element has no 'name' nor 'ref' attributes");
	}

	/* nillable = boolean : false */
	attrs = element->properties;
	attr = get_attribute(attrs, "nillable");
	if (attr) {
		if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' and 'nillable' attributes");
		}
		if (!stricmp(attr->children->content, "true") ||
			!stricmp(attr->children->content, "1")) {
			cur_type->nillable = TRUE;
		} else {
			cur_type->nillable = FALSE;
		}
	} else {
		cur_type->nillable = FALSE;
	}

	attr = get_attribute(attrs, "fixed");
	if (attr) {
		if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' and 'fixed' attributes");
		}
		cur_type->fixed = estrdup(attr->children->content);
	}

	attr = get_attribute(attrs, "default");
	if (attr) {
		if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' and 'fixed' attributes");
		} else if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'default' and 'fixed' attributes");
		}
		cur_type->def = estrdup(attr->children->content);
	}

	/* form */
	attr = get_attribute(attrs, "form");
	if (attr) {
		if (strncmp(attr->children->content,"qualified",sizeof("qualified")) == 0) {
		  cur_type->form = XSD_FORM_QUALIFIED;
		} else if (strncmp(attr->children->content,"unqualified",sizeof("unqualified")) == 0) {
		  cur_type->form = XSD_FORM_UNQUALIFIED;
		} else {
		  cur_type->form = XSD_FORM_DEFAULT;
		}
	} else {
	  cur_type->form = XSD_FORM_DEFAULT;
	}
	if (cur_type->form == XSD_FORM_DEFAULT) {
 		xmlNodePtr parent = element->parent;
 		while (parent) {
			if (node_is_equal_ex(parent, "schema", SCHEMA_NAMESPACE)) {
				xmlAttrPtr def;
				def = get_attribute(parent->properties, "elementFormDefault");
				if(def == NULL || strncmp(def->children->content, "qualified", sizeof("qualified"))) {
					cur_type->form = XSD_FORM_UNQUALIFIED;
				} else {
					cur_type->form = XSD_FORM_QUALIFIED;
				}
				break;
			}
			parent = parent->parent;
  	}
		if (parent == NULL) {
			cur_type->form = XSD_FORM_UNQUALIFIED;
		}	
	}

	/* type = QName */
	type = get_attribute(attrs, "type");
	if (type) {
		char *cptype, *str_ns;
		xmlNsPtr nsptr;

		if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' and 'type' attributes");
		}
		parse_namespace(type->children->content, &cptype, &str_ns);
		nsptr = xmlSearchNs(element->doc, element, str_ns);
		if (nsptr != NULL) {
			cur_type->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, (char *)cptype);
		}
		if (str_ns) {efree(str_ns);}
		if (cptype) {efree(cptype);}
	}

	trav = element->children;
	if (trav != NULL && node_is_equal(trav, "annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"simpleType")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' attribute and subtype");
			} else if (type != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: element has both 'type' attribute and subtype");
			}
			schema_simpleType(sdl, tns, trav, cur_type);
			trav = trav->next;
		} else if (node_is_equal(trav,"complexType")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: element has both 'ref' attribute and subtype");
			} else if (type != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: element has both 'type' attribute and subtype");
			}
			schema_complexType(sdl, tns, trav, cur_type);
			trav = trav->next;
		}
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"unique")) {
			/* TODO: <unique> support */
		} else if (node_is_equal(trav,"key")) {
			/* TODO: <key> support */
		} else if (node_is_equal(trav,"keyref")) {
			/* TODO: <keyref> support */
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in element", trav->name);
		}
		trav = trav->next;
	}

	return TRUE;
}

/*
<attribute
  default = string
  fixed = string
  form = (qualified | unqualified)
  id = ID
  name = NCName
  ref = QName
  type = QName
  use = (optional | prohibited | required) : optional
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?))
</attribute>
*/
static int schema_attribute(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType, sdlTypePtr cur_type, sdlCtx *ctx)
{
	sdlAttributePtr newAttr;
	xmlAttrPtr attr, name, ref = NULL, type = NULL;
	xmlNodePtr trav;

	name = get_attribute(attrType->properties, "name");
	if (name == NULL) {
		name = ref = get_attribute(attrType->properties, "ref");
	}
	if (name) {
		HashTable *addHash;
		smart_str key = {0};

		newAttr = emalloc(sizeof(sdlAttribute));
		memset(newAttr, 0, sizeof(sdlAttribute));

		if (ref) {
			char *attr_name, *ns;
			xmlNsPtr nsptr;

			parse_namespace(ref->children->content, &attr_name, &ns);
			nsptr = xmlSearchNs(attrType->doc, attrType, ns);
			if (nsptr != NULL) {
				smart_str_appends(&key, nsptr->href);
				smart_str_appendc(&key, ':');
				newAttr->namens = estrdup(nsptr->href);
			}
			smart_str_appends(&key, attr_name);
			smart_str_0(&key);
			newAttr->ref = estrdup(key.c);
			if (attr_name) {efree(attr_name);}
			if (ns) {efree(ns);}
		} else {
			xmlAttrPtr ns;

			ns = get_attribute(attrType->properties, "targetNamespace");
			if (ns == NULL) {
				ns = tns;
			}
			if (ns != NULL) {
				smart_str_appends(&key, ns->children->content);
				smart_str_appendc(&key, ':');
				newAttr->namens = estrdup(ns->children->content);
			}
			smart_str_appends(&key, name->children->content);
			smart_str_0(&key);
		}

		if (cur_type == NULL) {
			addHash = ctx->attributes;
		} else {
			if (cur_type->attributes == NULL) {
				cur_type->attributes = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->attributes, 0, NULL, delete_attribute, 0);
			}
			addHash = cur_type->attributes;
		}

		if (zend_hash_add(addHash, key.c, key.len + 1, &newAttr, sizeof(sdlAttributePtr), NULL) != SUCCESS) {
			soap_error1(E_ERROR, "Parsing Schema: attribute '%s' already defined", key.c);
		}
		smart_str_free(&key);
	} else{
		soap_error0(E_ERROR, "Parsing Schema: attribute has no 'name' nor 'ref' attributes");
	}

	/* type = QName */
	type = get_attribute(attrType->properties, "type");
	if (type) {
		char *cptype, *str_ns;
		xmlNsPtr nsptr;

		if (ref != NULL) {
			soap_error0(E_ERROR, "Parsing Schema: attribute has both 'ref' and 'type' attributes");
		}
		parse_namespace(type->children->content, &cptype, &str_ns);
		nsptr = xmlSearchNs(attrType->doc, attrType, str_ns);
		if (nsptr != NULL) {
			newAttr->encode = get_create_encoder(sdl, cur_type, (char *)nsptr->href, (char *)cptype);
		}
		if (str_ns) {efree(str_ns);}
		if (cptype) {efree(cptype);}
	}

	attr = attrType->properties;
	while (attr != NULL) {
		if (attr_is_equal_ex(attr, "default", SCHEMA_NAMESPACE)) {
			newAttr->def = estrdup(attr->children->content);
		} else if (attr_is_equal_ex(attr, "fixed", SCHEMA_NAMESPACE)) {
			newAttr->fixed = estrdup(attr->children->content);
		} else if (attr_is_equal_ex(attr, "form", SCHEMA_NAMESPACE)) {
			if (strncmp(attr->children->content,"qualified",sizeof("qualified")) == 0) {
			  newAttr->form = XSD_FORM_QUALIFIED;
			} else if (strncmp(attr->children->content,"unqualified",sizeof("unqualified")) == 0) {
			  newAttr->form = XSD_FORM_UNQUALIFIED;
			} else {
			  newAttr->form = XSD_FORM_DEFAULT;
			}
		} else if (attr_is_equal_ex(attr, "id", SCHEMA_NAMESPACE)) {
			/* skip */
		} else if (attr_is_equal_ex(attr, "name", SCHEMA_NAMESPACE)) {
			newAttr->name = estrdup(attr->children->content);
		} else if (attr_is_equal_ex(attr, "ref", SCHEMA_NAMESPACE)) {
			/* already processed */
		} else if (attr_is_equal_ex(attr, "type", SCHEMA_NAMESPACE)) {
			/* already processed */
		} else if (attr_is_equal_ex(attr, "use", SCHEMA_NAMESPACE)) {
			if (strncmp(attr->children->content,"prohibited",sizeof("prohibited")) == 0) {
			  newAttr->use = XSD_USE_PROHIBITED;
			} else if (strncmp(attr->children->content,"required",sizeof("required")) == 0) {
			  newAttr->use = XSD_USE_REQUIRED;
			} else if (strncmp(attr->children->content,"optional",sizeof("optional")) == 0) {
			  newAttr->use = XSD_USE_OPTIONAL;
			} else {
			  newAttr->use = XSD_USE_DEFAULT;
			}
		} else {
			xmlNsPtr nsPtr = attr_find_ns(attr);

			if (strncmp(nsPtr->href, SCHEMA_NAMESPACE, sizeof(SCHEMA_NAMESPACE))) {
				smart_str key2 = {0};
				sdlExtraAttributePtr ext;
				xmlNsPtr nsptr;
				char *value, *ns;

				ext = emalloc(sizeof(sdlExtraAttribute));
				memset(ext, 0, sizeof(sdlExtraAttribute));
				parse_namespace(attr->children->content, &value, &ns);
				nsptr = xmlSearchNs(attr->doc, attr->parent, ns);
				if (nsptr) {
					ext->ns = estrdup(nsptr->href);
					ext->val = estrdup(value);
				} else {
					ext->val = estrdup(attr->children->content);
				}
				if (ns) {efree(ns);}
				efree(value);

				if (!newAttr->extraAttributes) {
					newAttr->extraAttributes = emalloc(sizeof(HashTable));
					zend_hash_init(newAttr->extraAttributes, 0, NULL, delete_extra_attribute, 0);
				}

				smart_str_appends(&key2, nsPtr->href);
				smart_str_appendc(&key2, ':');
				smart_str_appends(&key2, attr->name);
				smart_str_0(&key2);
				zend_hash_add(newAttr->extraAttributes, key2.c, key2.len + 1, &ext, sizeof(sdlExtraAttributePtr), NULL);
				smart_str_free(&key2);
			}
		}
		attr = attr->next;
	}
	if (newAttr->form == XSD_FORM_DEFAULT) {
 		xmlNodePtr parent = attrType->parent;
 		while (parent) {
			if (node_is_equal_ex(parent, "schema", SCHEMA_NAMESPACE)) {
				xmlAttrPtr def;
				def = get_attribute(parent->properties, "attributeFormDefault");
				if(def == NULL || strncmp(def->children->content, "qualified", sizeof("qualified"))) {
					newAttr->form = XSD_FORM_UNQUALIFIED;
				} else {
					newAttr->form = XSD_FORM_QUALIFIED;
				}
				break;
			}
			parent = parent->parent;
  	}
		if (parent == NULL) {
			newAttr->form = XSD_FORM_UNQUALIFIED;
		}	
	}
	trav = attrType->children;
	if (trav != NULL && node_is_equal(trav, "annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	if (trav != NULL) {
		if (node_is_equal(trav,"simpleType")) {
			sdlTypePtr dummy_type;
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: attribute has both 'ref' attribute and subtype");
			} else if (type != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: attribute has both 'type' attribute and subtype");
			}
			dummy_type = emalloc(sizeof(sdlType));
			memset(dummy_type, 0, sizeof(sdlType));
			dummy_type->name = estrdup("anonymous");
			dummy_type->namens = estrdup(tns->children->content);
			schema_simpleType(sdl, tns, trav, dummy_type);
			newAttr->encode = dummy_type->encode;
			delete_type(&dummy_type);
			trav = trav->next;
		}
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in attribute", trav->name);
	}
	return TRUE;
}

static int schema_attributeGroup(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrGroup, sdlTypePtr cur_type, sdlCtx *ctx)
{
	xmlNodePtr trav;
	xmlAttrPtr name, ref = NULL;


	name = get_attribute(attrGroup->properties, "name");
	if (name == NULL) {
		name = ref = get_attribute(attrGroup->properties, "ref");
	}
	if (name) {
		if (cur_type == NULL) {
			xmlAttrPtr ns;
			sdlTypePtr newType;
			smart_str key = {0};

			ns = get_attribute(attrGroup->properties, "targetNamespace");
			if (ns == NULL) {
				ns = tns;
			}
			newType = emalloc(sizeof(sdlType));
			memset(newType, 0, sizeof(sdlType));
			newType->name = estrdup(name->children->content);
			newType->namens = estrdup(ns->children->content);

			smart_str_appends(&key, newType->namens);
			smart_str_appendc(&key, ':');
			smart_str_appends(&key, newType->name);
			smart_str_0(&key);

			if (zend_hash_add(ctx->attributeGroups, key.c, key.len + 1, &newType, sizeof(sdlTypePtr), NULL) != SUCCESS) {
				soap_error1(E_ERROR, "Parsing Schema: attributeGroup '%s' already defined", key.c);
			}
			cur_type = newType;
			smart_str_free(&key);
		} else if (ref) {
			sdlAttributePtr newAttr;
			char *group_name, *ns;
			smart_str key = {0};
			xmlNsPtr nsptr;

			if (cur_type->attributes == NULL) {
				cur_type->attributes = emalloc(sizeof(HashTable));
				zend_hash_init(cur_type->attributes, 0, NULL, delete_attribute, 0);
			}
			newAttr = emalloc(sizeof(sdlAttribute));
			memset(newAttr, 0, sizeof(sdlAttribute));

			parse_namespace(ref->children->content, &group_name, &ns);
			nsptr = xmlSearchNs(attrGroup->doc, attrGroup, ns);
			if (nsptr != NULL) {
				smart_str_appends(&key, nsptr->href);
				smart_str_appendc(&key, ':');
			}
			smart_str_appends(&key, group_name);
			smart_str_0(&key);
			newAttr->ref = estrdup(key.c);
			if (group_name) {efree(group_name);}
			if (ns) {efree(ns);}
			smart_str_free(&key);

			zend_hash_next_index_insert(cur_type->attributes, &newAttr, sizeof(sdlAttributePtr), NULL);
			cur_type = NULL;
		}
	} else{
		soap_error0(E_ERROR, "Parsing Schema: attributeGroup has no 'name' nor 'ref' attributes");
	}

	trav = attrGroup->children;
	if (trav != NULL && node_is_equal(trav, "annotation")) {
		/* TODO: <annotation> support */
		trav = trav->next;
	}
	while (trav != NULL) {
		if (node_is_equal(trav,"attribute")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: attributeGroup has both 'ref' attribute and subattribute");
			}
			schema_attribute(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"attributeGroup")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: attributeGroup has both 'ref' attribute and subattribute");
			}
			schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
		} else if (node_is_equal(trav,"anyAttribute")) {
			if (ref != NULL) {
				soap_error0(E_ERROR, "Parsing Schema: attributeGroup has both 'ref' attribute and subattribute");
			}
			/* TODO: <anyAttribute> support */
			trav = trav->next;
			break;
		} else {
			soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in attributeGroup", trav->name);
		}
		trav = trav->next;
	}
	if (trav != NULL) {
		soap_error1(E_ERROR, "Parsing Schema: unexpected <%s> in attributeGroup", trav->name);
	}
	return TRUE;
}

static void copy_extra_attribute(void *attribute)
{
	sdlExtraAttributePtr *attr = (sdlExtraAttributePtr*)attribute;
	sdlExtraAttributePtr new_attr;

	new_attr = emalloc(sizeof(sdlExtraAttribute));
	memcpy(new_attr, *attr, sizeof(sdlExtraAttribute));
	*attr = new_attr;
	if (new_attr->ns) {
		new_attr->ns = estrdup(new_attr->ns);
	}
	if (new_attr->val) {
		new_attr->val = estrdup(new_attr->val);
	}
}

static void schema_attribute_fixup(sdlCtx *ctx, sdlAttributePtr attr)
{
	sdlAttributePtr *tmp;

	if (attr->ref != NULL) {
		if (ctx->attributes != NULL) {
			if (zend_hash_find(ctx->attributes, attr->ref, strlen(attr->ref)+1, (void**)&tmp) == SUCCESS) {
				schema_attribute_fixup(ctx, *tmp);
				if ((*tmp)->name != NULL && attr->name == NULL) {
					attr->name = estrdup((*tmp)->name);
				}
				if ((*tmp)->namens != NULL && attr->namens == NULL) {
					attr->namens = estrdup((*tmp)->namens);
				}
				if ((*tmp)->def != NULL && attr->def == NULL) {
					attr->def = estrdup((*tmp)->def);
				}
				if ((*tmp)->fixed != NULL && attr->fixed == NULL) {
					attr->fixed = estrdup((*tmp)->fixed);
				}
				if (attr->form == XSD_FORM_DEFAULT) {
					attr->form = (*tmp)->form;
				}
				if (attr->use == XSD_USE_DEFAULT) {
					attr->use  = (*tmp)->use;
				}
				if ((*tmp)->extraAttributes != NULL) {
				  xmlNodePtr node;

					attr->extraAttributes = emalloc(sizeof(HashTable));
					zend_hash_init(attr->extraAttributes, 0, NULL, delete_extra_attribute, 0);
					zend_hash_copy(attr->extraAttributes, (*tmp)->extraAttributes, copy_extra_attribute, &node, sizeof(xmlNodePtr));
				}
				attr->encode = (*tmp)->encode;
			}
		}
		if (attr->name == NULL && attr->ref != NULL) {
			char *name = strrchr(attr->ref, ':');
			if (name) {
				attr->name = estrdup(name+1);
			} else{
				attr->name = estrdup(attr->ref);
			}
		}
		efree(attr->ref);
		attr->ref = NULL;
	}
}

static void schema_attributegroup_fixup(sdlCtx *ctx, sdlAttributePtr attr, HashTable *ht)
{
	sdlTypePtr *tmp;
	sdlAttributePtr *tmp_attr;

	if (attr->ref != NULL) {
		if (ctx->attributeGroups != NULL) {
			if (zend_hash_find(ctx->attributeGroups, attr->ref, strlen(attr->ref)+1, (void**)&tmp) == SUCCESS) {
				if ((*tmp)->attributes) {
					zend_hash_internal_pointer_reset((*tmp)->attributes);
					while (zend_hash_get_current_data((*tmp)->attributes,(void**)&tmp_attr) == SUCCESS) {
						if (zend_hash_get_current_key_type((*tmp)->attributes) == HASH_KEY_IS_STRING) {
							char* key;
							uint key_len;
							sdlAttributePtr newAttr;

							schema_attribute_fixup(ctx,*tmp_attr);

							newAttr = emalloc(sizeof(sdlAttribute));
							memcpy(newAttr, *tmp_attr, sizeof(sdlAttribute));
							if (newAttr->def) {newAttr->def = estrdup(newAttr->def);}
							if (newAttr->fixed) {newAttr->fixed = estrdup(newAttr->fixed);}
							if (newAttr->namens) {newAttr->namens = estrdup(newAttr->namens);}
							if (newAttr->name) {newAttr->name = estrdup(newAttr->name);}
							if (newAttr->extraAttributes) {
							  xmlNodePtr node;
								HashTable *ht = emalloc(sizeof(HashTable));
								zend_hash_init(ht, 0, NULL, delete_extra_attribute, 0);
								zend_hash_copy(ht, newAttr->extraAttributes, copy_extra_attribute, &node, sizeof(xmlNodePtr));
								newAttr->extraAttributes = ht;
							}

							zend_hash_get_current_key_ex((*tmp)->attributes, &key, &key_len, NULL, 0, NULL);
							zend_hash_add(ht, key, key_len, &newAttr, sizeof(sdlAttributePtr), NULL);

							zend_hash_move_forward((*tmp)->attributes);
						} else {
							ulong index;

							schema_attributegroup_fixup(ctx,*tmp_attr, ht);
							zend_hash_get_current_key((*tmp)->attributes, NULL, &index, 0);
							zend_hash_index_del((*tmp)->attributes, index);
						}
					}
				}
			}
		}
		efree(attr->ref);
		attr->ref = NULL;
	}
}

static void schema_content_model_fixup(sdlCtx *ctx, sdlContentModelPtr model)
{
	switch (model->kind) {
		case XSD_CONTENT_GROUP_REF: {
			sdlTypePtr *tmp;

			if (ctx->sdl->groups && zend_hash_find(ctx->sdl->groups, model->u.group_ref, strlen(model->u.group_ref)+1, (void**)&tmp) == SUCCESS) {
				schema_type_fixup(ctx,*tmp);
				efree(model->u.group_ref);
				model->kind = XSD_CONTENT_GROUP;
				model->u.group = (*tmp);
			} else {
				soap_error0(E_ERROR, "Parsing Schema: unresolved group 'ref' attribute");
			}
			break;
		}
		case XSD_CONTENT_CHOICE: {
			if (model->max_occurs != 1) {
				HashPosition pos;
				sdlContentModelPtr *tmp;

				zend_hash_internal_pointer_reset_ex(model->u.content, &pos);
				while (zend_hash_get_current_data_ex(model->u.content, (void**)&tmp, &pos) == SUCCESS) {
					(*tmp)->min_occurs = 0;
					(*tmp)->max_occurs = model->max_occurs;
					zend_hash_move_forward_ex(model->u.content, &pos);
				}

				model->kind = XSD_CONTENT_ALL;
				model->min_occurs = 1;
				model->max_occurs = 1;
			}
		}
		case XSD_CONTENT_SEQUENCE:
		case XSD_CONTENT_ALL: {
			sdlContentModelPtr *tmp;

			zend_hash_internal_pointer_reset(model->u.content);
			while (zend_hash_get_current_data(model->u.content, (void**)&tmp) == SUCCESS) {
				schema_content_model_fixup(ctx, *tmp);
				zend_hash_move_forward(model->u.content);
			}
			break;
		}
		default:
			break;
	}
}

static void schema_type_fixup(sdlCtx *ctx, sdlTypePtr type)
{
	sdlTypePtr *tmp;
	sdlAttributePtr *attr;

	if (type->ref != NULL) {
		if (ctx->sdl->elements != NULL) {
			if (zend_hash_find(ctx->sdl->elements, type->ref, strlen(type->ref)+1, (void**)&tmp) == SUCCESS) {
				type->kind = (*tmp)->kind;
				type->encode = (*tmp)->encode;
				if ((*tmp)->nillable) {
				  type->nillable = 1;
				}
				if ((*tmp)->fixed) {
				  type->fixed = estrdup((*tmp)->fixed);
				}
				if ((*tmp)->def) {
				  type->def = estrdup((*tmp)->def);
				}
				type->form = (*tmp)->form;
			} else if (strcmp(type->ref, SCHEMA_NAMESPACE ":schema") == 0) {
				type->encode = get_conversion(XSD_ANYXML);
			} else {
				soap_error0(E_ERROR, "Parsing Schema: unresolved element 'ref' attribute");
			}
		}
		efree(type->ref);
		type->ref = NULL;
	}
	if (type->elements) {
		zend_hash_internal_pointer_reset(type->elements);
		while (zend_hash_get_current_data(type->elements,(void**)&tmp) == SUCCESS) {
			schema_type_fixup(ctx,*tmp);
			zend_hash_move_forward(type->elements);
		}
	}
	if (type->model) {
		schema_content_model_fixup(ctx, type->model);
	}
	if (type->attributes) {
		zend_hash_internal_pointer_reset(type->attributes);
		while (zend_hash_get_current_data(type->attributes,(void**)&attr) == SUCCESS) {
			if (zend_hash_get_current_key_type(type->attributes) == HASH_KEY_IS_STRING) {
				schema_attribute_fixup(ctx,*attr);
				zend_hash_move_forward(type->attributes);
			} else {
				ulong index;

				schema_attributegroup_fixup(ctx,*attr,type->attributes);
				zend_hash_get_current_key(type->attributes, NULL, &index, 0);
				zend_hash_index_del(type->attributes, index);
			}
		}
	}
}

void schema_pass2(sdlCtx *ctx)
{
	sdlPtr sdl = ctx->sdl;
	sdlAttributePtr *attr;
	sdlTypePtr *type;

	if (ctx->attributes) {
		zend_hash_internal_pointer_reset(ctx->attributes);
		while (zend_hash_get_current_data(ctx->attributes,(void**)&attr) == SUCCESS) {
			schema_attribute_fixup(ctx,*attr);
			zend_hash_move_forward(ctx->attributes);
		}
	}
	if (ctx->attributeGroups) {
		zend_hash_internal_pointer_reset(ctx->attributeGroups);
		while (zend_hash_get_current_data(ctx->attributeGroups,(void**)&type) == SUCCESS) {
			schema_type_fixup(ctx,*type);
			zend_hash_move_forward(ctx->attributeGroups);
		}
	}
	if (sdl->elements) {
		zend_hash_internal_pointer_reset(sdl->elements);
		while (zend_hash_get_current_data(sdl->elements,(void**)&type) == SUCCESS) {
			schema_type_fixup(ctx,*type);
			zend_hash_move_forward(sdl->elements);
		}
	}
	if (sdl->groups) {
		zend_hash_internal_pointer_reset(sdl->groups);
		while (zend_hash_get_current_data(sdl->groups,(void**)&type) == SUCCESS) {
			schema_type_fixup(ctx,*type);
			zend_hash_move_forward(sdl->groups);
		}
	}
	if (sdl->types) {
		zend_hash_internal_pointer_reset(sdl->types);
		while (zend_hash_get_current_data(sdl->types,(void**)&type) == SUCCESS) {
			schema_type_fixup(ctx,*type);
			zend_hash_move_forward(sdl->types);
		}
	}
	if (ctx->attributes) {
		zend_hash_destroy(ctx->attributes);
		efree(ctx->attributes);
	}
	if (ctx->attributeGroups) {
		zend_hash_destroy(ctx->attributeGroups);
		efree(ctx->attributeGroups);
	}
}

void delete_model(void *handle)
{
	sdlContentModelPtr tmp = *((sdlContentModelPtr*)handle);
	switch (tmp->kind) {
		case XSD_CONTENT_ELEMENT:
		case XSD_CONTENT_GROUP:
			break;
		case XSD_CONTENT_SEQUENCE:
		case XSD_CONTENT_ALL:
		case XSD_CONTENT_CHOICE:
			zend_hash_destroy(tmp->u.content);
			efree(tmp->u.content);
			break;
		case XSD_CONTENT_GROUP_REF:
			efree(tmp->u.group_ref);
			break;
		default:
			break;
	}
	efree(tmp);
}

void delete_model_persistent(void *handle)
{
	sdlContentModelPtr tmp = *((sdlContentModelPtr*)handle);
	switch (tmp->kind) {
		case XSD_CONTENT_ELEMENT:
		case XSD_CONTENT_GROUP:
			break;
		case XSD_CONTENT_SEQUENCE:
		case XSD_CONTENT_ALL:
		case XSD_CONTENT_CHOICE:
			zend_hash_destroy(tmp->u.content);
			free(tmp->u.content);
			break;
		case XSD_CONTENT_GROUP_REF:
			free(tmp->u.group_ref);
			break;
		default:
			break;
	}
	free(tmp);
}

void delete_type(void *data)
{
	sdlTypePtr type = *((sdlTypePtr*)data);
	if (type->name) {
		efree(type->name);
	}
	if (type->namens) {
		efree(type->namens);
	}
	if (type->def) {
		efree(type->def);
	}
	if (type->fixed) {
		efree(type->fixed);
	}
	if (type->elements) {
		zend_hash_destroy(type->elements);
		efree(type->elements);
	}
	if (type->attributes) {
		zend_hash_destroy(type->attributes);
		efree(type->attributes);
	}
	if (type->model) {
		delete_model((void**)&type->model);
	}
	if (type->restrictions) {
		delete_restriction_var_int(&type->restrictions->minExclusive);
		delete_restriction_var_int(&type->restrictions->minInclusive);
		delete_restriction_var_int(&type->restrictions->maxExclusive);
		delete_restriction_var_int(&type->restrictions->maxInclusive);
		delete_restriction_var_int(&type->restrictions->totalDigits);
		delete_restriction_var_int(&type->restrictions->fractionDigits);
		delete_restriction_var_int(&type->restrictions->length);
		delete_restriction_var_int(&type->restrictions->minLength);
		delete_restriction_var_int(&type->restrictions->maxLength);
		delete_restriction_var_char(&type->restrictions->whiteSpace);
		delete_restriction_var_char(&type->restrictions->pattern);
		if (type->restrictions->enumeration) {
			zend_hash_destroy(type->restrictions->enumeration);
			efree(type->restrictions->enumeration);
		}
		efree(type->restrictions);
	}
	efree(type);
}

void delete_type_persistent(void *data)
{
	sdlTypePtr type = *((sdlTypePtr*)data);
	if (type->name) {
		free(type->name);
	}
	if (type->namens) {
		free(type->namens);
	}
	if (type->def) {
		free(type->def);
	}
	if (type->fixed) {
		free(type->fixed);
	}
	if (type->elements) {
		zend_hash_destroy(type->elements);
		free(type->elements);
	}
	if (type->attributes) {
		zend_hash_destroy(type->attributes);
		free(type->attributes);
	}
	if (type->model) {
		delete_model_persistent((void**)&type->model);
	}
	if (type->restrictions) {
		delete_restriction_var_int_persistent(&type->restrictions->minExclusive);
		delete_restriction_var_int_persistent(&type->restrictions->minInclusive);
		delete_restriction_var_int_persistent(&type->restrictions->maxExclusive);
		delete_restriction_var_int_persistent(&type->restrictions->maxInclusive);
		delete_restriction_var_int_persistent(&type->restrictions->totalDigits);
		delete_restriction_var_int_persistent(&type->restrictions->fractionDigits);
		delete_restriction_var_int_persistent(&type->restrictions->length);
		delete_restriction_var_int_persistent(&type->restrictions->minLength);
		delete_restriction_var_int_persistent(&type->restrictions->maxLength);
		delete_restriction_var_char_persistent(&type->restrictions->whiteSpace);
		delete_restriction_var_char_persistent(&type->restrictions->pattern);
		if (type->restrictions->enumeration) {
			zend_hash_destroy(type->restrictions->enumeration);
			free(type->restrictions->enumeration);
		}
		free(type->restrictions);
	}
	free(type);
}

void delete_extra_attribute(void *attribute)
{
	sdlExtraAttributePtr attr = *((sdlExtraAttributePtr*)attribute);

	if (attr->ns) {
		efree(attr->ns);
	}
	if (attr->val) {
		efree(attr->val);
	}
	efree(attr);
}

void delete_extra_attribute_persistent(void *attribute)
{
	sdlExtraAttributePtr attr = *((sdlExtraAttributePtr*)attribute);

	if (attr->ns) {
		free(attr->ns);
	}
	if (attr->val) {
		free(attr->val);
	}
	free(attr);
}

void delete_attribute(void *attribute)
{
	sdlAttributePtr attr = *((sdlAttributePtr*)attribute);

	if (attr->def) {
		efree(attr->def);
	}
	if (attr->fixed) {
		efree(attr->fixed);
	}
	if (attr->name) {
		efree(attr->name);
	}
	if (attr->namens) {
		efree(attr->namens);
	}
	if (attr->ref) {
		efree(attr->ref);
	}
	if (attr->extraAttributes) {
		zend_hash_destroy(attr->extraAttributes);
		efree(attr->extraAttributes);
	}
	efree(attr);
}

void delete_attribute_persistent(void *attribute)
{
	sdlAttributePtr attr = *((sdlAttributePtr*)attribute);

	if (attr->def) {
		free(attr->def);
	}
	if (attr->fixed) {
		free(attr->fixed);
	}
	if (attr->name) {
		free(attr->name);
	}
	if (attr->namens) {
		free(attr->namens);
	}
	if (attr->ref) {
		free(attr->ref);
	}
	if (attr->extraAttributes) {
		zend_hash_destroy(attr->extraAttributes);
		free(attr->extraAttributes);
	}
	free(attr);
}

void delete_restriction_var_int(void *rvi)
{
	sdlRestrictionIntPtr ptr = *((sdlRestrictionIntPtr*)rvi);
	if (ptr) {
		efree(ptr);
	}
}

void delete_restriction_var_int_persistent(void *rvi)
{
	sdlRestrictionIntPtr ptr = *((sdlRestrictionIntPtr*)rvi);
	if (ptr) {
		free(ptr);
	}
}

void delete_restriction_var_char(void *srvc)
{
	sdlRestrictionCharPtr ptr = *((sdlRestrictionCharPtr*)srvc);
	if (ptr) {
		if (ptr->value) {
			efree(ptr->value);
		}
		efree(ptr);
	}
}

void delete_restriction_var_char_persistent(void *srvc)
{
	sdlRestrictionCharPtr ptr = *((sdlRestrictionCharPtr*)srvc);
	if (ptr) {
		if (ptr->value) {
			free(ptr->value);
		}
		free(ptr);
	}
}
