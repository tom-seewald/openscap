/**
 * @file cpelang_priv.h
 * \brief Interface to Common Platform Enumeration (CPE) Language.
 *
 * See more details at http://nvd.nist.gov/cpe.cfm
 */

/*
 * Copyright 2009 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors:
 *      Maros Barabas <mbarabas@redhat.com>
 */

#ifndef CPELANG_PRIV_H_
#define CPELANG_PRIV_H_

#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include "cpe_lang.h"
#include "../common/util.h"
#include "../common/list.h"
#include "../common/elements.h"
#include "../common/public/oscap.h"

/**
 * @cond INTERNAL
 */
 /* @endcond */

/**
 * @struct cpe_testexpr
 * CPE language boolean expression
 */
struct cpe_testexpr {
	cpe_lang_oper_t oper;	///< operator
	union {
		struct oscap_list *expr;	///< array of subexpressions for operators
		struct cpe_name *cpe;	///< CPE for match operation
		struct {
			char* system;
			char* href;
			char* id;
		} check;
	} meta;			///< operation metadata
};

/**
 * @struct cpe_lang_model
 * CPE platform specification
 */
struct cpe_lang_model;

/**
 * @struct cpe_platform
 * Single platform representation in CPE language
 */
struct cpe_platform;

/**
 * Parse function for CPE Lang model
 * @param reader xmlTextReaderPtr structure representing XML model
 */
struct cpe_lang_model *cpe_lang_model_parse(xmlTextReaderPtr reader);

/**
 * Parse CPE platform structure
 * @param reader xmlTextReaderPtr structure representing XML model
 * @return cpe_platform structure with CPE platform item
 */
struct cpe_platform *cpe_platform_parse(xmlTextReaderPtr reader);

/**
 * Parse CPE test expression structure
 * @param reader xmlTextReaderPtr structure representing XML model
 * @return cpe_testexpr structure with CPE test expression item
 */
struct cpe_testexpr *cpe_testexpr_parse(xmlTextReaderPtr reader);

/**
 * Function for export CPE language model to XML
 * @param spec CPE language model structure
 * @param file filename
 */
void cpe_lang_model_export_xml(const struct cpe_lang_model *spec, const char *file);

/**
 * Function for export CPE language top element
 * @param writer xmlTextWriterPtr structure representing XML model
 * @param spec CPE language model structure
 */
void cpe_lang_export(const struct cpe_lang_model *spec, xmlTextWriterPtr writer);

/**
 * Function for export CPE platform element
 * @param writer xmlTextWriterPtr structure representing XML model
 * @param platform CPE platform structure
 */
void cpe_platform_export(const struct cpe_platform *platform, xmlTextWriterPtr writer);

/**
 * Function for export CPE test expression element
 * @param writer xmlTextWriterPtr structure representing XML model
 * @param expr CPE test expression structure
 */
void cpe_testexpr_export(const struct cpe_testexpr *expr, xmlTextWriterPtr writer);

char *cpe_lang_model_detect_version_priv(xmlTextReader *reader);

/**
 * Sets the origin file hint
 * @see cpe_lang_model_get_origin_file
 */
bool cpe_lang_model_set_origin_file(struct cpe_lang_model* lang_model, const char* origin_file);

/**
 * Gets the file the CPE dict model was loaded from
 * This is necessary to figure out the full OVAL file path for applicability
 * testing. We can't do applicability here in the CPE module because that
 * would create awful interdependencies.
 */
const char* cpe_lang_model_get_origin_file(const struct cpe_lang_model* lang_model);


/** 
 * @cond INTERNAL
 */
 /* @endcond */

#endif
