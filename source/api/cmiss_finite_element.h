/*******************************************************************************
FILE : cmiss_finite_element.h

LAST MODIFIED : 8 November 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
#ifndef __CMISS_FINITE_ELEMENT_H__
#define __CMISS_FINITE_ELEMENT_H__

#include "api/cmiss_region.h"

/*
Global types
------------
*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_element FE_element

struct Cmiss_element;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node FE_node

struct Cmiss_node;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

/* Put temporarily into api/cmiss_region.h until we resolve the circularness,
	maybe we need a types declaration file that we can include from everywhere
typedef int (*Cmiss_node_iterator_function)(struct Cmiss_node *node, void *user_data); */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_nodal_value_type FE_nodal_value_type

enum FE_nodal_value_type
/*******************************************************************************
LAST MODIFIED : 27 January 1998

DESCRIPTION :
The type of a nodal value.
Must add new enumerators and keep values in sync with functions
ENUMERATOR_STRING, ENUMERATOR_GET_VALID_STRINGS and STRING_TO_ENUMERATOR.
Note these functions expect the first enumerator to be number 1, and all
subsequent enumerators to be sequential, unlike the default behaviour which
starts at 0.
==============================================================================*/
{
	FE_NODAL_VALUE,
	FE_NODAL_D_DS1,
	FE_NODAL_D_DS2,
	FE_NODAL_D_DS3,
	FE_NODAL_D2_DS1DS2,
	FE_NODAL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3,
	FE_NODAL_UNKNOWN
}; /* enum FE_nodal_value_type */

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node_field_creator FE_node_field_creator

struct Cmiss_node_field_creator;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_node_field_creator * Cmiss_node_field_creator_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_time_version FE_time_version

struct FE_time_version;

typedef struct Cmiss_time_version * Cmiss_time_version_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_basis_type FE_basis_type

enum FE_basis_type
/*******************************************************************************
LAST MODIFIED : 20 October 1997

DESCRIPTION :
The different basis types available.
NOTE: Must keep this up to date with functions
FE_basis_type_string
==============================================================================*/
{
	FE_BASIS_TYPE_INVALID=-1,
	NO_RELATION=0,
		/*???DB.  Used on the off-diagonals of the type matrix */
	BSPLINE,
	CUBIC_HERMITE,
	CUBIC_LAGRANGE,
	FOURIER,
	HERMITE_LAGRANGE,
	LAGRANGE_HERMITE,
	LINEAR_LAGRANGE,
	LINEAR_SIMPLEX,
	POLYGON,
	QUADRATIC_LAGRANGE,
	QUADRATIC_SIMPLEX,
	SERENDIPITY,
	SINGULAR,
	TRANSITION
}; /* enum FE_basis_type */

/*
Global functions
----------------
*/

int Cmiss_node_get_identifier(Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/

struct Cmiss_node_field_creator *CREATE(Cmiss_node_field_creator)(
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
An object for defining the components, number_of_versions,
number_of_derivatives and their types at a node.
By default each component has 1 version and no derivatives.
==============================================================================*/

int DESTROY(Cmiss_node_field_creator)(
	struct Cmiss_node_field_creator **node_field_creator_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
Frees the memory for the node field creator and sets 
<*node_field_creator_address> to NULL.
==============================================================================*/

#define Cmiss_node_field_creator_define_derivative FE_node_field_creator_define_derivative

int Cmiss_node_field_creator_define_derivative(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	enum Cmiss_nodal_value_type derivative_type);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Adds the derivative of specified <derivative_type> to the <component_number>
specified.
==============================================================================*/

#define Cmiss_node_field_creator_define_versions FE_node_field_creator_define_versions

int Cmiss_node_field_creator_define_versions(
	struct Cmiss_node_field_creator *node_field_creator, int component_number,
	int number_of_versions);
/*******************************************************************************
LAST MODIFIED: 16 November 2001

DESCRIPTION:
Specifies the <number_of_versions> for <component_number> specified.
==============================================================================*/

Cmiss_node_id create_Cmiss_node(int node_identifier,
	Cmiss_region_id region);
/*******************************************************************************
LAST MODIFIED : 8 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
A blank node with the given identifier but no fields is returned.
The new node is set to belong to the ultimate master FE_region of <region>.
==============================================================================*/

Cmiss_node_id create_Cmiss_node_from_template(int node_identifier,
	Cmiss_node_id template_node);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Creates and returns a node with the specified <cm_node_identifier>.
Note that <cm_node_identifier> must be non-negative.
The node copies all the fields and values of the <template_node> and will
belong to the same region.
==============================================================================*/

int destroy_Cmiss_node(Cmiss_node_id *node_id_address);
/*******************************************************************************
LAST MODIFIED : 1 November 2004

DESCRIPTION :
Frees the memory for the node, sets <*node_address> to NULL.
==============================================================================*/

Cmiss_element_id create_Cmiss_element_with_line_shape(int element_identifier,
	Cmiss_region_id region, int dimension);
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Creates an element that has a line shape product of the specified <dimension>.
==============================================================================*/

int Cmiss_element_set_node(Cmiss_element_id element, int node_index,
	Cmiss_node_id node);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Sets node <node_number>, from 0 to number_of_nodes-1 of <element> to <node>.
<element> must already have a shape and node_scale_field_information.
Should only be called for unmanaged elements.
==============================================================================*/

int Cmiss_element_get_identifier(Cmiss_element_id element);
/*******************************************************************************
LAST MODIFIED : 11 November 2004

DESCRIPTION :
Returns the integer identifier of the <element>.
==============================================================================*/

#endif /* __CMISS_FINITE_ELEMENT_H__ */
