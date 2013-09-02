/***************************************************************************//**
 * FILE : graphicsfilter.hpp
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef CMZN_GRAPHICSFILTER_HPP__
#define CMZN_GRAPHICSFILTER_HPP__

#include "zinc/graphicsfilter.h"
#include "zinc/graphic.hpp"
#include "zinc/region.hpp"

namespace zinc
{

class GraphicsFilter
{
protected:
	cmzn_graphics_filter_id id;

public:

	GraphicsFilter() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFilter(cmzn_graphics_filter_id in_graphics_filter_id) :
		id(in_graphics_filter_id)
	{  }

	GraphicsFilter(const GraphicsFilter& graphicsFilter) :
		id(cmzn_graphics_filter_access(graphicsFilter.id))
	{  }

	GraphicsFilter& operator=(const GraphicsFilter& graphicsFilter)
	{
		cmzn_graphics_filter_id temp_id = cmzn_graphics_filter_access(graphicsFilter.id);
		if (0 != id)
		{
			cmzn_graphics_filter_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsFilter()
	{
		if (0 != id)
		{
			cmzn_graphics_filter_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_graphics_filter_id getId()
	{
		return id;
	}

	bool isManaged()
	{
		return cmzn_graphics_filter_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_graphics_filter_set_managed(id, value);
	}

	int evaluateGraphic(Graphic& graphic)
	{
		return cmzn_graphics_filter_evaluate_graphic(id, graphic.getId());
	}

	bool isInverse()
	{
		return cmzn_graphics_filter_is_inverse(id);
	}

	int setInverse(bool value)
	{
		return cmzn_graphics_filter_set_inverse(id, value);
	}

	char *getName()
	{
		return cmzn_graphics_filter_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_graphics_filter_set_name(id, name);
	}
};

class GraphicsFilterOperator : public GraphicsFilter
{
public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFilterOperator(cmzn_graphics_filter_operator_id graphics_filter_id) :
		GraphicsFilter(reinterpret_cast<cmzn_graphics_filter_id>(graphics_filter_id))
	{ }

	GraphicsFilterOperator(GraphicsFilter& graphicsFilter) :
		GraphicsFilter(reinterpret_cast<cmzn_graphics_filter_id>(
			cmzn_graphics_filter_cast_operator(graphicsFilter.getId())))
	{	}

	int appendOperand(GraphicsFilter& operand)
	{
		return cmzn_graphics_filter_operator_append_operand(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), operand.getId());
	}

	GraphicsFilter getFirstOperand()
	{
		return GraphicsFilter(cmzn_graphics_filter_operator_get_first_operand(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id)));
	}

	GraphicsFilter getNextOperand(GraphicsFilter& refOperand)
	{
		return GraphicsFilter(cmzn_graphics_filter_operator_get_next_operand(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), refOperand.getId()));
	}

	int getOperandIsActive(GraphicsFilter& operand)
	{
		return cmzn_graphics_filter_operator_get_operand_is_active(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), operand.getId());
	}

	int setOperandIsActive(GraphicsFilter& operand, int isActive)
	{
		return cmzn_graphics_filter_operator_set_operand_is_active(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), operand.getId(), isActive);
	}

	int insertOperandBefore(GraphicsFilter& operand, GraphicsFilter& refOperand)
	{
		return cmzn_graphics_filter_operator_insert_operand_before(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), operand.getId(), refOperand.getId());
	}

	int removeOperand(GraphicsFilter& operand)
	{
		return cmzn_graphics_filter_operator_remove_operand(
			reinterpret_cast<cmzn_graphics_filter_operator_id>(id), operand.getId());
	}
};

class GraphicsFilterModule
{
protected:
	cmzn_graphics_filter_module_id id;

public:

	GraphicsFilterModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFilterModule(cmzn_graphics_filter_module_id in_graphics_filter_module_id) :
		id(in_graphics_filter_module_id)
	{  }

	GraphicsFilterModule(const GraphicsFilterModule& graphicsFiltermodule) :
		id(cmzn_graphics_filter_module_access(graphicsFiltermodule.id))
	{  }

	GraphicsFilterModule& operator=(const GraphicsFilterModule& graphicsFiltermodule)
	{
		cmzn_graphics_filter_module_id temp_id = cmzn_graphics_filter_module_access(
			graphicsFiltermodule.id);
		if (0 != id)
		{
			cmzn_graphics_filter_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsFilterModule()
	{
		if (0 != id)
		{
			cmzn_graphics_filter_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_graphics_filter_module_id getId()
	{
		return id;
	}

	GraphicsFilter createFilterVisibilityFlags()
	{
		return GraphicsFilter(cmzn_graphics_filter_module_create_filter_visibility_flags(id));
	}

	GraphicsFilter createFilterDomainType(Field::DomainType domainType)
	{
		return GraphicsFilter(cmzn_graphics_filter_module_create_filter_domain_type(id,
			static_cast<cmzn_field_domain_type>(domainType)));
	}

	GraphicsFilter createFilterGraphicName(const char *matchName)
	{
		return GraphicsFilter(cmzn_graphics_filter_module_create_filter_graphic_name(id, matchName));
	}

	GraphicsFilter createFilterGraphicType(Graphic::Type graphicType)
	{
		return GraphicsFilter(cmzn_graphics_filter_module_create_filter_graphic_type(id,
			static_cast<cmzn_graphic_type>(graphicType)));
	}

	GraphicsFilter createFilterRegion(Region& matchRegion)
	{
		return GraphicsFilter(cmzn_graphics_filter_module_create_filter_region(
			id, matchRegion.getId()));
	}

	GraphicsFilterOperator createFilterOperatorAnd()
	{
		return GraphicsFilterOperator(reinterpret_cast<cmzn_graphics_filter_operator_id>(
			cmzn_graphics_filter_module_create_filter_operator_and(id)));
	}

	GraphicsFilterOperator createFilterOperatorOr()
	{
		return GraphicsFilterOperator(reinterpret_cast<cmzn_graphics_filter_operator_id>(
			cmzn_graphics_filter_module_create_filter_operator_or(id)));
	}

	GraphicsFilter findFilterByName(const char *name)
	{
		return GraphicsFilter(cmzn_graphics_filter_module_find_filter_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_graphics_filter_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_graphics_filter_module_end_change(id);
	}

	GraphicsFilter getDefaultFilter()
	{
		return GraphicsFilter(cmzn_graphics_filter_module_get_default_filter(id));
	}

	int setDefaultFilter(GraphicsFilter &filter)
	{
		return cmzn_graphics_filter_module_set_default_filter(id, filter.getId());
	}
};

}  // namespace zinc

#endif
