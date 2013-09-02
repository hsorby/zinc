/***************************************************************************//**
 * FILE : spectrum.hpp
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
#ifndef CMZN_SPECTRUM_HPP__
#define CMZN_SPECTRUM_HPP__

#include "zinc/spectrum.h"

namespace zinc
{

class SpectrumComponent
{
protected:
	cmzn_spectrum_component_id id;

public:

	SpectrumComponent() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit SpectrumComponent(
		cmzn_spectrum_component_id in_spectrum_component_id) :
		id(in_spectrum_component_id)
	{
	}

	SpectrumComponent(const SpectrumComponent& spectrumComponent) :
		id(cmzn_spectrum_component_access(spectrumComponent.id))
	{
	}

	SpectrumComponent& operator=(const SpectrumComponent& spectrumComponent)
	{
		cmzn_spectrum_component_id temp_id = cmzn_spectrum_component_access(
			spectrumComponent.id);
		if (0 != id)
		{
			cmzn_spectrum_component_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SpectrumComponent()
	{
		if (0 != id)
		{
			cmzn_spectrum_component_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum ScaleType
	{
		SCALE_INVALID = CMISS_SPECTRUM_COMPONENT_SCALE_INVALID,
		SCALE_LINEAR = CMISS_SPECTRUM_COMPONENT_SCALE_LINEAR,
		SCALE_LOG = CMISS_SPECTRUM_COMPONENT_SCALE_LOG
	};

	enum ColourMapping
	{
		COLOUR_MAPPING_INVALID = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_INVALID,
		COLOUR_MAPPING_ALPHA = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_ALPHA,
		COLOUR_MAPPING_BANDED = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BANDED,
		COLOUR_MAPPING_BLUE = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BLUE,
		COLOUR_MAPPING_GREEN = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_GREEN,
		COLOUR_MAPPING_MONOCHROME = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME,
		COLOUR_MAPPING_RAINBOW = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW,
		COLOUR_MAPPING_RED = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RED,
		COLOUR_MAPPING_STEP = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_STEP,
		COLOUR_MAPPING_WHITE_TO_BLUE = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_BLUE,
		COLOUR_MAPPING_WHITE_TO_RED = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_RED,
		COLOUR_MAPPING_WHITE_TO_GREEN = CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_GREEN
	};

	cmzn_spectrum_component_id getId()
	{
		return id;
	}

	double getRangeMinumum()
	{
		return cmzn_spectrum_component_get_range_minimum(id);
	}

	int setRangeMinimum(double value)
	{
		return cmzn_spectrum_component_set_range_minimum(id, value);
	}

	double getRangeMaximum()
	{
		return cmzn_spectrum_component_get_range_maximum(id);
	}

	int setRangeMaximum(double value)
	{
		return cmzn_spectrum_component_set_range_maximum(id, value);
	}

	double getColourMinumum()
	{
		return cmzn_spectrum_component_get_colour_minimum(id);
	}

	int setColourMinimum(double value)
	{
		return cmzn_spectrum_component_set_colour_minimum(id,value);
	}

	double getColourMaxumum()
	{
		return cmzn_spectrum_component_get_colour_maximum(id);
	}

	int setColourMaximum(double value)
	{
		return cmzn_spectrum_component_set_colour_maximum(id, value);
	}

	double getStepValue()
	{
		return cmzn_spectrum_component_get_step_value(id);
	}

	int setStepValue(double value)
	{
		return cmzn_spectrum_component_set_step_value(id, value);
	}

	double getExaggeration()
	{
		return cmzn_spectrum_component_get_exaggeration(id);
	}

	int setExaggeration(double value)
	{
		return cmzn_spectrum_component_set_exaggeration(id, value);
	}

	double getBandedRatio()
	{
		return cmzn_spectrum_component_get_banded_ratio(id);
	}

	int setBandedRatio(double value)
	{
		return cmzn_spectrum_component_set_banded_ratio(id, value);
	}

	bool isActive()
	{
		return cmzn_spectrum_component_is_active(id);
	}

	int setActive(bool active)
	{
		return cmzn_spectrum_component_set_active(id, active);
	}

	bool isColourReverse()
	{
		return cmzn_spectrum_component_is_colour_reverse(id);
	}

	int setColourReverse(bool reverse)
	{
		return cmzn_spectrum_component_set_colour_reverse(id, reverse);
	}

	bool isExtendAbove()
	{
		return cmzn_spectrum_component_is_extend_above(id);
	}

	int setExtendAbove(bool extend_above)
	{
		return cmzn_spectrum_component_set_extend_above(id, extend_above);
	}

	bool isExtendBelow()
	{
		return cmzn_spectrum_component_is_extend_below(id);
	}

	int setExtendBelow(bool extend_below)
	{
		return cmzn_spectrum_component_set_extend_below(id, extend_below);
	}

	int getFieldComponent()
	{
		return cmzn_spectrum_component_get_field_component(id);
	}

	int setFieldComponent(int componentNumber)
	{
		return cmzn_spectrum_component_set_field_component(id,
			componentNumber);
	}

	int getNumberOfBands()
	{
		return cmzn_spectrum_component_get_number_of_bands(id);
	}

	int setNumberOfBands(int numberOfBands)
	{
		return cmzn_spectrum_component_set_number_of_bands(id, numberOfBands);
	}

	ScaleType getScaleType()
	{
		return static_cast<ScaleType>(cmzn_spectrum_component_get_scale_type(
			id));
	}

	int setScaleType(ScaleType scaleType)
	{
		return cmzn_spectrum_component_set_scale_type(id,
			static_cast<cmzn_spectrum_component_scale_type>(scaleType));
	}

	ColourMapping getColourMapping()
	{
		return static_cast<ColourMapping>(cmzn_spectrum_component_get_colour_mapping(
			id));
	}

	int setColourMapping(ColourMapping colourMapping)
	{
		return cmzn_spectrum_component_set_colour_mapping(id,
			static_cast<cmzn_spectrum_component_colour_mapping>(colourMapping));
	}

};

class Spectrum
{
protected:
	cmzn_spectrum_id id;

public:

	Spectrum() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit Spectrum(cmzn_spectrum_id in_spectrum_id) :
		id(in_spectrum_id)
	{
	}

	Spectrum(const Spectrum& spectrum) :
		id(cmzn_spectrum_access(spectrum.id))
	{
	}

	Spectrum& operator=(const Spectrum& spectrum)
	{
		cmzn_spectrum_id temp_id = cmzn_spectrum_access(spectrum.id);
		if (0 != id)
		{
			cmzn_spectrum_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Spectrum()
	{
		if (0 != id)
		{
			cmzn_spectrum_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_spectrum_id getId()
	{
		return id;
	}

	bool isManaged()
	{
		return cmzn_spectrum_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_spectrum_set_managed(id, value);
	}

	int beginChange()
	{
		return cmzn_spectrum_begin_change(id);
	}

	int endChange()
	{
		return cmzn_spectrum_end_change(id);
	}

	char *getName()
	{
		return cmzn_spectrum_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_spectrum_set_name(id, name);
	}

	int getNumberOfComponents()
	{
		return cmzn_spectrum_get_number_of_components(id);
	}

	SpectrumComponent createComponent()
	{
		return SpectrumComponent(cmzn_spectrum_create_component(id));
	}

	SpectrumComponent getFirstComponent()
	{
		return SpectrumComponent(cmzn_spectrum_get_first_component(id));
	}

	SpectrumComponent getNextComponent(SpectrumComponent& refComponent)
	{
		return SpectrumComponent(
			cmzn_spectrum_get_next_component(id, refComponent.getId()));
	}

	SpectrumComponent getPreviousComponent(SpectrumComponent& refComponent)
	{
		return SpectrumComponent(
			cmzn_spectrum_get_previous_component(id, refComponent.getId()));
	}

	int moveComponentBefore(SpectrumComponent& component,
		SpectrumComponent& refComponent)
	{
		return cmzn_spectrum_move_component_before(id, component.getId(),
			refComponent.getId());
	}

	int removeComponent(SpectrumComponent component)
	{
		return cmzn_spectrum_remove_component(id, component.getId());
	}

	int removeAllComponents()
	{
		return cmzn_spectrum_remove_all_components(id);
	}

	bool isMaterialOverwrite()
	{
		return cmzn_spectrum_is_material_overwrite(id);
	}

	int setMaterialOverwrite(bool overwrite)
	{
		return cmzn_spectrum_set_material_overwrite(id, overwrite);
	}

};

class SpectrumModule
{
protected:
	cmzn_spectrum_module_id id;

public:

	SpectrumModule() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit SpectrumModule(cmzn_spectrum_module_id in_spectrum_module_id) :
		id(in_spectrum_module_id)
	{
	}

	SpectrumModule(const SpectrumModule& spectrumModule) :
		id(cmzn_spectrum_module_access(spectrumModule.id))
	{
	}

	SpectrumModule& operator=(const SpectrumModule& spectrumModule)
	{
		cmzn_spectrum_module_id temp_id = cmzn_spectrum_module_access(
			spectrumModule.id);
		if (0 != id)
		{
			cmzn_spectrum_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~SpectrumModule()
	{
		if (0 != id)
		{
			cmzn_spectrum_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_spectrum_module_id getId()
	{
		return id;
	}

	Spectrum createSpectrum()
	{
		return Spectrum(cmzn_spectrum_module_create_spectrum(id));
	}

	Spectrum findSpectrumByName(const char *name)
	{
		return Spectrum(cmzn_spectrum_module_find_spectrum_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_spectrum_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_spectrum_module_end_change(id);
	}

	Spectrum getDefaultSpectrum()
	{
		return Spectrum(cmzn_spectrum_module_get_default_spectrum(id));
	}

	int setDefaultSpectrum(Spectrum &spectrum)
	{
		return cmzn_spectrum_module_set_default_spectrum(id, spectrum.getId());
	}
};

} // namespace zinc

#endif
