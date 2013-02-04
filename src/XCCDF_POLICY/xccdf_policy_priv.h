/*
 * Copyright 2013 Red Hat Inc., Durham, North Carolina.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *   Simon Lukasik <slukasik@redhat.com>
 *
 */

#pragma once
#ifndef _OSCAP_XCCDF_POLICY_PRIV_H
#define _OSCAP_XCCDF_POLICY_PRIV_H

#include "common/util.h"
#include "public/xccdf_policy.h"

OSCAP_HIDDEN_START;

/**
 * Resolve text substitution in given fix element. Use given xccdf_policy settings
 * for resolving.
 * @memberof xccdf_policy
 * @param policy XCCDF policy used for substitution
 * @param fix a fix element to modify
 * @param test_resut the TestResult for xccdf:fact resolution
 * @returns 0 on success, 1 on failure, other value indicate warning
 */
int xccdf_policy_resolve_fix_substitution(struct xccdf_policy *policy, struct xccdf_fix *fix, struct xccdf_result *test_result);

/**
 * Get value of given value item in context of given policy
 * @memberof xccdf_policy
 * @param policy XCCDF policy
 * @param item the xccdf:Value to resolve
 * @returns string representation of resolved value_instance.
 * @retval NULL indicates failure
 */
const char * xccdf_policy_get_value_of_item(struct xccdf_policy * policy, struct xccdf_item * item);

OSCAP_HIDDEN_END;

#endif