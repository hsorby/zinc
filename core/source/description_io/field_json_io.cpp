/***************************************************************************//**
 * FILE : graphics_json_io.cpp
 *
 * The definition to graphics_json_io.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "description_io/field_json_io.hpp"
#include "general/debug.h"
#include "opencmiss/zinc/fieldalias.hpp"
#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldcomposite.hpp"
#include "opencmiss/zinc/fieldconditional.hpp"
#include "opencmiss/zinc/fieldconstant.hpp"
#include "opencmiss/zinc/fieldcoordinatetransformation.hpp"
#include "opencmiss/zinc/fieldderivatives.hpp"
#include "opencmiss/zinc/fieldfibres.hpp"
#include "opencmiss/zinc/fieldfiniteelement.hpp"
#include "opencmiss/zinc/fieldlogicaloperators.hpp"
#include "opencmiss/zinc/fieldmatrixoperators.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.hpp"

#include <string.h>

/*
 * header not yet supported:
 * fieldgroup
 * fieldimage
 * fieldimageprocessing
 *
 *
 */

OpenCMISS::Zinc::Field *getSourceFields(Json::Value &fieldSettings, int *count,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int numberOfSourceFields = 0;
	if (fieldSettings["NumberOfSourceFields"].isInt())
	{
		numberOfSourceFields = fieldSettings["NumberOfSourceFields"].asInt();
	}

	OpenCMISS::Zinc::Field *sourceFields = 0;

	if (numberOfSourceFields > 0)
	{
		sourceFields = new OpenCMISS::Zinc::Field[numberOfSourceFields];
		if (fieldSettings["SourceField"].isArray() &&
			fieldSettings["SourceField"].size() == numberOfSourceFields)
		{
			for (unsigned int i = 0; i < fieldSettings["SourceField"].size(); i++)
			{
				const char *sourceFieldName = fieldSettings["SourceField"][i].asCString();
				sourceFields[i] = jsonImport->getFieldByName(sourceFieldName);
			}
		}
	}
	*count = numberOfSourceFields;

	return sourceFields;
}

OpenCMISS::Zinc::Field importGenericOneComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 1)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_ALIAS:
				field = fieldmodule.createFieldAlias(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_LOG:
				field = fieldmodule.createFieldLog(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_SQRT:
				field = fieldmodule.createFieldSqrt(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EXP:
				field = fieldmodule.createFieldExp(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_ABS:
				field = fieldmodule.createFieldAbs(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_COORDINATE_TRANFORMATION:
				field = fieldmodule.createFieldCoordinateTransformation(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_IS_DEFINED:
				field = fieldmodule.createFieldIsDefined(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_NOT:
				field = fieldmodule.createFieldNot(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_DETERMINANT:
				field = fieldmodule.createFieldDeterminant(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EIGENVALUES:
				field = fieldmodule.createFieldEigenvalues(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EIGENVECTORS:
			{
				field = OpenCMISS::Zinc::Field(cmzn_fieldmodule_create_field_eigenvectors(
					fieldmodule.getId(),	sourcefields[0].getId()));
			}	break;
			case CMZN_FIELD_TYPE_MATRIX_INVERT:
				field = fieldmodule.createFieldMatrixInvert(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_TRANSPOSE:
			{
				if (fieldSettings["FieldTranspose"]["SourceNumberOfRows"].isInt())
				{
					field = fieldmodule.createFieldTranspose(
						fieldSettings["FieldTranspose"]["SourceNumberOfRows"].asInt(),
						sourcefields[0]);
				}
			}	break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importGenericTwoComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 2)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_ADD:
				field = fieldmodule.createFieldAdd(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_POWER:
				field = fieldmodule.createFieldPower(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_MULTIPLY:
				field = fieldmodule.createFieldMultiply(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_DIVIDE:
				field = fieldmodule.createFieldDivide(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_SUBTRACT:
				field = fieldmodule.createFieldSubtract(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANFORMATION:
				field = fieldmodule.createFieldVectorCoordinateTransformation(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_CURL:
				field = fieldmodule.createFieldCurl(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_DIVERGENCE:
				field = fieldmodule.createFieldDivergence(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_GRADIENT:
				field = fieldmodule.createFieldGradient(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_FIBRE_AXES:
				field = fieldmodule.createFieldFibreAxes(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_EMBEDDED:
				field = fieldmodule.createFieldEmbedded(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_AND:
				field = fieldmodule.createFieldAnd(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_EQUAL_TO:
				field = fieldmodule.createFieldEqualTo(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_GREATER_THAN:
				field = fieldmodule.createFieldGreaterThan(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_LESS_THAN:
				field = fieldmodule.createFieldLessThan(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_OR:
				field = fieldmodule.createFieldOr(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_XOR:
				field = fieldmodule.createFieldXor(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_PROJECTION:
				field = fieldmodule.createFieldProjection(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
			{
				if (fieldSettings["FieldMatrixMultiply"]["NumberOfRows"].isInt())
				{
					field = fieldmodule.createFieldMatrixMultiply(
						fieldSettings["FieldMatrixMultiply"]["NumberOfRows"].asInt(),
						sourcefields[0], sourcefields[1]);
				}
			}	break;
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importGenericThreeComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 3)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_IF:
				field = fieldmodule.createFieldIf(sourcefields[0], sourcefields[1], sourcefields[3]);
				break;
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importCompositeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount > 0)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_COMPONENT:
				if (fieldSettings["FieldComponent"].isObject())
				{
					Json::Value typeSettings = fieldSettings["FieldComponent"];
					if (typeSettings["SourceComponentIndex"].isArray())
					{
						int componentCount = typeSettings["SourceComponentIndex"].size();
						if (componentCount == 1)
							field = fieldmodule.createFieldComponent(
								sourcefields[0], typeSettings["SourceComponentIndex"][0].asInt());
						if (componentCount > 1)
						{
							int *indexes;
							indexes = new int[componentCount];
							for (int i = 0; i < componentCount; i++)
								indexes[i] = typeSettings["SourceComponentIndex"][i].asInt();
							field = fieldmodule.createFieldComponent(sourcefields[0],
								componentCount, indexes);
							delete[] indexes;
						}
					}
				}	break;
			case CMZN_FIELD_TYPE_CONCATENATE:
				field = fieldmodule.createFieldConcatenate(sourcesCount, sourcefields);
				break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importConstantField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings)
{
	OpenCMISS::Zinc::Field field(0);

	switch (type)
	{
		case CMZN_FIELD_TYPE_CONSTANT:
		{
			if (fieldSettings["FieldConstant"]["values"].isArray())
			{
				unsigned int count = fieldSettings["FieldConstant"]["values"].size();
				double *values;
				values = new double[count];
				for (unsigned int i = 0; i < count; i++)
					values[i] = fieldSettings["FieldConstant"]["values"][i].asDouble();
				field = fieldmodule.createFieldConstant(count, values);
				delete[] values;
			}
		} break;
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
		{
			if (fieldSettings["FieldStringConstant"]["stringValue"].isString())
				field = fieldmodule.createFieldStringConstant(
					fieldSettings["FieldStringConstant"]["stringValue"].asCString());
		} break;
		default:
			break;
	}
	return field;
}

OpenCMISS::Zinc::Field importDerivativeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 1)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_DERIVATIVE:
			{
				if (fieldSettings["FieldDerivative"]["xiIndex"].isInt())
				{

					field = fieldmodule.createFieldDerivative(sourcefields[0],
						fieldSettings["FieldDerivative"]["xiIndex"].asInt());
				}
			} break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importFiniteElementField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);
	switch (type)
	{
		case CMZN_FIELD_TYPE_EMBEDDED:
			field = importGenericTwoComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_STORED_STRING:
			field = fieldmodule.createFieldStoredString();
			break;
		case CMZN_FIELD_TYPE_IS_EXTERIOR:
			field = fieldmodule.createFieldIsExterior();
			break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			if (fieldSettings["FieldIsOnFace"]["ElementFaceType"].isString())
			{
				enum cmzn_element_face_type type = cmzn_element_face_type_enum_from_string(
					fieldSettings["FieldIsOnFace"]["ElementFaceType"].asCString());
				field = OpenCMISS::Zinc::Field(cmzn_fieldmodule_create_field_is_on_face(
					fieldmodule.getId(), type));
			}
		}	break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount > 0)
			{
				OpenCMISS::Zinc::FieldEdgeDiscontinuity derivedField = fieldmodule.createFieldEdgeDiscontinuity(
					sourcefields[0]);
				if (sourcesCount == 2)
				{
					derivedField.setConditionalField(sourcefields[1]);
				}
				if (fieldSettings["FieldEdgeDiscontinuity"]["Measure"].isString())
					cmzn_field_edge_discontinuity_set_measure(derivedField.getDerivedId(),
						cmzn_field_edge_discontinuity_measure_enum_from_string(
							fieldSettings["FieldEdgeDiscontinuity"]["Measure"].asCString()));
				field = derivedField;
			}
			delete[] sourcefields;
		}	break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount == 1 && fieldSettings["FieldNodeValue"]["NodeValueLabel"].isString())
			{
				enum cmzn_node_value_label valueLabel = cmzn_node_value_label_enum_from_string(
					fieldSettings["FieldNodeValue"]["NodeValueLabel"].asCString());
				int version = fieldSettings["FieldNodeValue"]["VersionNumber"].isInt();
				field = OpenCMISS::Zinc::Field(
					cmzn_fieldmodule_create_field_node_value(fieldmodule.getId(),
						sourcefields[0].getId(), valueLabel, version));
			}
			delete[] sourcefields;
		}	break;
		default:
			break;

	}
	return field;
}

OpenCMISS::Zinc::Field importTypeSpecificField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);

	switch (type)
	{
		case CMZN_FIELD_TYPE_ALIAS:
		case CMZN_FIELD_TYPE_LOG:
		case CMZN_FIELD_TYPE_SQRT:
		case CMZN_FIELD_TYPE_EXP:
		case CMZN_FIELD_TYPE_ABS:
		case CMZN_FIELD_TYPE_IDENTITY:
		case CMZN_FIELD_TYPE_COORDINATE_TRANFORMATION:
		case CMZN_FIELD_TYPE_IS_DEFINED:
		case CMZN_FIELD_TYPE_NOT:
		case CMZN_FIELD_TYPE_DETERMINANT:
		case CMZN_FIELD_TYPE_EIGENVALUES:
		case CMZN_FIELD_TYPE_EIGENVECTORS:
		case CMZN_FIELD_TYPE_MATRIX_INVERT:
			field = importGenericOneComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_ADD:
		case CMZN_FIELD_TYPE_POWER:
		case CMZN_FIELD_TYPE_MULTIPLY:
		case CMZN_FIELD_TYPE_DIVIDE:
		case CMZN_FIELD_TYPE_SUBTRACT:
		case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANFORMATION:
		case CMZN_FIELD_TYPE_CURL:
		case CMZN_FIELD_TYPE_DIVERGENCE:
		case CMZN_FIELD_TYPE_GRADIENT:
		case CMZN_FIELD_TYPE_FIBRE_AXES:
		case CMZN_FIELD_TYPE_AND:
		case CMZN_FIELD_TYPE_EQUAL_TO:
		case CMZN_FIELD_TYPE_GREATER_THAN:
		case CMZN_FIELD_TYPE_LESS_THAN:
		case CMZN_FIELD_TYPE_OR:
		case CMZN_FIELD_TYPE_XOR:
		case CMZN_FIELD_TYPE_PROJECTION:
		case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
			field = importGenericTwoComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		case CMZN_FIELD_TYPE_CONCATENATE:
			field = importCompositeField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_IF:
			field = importGenericThreeComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_CONSTANT:
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
			field = importConstantField(type, fieldmodule, fieldSettings);
			break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
			field = importDerivativeField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_EMBEDDED:
		case CMZN_FIELD_TYPE_STORED_STRING:
		case CMZN_FIELD_TYPE_IS_EXTERIOR:
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		case CMZN_FIELD_TYPE_NODE_VALUE:
			field = importFiniteElementField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		default:
			break;
	}

	return field;
}


void FieldJsonIO::exportTypeSpecificParameters(Json::Value &fieldSettings)
{
	enum cmzn_field_type type = cmzn_field_get_type(field.getId());
	char *type_string = cmzn_field_type_enum_to_string(cmzn_field_get_type(field.getId()));
	fieldSettings["Type"] = type_string;
	DEALLOCATE(type_string);
	int numberOfComponent = field.getNumberOfComponents();
	switch (type)
	{
		case CMZN_FIELD_TYPE_COMPONENT:
		{
			OpenCMISS::Zinc::FieldComponent fieldComponent = field.castComponent();
			Json::Value typeSettings;
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["SourceComponentIndex"].append(fieldComponent.getSourceComponentIndex(i+1));
			}
			fieldSettings["FieldComponent"] = typeSettings;
		} break;
		case CMZN_FIELD_TYPE_CONSTANT:
		{
			Json::Value typeSettings;
			OpenCMISS::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
			double *values = 0;
			values = new double[numberOfComponent];
			field.evaluateReal(fieldcache, numberOfComponent, values);
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["values"].append(values[i]);
			}
			delete[] values;
			fieldSettings["FieldConstant"] = typeSettings;
		} break;
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
		{
			Json::Value typeSettings;
			OpenCMISS::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
			char *string_value = field.evaluateString(fieldcache);
			typeSettings["stringValue"] = string_value;
			fieldSettings["FieldStringConstant"] = typeSettings;
			DEALLOCATE(string_value);
		} break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
		{
			Json::Value typeSettings;
			int xi_index = cmzn_field_derivative_get_xi_index(field.getId());
			typeSettings["xiIndex"] = xi_index;
			fieldSettings["FieldDerivative"] = typeSettings;
		} break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			Json::Value typeSettings;
			char *enumString = cmzn_element_face_type_enum_to_string(
				cmzn_field_is_on_face_get_face_type(field.getId()));
			if (enumString)
			{
				typeSettings["ElementFaceType"] = enumString;
				DEALLOCATE(enumString);
			}
			fieldSettings["FieldIsOnFace"] = typeSettings;
		}	break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			Json::Value typeSettings;
			char *enumString = cmzn_field_edge_discontinuity_measure_enum_to_string(
				cmzn_field_edge_discontinuity_get_measure(field.castEdgeDiscontinuity().getDerivedId()));
			if (enumString)
			{
				typeSettings["Measure"] = enumString;
				DEALLOCATE(enumString);
			}
			fieldSettings["FieldEdgeDiscontinuity"] = typeSettings;
		}	break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			Json::Value typeSettings;
			char *enumString = cmzn_node_value_label_enum_to_string(
				cmzn_field_node_value_get_value_label(field.getId()));
			if (enumString)
			{
				typeSettings["NodeValueLabel"] = enumString;
				DEALLOCATE(enumString);
			}
			int versioNumber = cmzn_field_node_value_get_version(field.getId());
			typeSettings["VersionNumber"] = versioNumber;
			fieldSettings["FieldNodeValue"] = typeSettings;
		}	break;
		case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
		{
			Json::Value typeSettings;
			int numberOfRows = cmzn_field_matrix_multiply_get_number_of_rows(field.getId());
			typeSettings["NumberOfRows"] = numberOfRows;
			fieldSettings["FieldMatrixMultiply"] = typeSettings;
		}	break;
		case CMZN_FIELD_TYPE_TRANSPOSE:
		{
			Json::Value typeSettings;
			int sourceNumberOfRows = cmzn_field_transpose_get_source_number_of_rowss(field.getId());
			typeSettings["SourceNumberOfRows"] = sourceNumberOfRows;
			fieldSettings["FieldTranspose"] = typeSettings;
		}	break;
		default:
			break;
	}
}

void FieldJsonIO::ioFiniteElementOnlyEntries(Json::Value &fieldSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		int numberOfComponents = field.getNumberOfComponents();
		fieldSettings["TypeCoordinate"] = field.isTypeCoordinate();
		fieldSettings["NumberOfComponents"] = numberOfComponents;
		for (int i = 0; i < numberOfComponents; i++)
		{
			char *name = field.getComponentName(1 + i);
			fieldSettings["ComponentName"].append(name);
			DEALLOCATE(name);
		}
	}
	else
	{
		if (fieldSettings["TypeCoordinate"].isBool())
			field.setTypeCoordinate(fieldSettings["TypeCoordinate"].asBool());
		if (fieldSettings["ComponentName"].isArray())
		{
			unsigned int numberOfComponents = fieldSettings["ComponentName"].size();
			for (unsigned int i = 0; i < numberOfComponents; i++)
			{
				field.setComponentName(i+1, fieldSettings["ComponentName"][i].asCString());
			}
		}
	}
}

void FieldJsonIO::ioEntries(Json::Value &fieldSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		char *name = field.getName();
		fieldSettings["Name"] = name;
		DEALLOCATE(name);

		fieldSettings["IsManaged"] = field.isManaged();
		char *system_string = cmzn_field_coordinate_system_type_enum_to_string(
			cmzn_field_get_coordinate_system_type(field.getId()));
		fieldSettings["CoordinateSystemType"] = system_string;
		fieldSettings["CoordinateSystemFocus"] = field.getCoordinateSystemFocus();
		DEALLOCATE(system_string);
		int numberOfSourceFields = field.getNumberOfSourceFields();
		fieldSettings["NumberOfSourceFields"] = numberOfSourceFields;
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			OpenCMISS::Zinc::Field sourceField = field.getSourceField(1 + i);
			char *sourceName = sourceField.getName();
			fieldSettings["SourceField"].append(sourceName);
			DEALLOCATE(sourceName);
		}
		exportTypeSpecificParameters(fieldSettings);
	}
	else
	{
		if (fieldSettings["Name"].isString())
		{
			field.setName(fieldSettings["Name"].asCString());
		}
		if (fieldSettings["CoordinateSystemType"].isString())
			cmzn_field_set_coordinate_system_type(field.getId(),
				cmzn_field_coordinate_system_type_enum_from_string(
					fieldSettings["CoordinateSystemType"].asCString()));
		if (fieldSettings["CoordinateSystemFocus"].isDouble())
			field.setCoordinateSystemFocus(fieldSettings["CoordinateSystemFocus"].asDouble());

		field.setManaged(true);
	}
}
