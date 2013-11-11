/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>

#include "zinc/scenefilter.h"
#include "zinc/status.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/render_gl.h"
#include "graphics/scene.h"
#include "graphics/scene_picker.hpp"
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
#include "interaction/interaction_volume.h"
#include "region/cmiss_region.h"

#define SELECT_BUFFER_SIZE_INCREMENT 10000

cmzn_scenepicker::cmzn_scenepicker(cmzn_scenefiltermodule_id filter_module_in) :
	interaction_volume(0),
	top_scene(0),
	scene_viewer(0),
	centre_x(0), centre_y(0),
	size_x(0), size_y(0),
	coordinate_system(CMZN_SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT),
	filter(0),
	select_buffer(0),
	select_buffer_size(10000),
	number_of_hits(0),
	filter_module(cmzn_scenefiltermodule_access(filter_module_in)),
	access_count(1)
{
}

cmzn_scenepicker::~cmzn_scenepicker()
{
	if (interaction_volume)
		DEACCESS(Interaction_volume)(&interaction_volume);
	if (scene_viewer)
		cmzn_sceneviewer_destroy(&scene_viewer);
	if (top_scene)
		cmzn_scene_destroy(&top_scene);
	if (filter)
		cmzn_scenefilter_destroy(&filter);
	if (select_buffer)
		DEALLOCATE(select_buffer);
	if (filter_module)
		cmzn_scenefiltermodule_destroy(&filter_module);
}

void cmzn_scenepicker::updateViewerRectangle()
{
	if (scene_viewer)
	{
		if (interaction_volume)
			DEACCESS(Interaction_volume)(&interaction_volume);
		GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
		double viewport_bottom,viewport_height, viewport_left,viewport_width,
			viewport_pixels_per_unit_x, viewport_pixels_per_unit_y;
		int i, j, width, height;
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				temp_modelview_matrix[i*4+j] =
					scene_viewer->modelview_matrix[j*4+i];
				temp_projection_matrix[i*4+j] =
					scene_viewer->window_projection_matrix[j*4+i];
			}
		}

		Scene_viewer_get_viewport_info(scene_viewer,
			&viewport_left, &viewport_bottom, &viewport_pixels_per_unit_x,
			&viewport_pixels_per_unit_y);
		Scene_viewer_get_viewport_size(scene_viewer,	&width, &height);
		viewport_width  = ((double)width)/viewport_pixels_per_unit_x;
		viewport_height = ((double)height)/viewport_pixels_per_unit_y;
		/* recalculate the centre of y as windows coordinate is different from the
		 * GL coordinate */
		double new_centre_y = viewport_height - (double)(centre_y) - 1.0;
		interaction_volume = create_Interaction_volume_ray_frustum(
			temp_modelview_matrix, temp_projection_matrix,
			viewport_left, viewport_bottom, viewport_width, viewport_height,
			centre_x, new_centre_y, size_x, size_y);
	}
}

int cmzn_scenepicker::pickObjects()
{
	double modelview_matrix[16],projection_matrix[16];
	GLdouble opengl_modelview_matrix[16],opengl_projection_matrix[16];
	int i, j, return_code = CMZN_ERROR_GENERAL;
	updateViewerRectangle();
	if (select_buffer != NULL)
		return CMZN_OK;
	if (!has_current_context())
		return CMZN_ERROR_GENERAL;
	if (top_scene&&interaction_volume)
	{
		Render_graphics_opengl *renderer = Render_graphics_opengl_create_glbeginend_renderer();
		renderer->picking = 1;
		if (renderer->Scene_compile(top_scene, filter))
		{
			number_of_hits=-1;
			while (0>number_of_hits)
			{
				if (ALLOCATE(select_buffer,GLuint,select_buffer_size))
				{
					Interaction_volume_get_modelview_matrix(interaction_volume,
						modelview_matrix);
					Interaction_volume_get_projection_matrix(interaction_volume,
						projection_matrix);
					/* transpose projection matrix for OpenGL */
					for (i=0;i<4;i++)
					{
						for (j=0;j<4;j++)
						{
							opengl_modelview_matrix[j*4+i] = modelview_matrix[i*4+j];
							opengl_projection_matrix[j*4+i] = projection_matrix[i*4+j];
						}
					}
					renderer->set_world_view_matrix(opengl_modelview_matrix);

					glSelectBuffer(select_buffer_size,select_buffer);
					glRenderMode(GL_SELECT);
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glMultMatrixd(opengl_projection_matrix);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					glMultMatrixd(opengl_modelview_matrix);
					/* set an arbitrary viewport - not really needed
						   SAB 22 July 2004 This is causing the view frustrums
						   to not match when picking, so instead I am not changing the
						   viewport, so presumably the last rendered viewport is OK. */
					/* glViewport(0,0,1024,1024); */
					glDepthRange((GLclampd)0,(GLclampd)1);
					{
						do
						{
							return_code = renderer->Scene_tree_execute(top_scene);
						}
						while (return_code && renderer->next_layer());
					}
					glFlush();
					number_of_hits=glRenderMode(GL_RENDER);
					if (0<=number_of_hits)
					{
						return_code=CMZN_OK;
					}
					else
					{
						/* select buffer overflow; enlarge and repeat */
						select_buffer_size += SELECT_BUFFER_SIZE_INCREMENT;
						DEALLOCATE(select_buffer);
					}
				}
			}
		}

		delete renderer;
	}
	return return_code;
}

Region_node_map cmzn_scenepicker::getPickedRegionSortedNodes(
	enum cmzn_scenepicker_object_type type)
{
	Region_node_map node_map;

	if ((CMZN_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		cmzn_scene_id picked_scene = 0, existing_scene = 0;
		cmzn_graphics_id graphics = 0;
		cmzn_nodeset_id nodeset = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = scene
			 * select_buffer[4] = graphics position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 4)
			{
				if ((getSceneAndGraphics(select_buffer_ptr,
						&picked_scene, &graphics) && (0 != picked_scene) && (0 != graphics)))
				{
					if (((type == CMZN_SCENEPICKER_OBJECT_DATA) &&
							(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS == cmzn_graphics_get_field_domain_type(graphics))) ||
						((type == CMZN_SCENEPICKER_OBJECT_NODE) &&
							(CMZN_FIELD_DOMAIN_TYPE_NODES == cmzn_graphics_get_field_domain_type(graphics))))
					{
						if (picked_scene)
						{
							cmzn_region *region = cmzn_scene_get_region(picked_scene);
							if (existing_scene != picked_scene)
							{
								existing_scene = picked_scene;
								cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
								if (nodeset)
									cmzn_nodeset_destroy(&nodeset);
								nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(field_module,
									(type == CMZN_SCENEPICKER_OBJECT_DATA) ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
								cmzn_fieldmodule_destroy(&field_module);
							}
							cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset,
								(int)(select_buffer_ptr[6]));
							if (node)
							{
								node_map.insert(std::make_pair(region, node));
								cmzn_node_destroy(&node);
							}
						}
					}
					cmzn_graphics_destroy(&graphics);
				}
			}
		}
		if (nodeset)
			cmzn_nodeset_destroy(&nodeset);
	}
	return node_map;
}

Region_element_map cmzn_scenepicker::getPickedRegionSortedElements()
{
	Region_element_map element_map;

	if ((CMZN_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		cmzn_scene_id picked_scene = 0, existing_scene = 0;
		cmzn_graphics_id graphics = 0;
		cmzn_mesh_id mesh = 0;
		int current_element_type = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = scene
			 * select_buffer[4] = graphics position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 3)
			{
				if ((getSceneAndGraphics(select_buffer_ptr,
					&picked_scene, &graphics) && (0 != picked_scene) && (0 != graphics)))
				{
					if (cmzn_graphics_selects_elements(graphics))
					{
						if (picked_scene)
						{
							cmzn_region *region = cmzn_scene_get_region(picked_scene);
							int element_type = cmzn_graphics_get_domain_dimension(graphics);
							if ((existing_scene != picked_scene) ||
								(current_element_type != element_type))
							{
								existing_scene = picked_scene;
								current_element_type = element_type;
								cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
								if (mesh)
									cmzn_mesh_destroy(&mesh);
								mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, current_element_type);
								cmzn_fieldmodule_destroy(&field_module);
							}
							cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh,
								(int)(select_buffer_ptr[5]));
							if (element)
							{
								element_map.insert(std::make_pair(region, element));
								cmzn_element_destroy(&element);
							}
						}
					}
					cmzn_graphics_destroy(&graphics);
				}
			}
		}
		cmzn_mesh_destroy(&mesh);
	}
	return element_map;
}

void cmzn_scenepicker::reset()
{
	if (select_buffer)
	{
		DEALLOCATE(select_buffer);
		select_buffer = NULL;
	}
	select_buffer_size = 10000;
	number_of_hits = 0;
}

/*provide a select buffer pointer and return the scene and graphics */
int cmzn_scenepicker::getSceneAndGraphics(GLuint *select_buffer_ptr,
	cmzn_scene_id *picked_scene, cmzn_graphics_id *graphics)
{
	if (top_scene && select_buffer_ptr)
	{
		*picked_scene = cmzn_scene_get_child_of_position(top_scene, (int)(select_buffer_ptr[3]));
		*graphics = cmzn_scene_get_graphics_at_position(*picked_scene,
			(int)(select_buffer_ptr[4]));
		return 1;
	}
	else
	{
		*picked_scene = 0;
		*graphics = 0;
		return 0;
	}
}

cmzn_scenefilter_id cmzn_scenepicker::getScenefilter()
{
	return cmzn_scenefilter_access(filter);
}

int cmzn_scenepicker::setScenefilter(cmzn_scenefilter_id filter_in)
{
	if (filter_in != filter)
	{
		reset();
		if (filter)
			cmzn_scenefilter_destroy(&filter);
		if (filter_in)
			filter = cmzn_scenefilter_access(filter_in);
	}
	return CMZN_OK;
}

cmzn_scene_id cmzn_scenepicker::getScene()
{
	return cmzn_scene_access(top_scene);
}

int cmzn_scenepicker::setScene(cmzn_scene_id scene_in)
{
	if (scene_in)
	{
		if (scene_in != top_scene)
		{
			reset();
			if (top_scene)
				cmzn_scene_destroy(&top_scene);
			top_scene = cmzn_scene_access(scene_in);
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_scenepicker::setSceneviewerRectangle(cmzn_sceneviewer_id scene_viewer_in,
	enum cmzn_scenecoordinatesystem coordinate_system_in, double x1,
	double y1, double x2, double y2)
{
	reset();
	if (scene_viewer_in)
	{
		if (scene_viewer)
			cmzn_sceneviewer_destroy(&scene_viewer);
		coordinate_system = coordinate_system_in;
		scene_viewer = cmzn_sceneviewer_access(scene_viewer_in);
		size_x = x2 - x1;
		size_y = y2 - y1;
		centre_x = x1 + size_x/2;
		centre_y = y1 + size_y/2;
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_scenepicker::setInteractionVolume(struct Interaction_volume *interaction_volume_in)
{
	reset();
	if (scene_viewer)
		cmzn_sceneviewer_destroy(&scene_viewer);
	size_x = 0;
	size_y = 0;
	centre_x = 0;
	centre_y = 0;
	REACCESS(Interaction_volume)(&interaction_volume, interaction_volume_in);
	return 1;
}

cmzn_element_id cmzn_scenepicker::getNearestElement()
{
	cmzn_element_id nearest_element = 0;
	if ((CMZN_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names, current_element_type = 0;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		cmzn_scene_id picked_scene = 0, existing_scene = 0;
		cmzn_graphics_id graphics = 0;
		cmzn_mesh_id mesh = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = scene
			 * select_buffer[4] = graphics position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 3)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_element == NULL) || (nearest < current_nearest))
				{
					if ((getSceneAndGraphics(select_buffer_ptr,
						&picked_scene, &graphics) && (0 != picked_scene) && (0 != graphics)))
					{
						if (cmzn_graphics_selects_elements(graphics))
						{
							if (picked_scene)
							{
								cmzn_region *region = cmzn_scene_get_region(picked_scene);
								int element_type = cmzn_graphics_get_domain_dimension(graphics);
								if ((existing_scene != picked_scene) ||
									(current_element_type != element_type))
								{
									existing_scene = picked_scene;
									current_element_type = element_type;
									cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
									if (mesh)
										cmzn_mesh_destroy(&mesh);
									mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, current_element_type);
									cmzn_fieldmodule_destroy(&field_module);
								}

								cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh,
									(int)(select_buffer_ptr[5]));
								if (element)
								{
									if (nearest_element)
										cmzn_element_destroy(&nearest_element);
									nearest_element = element;
									current_nearest = nearest;
								}
							}
						}
						cmzn_graphics_destroy(&graphics);
					}
				}
			}
		}
		if (mesh)
			cmzn_mesh_destroy(&mesh);
	}
	return nearest_element;
}

cmzn_node_id cmzn_scenepicker::getNearestNode(enum cmzn_scenepicker_object_type type)
{
	cmzn_node_id nearest_node = 0;
	if ((CMZN_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		cmzn_scene_id picked_scene = 0, existing_scene = 0;
		cmzn_graphics_id graphics = 0;
		cmzn_nodeset_id nodeset = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = scene
			 * select_buffer[4] = graphics position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 4)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_node == NULL) || (nearest < current_nearest))
				{
					if ((getSceneAndGraphics(select_buffer_ptr,
						&picked_scene, &graphics) && (0 != picked_scene) && (0 != graphics)))
					{
						if (((type == CMZN_SCENEPICKER_OBJECT_DATA) &&
								(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS == cmzn_graphics_get_field_domain_type(graphics))) ||
							((type == CMZN_SCENEPICKER_OBJECT_NODE) &&
								(CMZN_FIELD_DOMAIN_TYPE_NODES == cmzn_graphics_get_field_domain_type(graphics))))
						{
							if (picked_scene)
							{
								if (existing_scene != picked_scene)
								{
									existing_scene = picked_scene;
									cmzn_region *region = cmzn_scene_get_region(picked_scene);
									cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
									if (nodeset)
										cmzn_nodeset_destroy(&nodeset);
									nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(field_module,
										(type == CMZN_SCENEPICKER_OBJECT_DATA) ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
									cmzn_fieldmodule_destroy(&field_module);
								}
								cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset,
									(int)(select_buffer_ptr[6]));
								if (node)
								{
									if (nearest_node)
										cmzn_node_destroy(&nearest_node);
									nearest_node = node;
									current_nearest = nearest;
								}
							}
						}
						cmzn_graphics_destroy(&graphics);
					}
				}
			}
		}
		if (nodeset)
			cmzn_nodeset_destroy(&nodeset);
	}
	return nearest_node;
}

cmzn_graphics_id cmzn_scenepicker::getNearestGraphics(enum cmzn_scenepicker_object_type type)
{
	if ((CMZN_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		cmzn_scene_id picked_scene = 0;
		cmzn_graphics_id graphics = 0, nearest_graphics = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = scene
			 * select_buffer[4] = graphics position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 2)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_graphics == NULL) || (nearest < current_nearest))
				{
					if ((getSceneAndGraphics(select_buffer_ptr,
						&picked_scene, &graphics) && (0 != picked_scene) && (0 != graphics)))
					{
						if ((type == CMZN_SCENEPICKER_OBJECT_ANY) ||
							((type == CMZN_SCENEPICKER_OBJECT_ELEMENT) &&
								(cmzn_graphics_selects_elements(graphics))) ||
							((type == CMZN_SCENEPICKER_OBJECT_DATA) &&
								(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS == cmzn_graphics_get_field_domain_type(graphics))) ||
							((type == CMZN_SCENEPICKER_OBJECT_NODE) &&
								(CMZN_FIELD_DOMAIN_TYPE_NODES == cmzn_graphics_get_field_domain_type(graphics))))
						{
							current_nearest = nearest;
							if (graphics != nearest_graphics)
								REACCESS(cmzn_graphics)(&nearest_graphics, graphics);
						}
						cmzn_graphics_destroy(&graphics);
					}
				}
			}
		}
		return nearest_graphics;
	}
	return 0;
}

int cmzn_scenepicker::addPickedElementsToGroup(cmzn_field_group_id group)
{
	if (group)
	{
		Region_element_map element_map = getPickedRegionSortedElements();
		if (!element_map.empty())
		{
			cmzn_region *sub_region = NULL;
			cmzn_field_group_id selection_group = NULL;
			Region_element_map::iterator pos;
			cmzn_mesh_group_id mesh_group[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			int iter = 0;
			for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
			{
				mesh_group[iter] = 0;
			}
			for (pos = element_map.begin(); pos != element_map.end(); ++pos)
			{
				if (pos->first != sub_region)
				{
					if (selection_group)
					{
						cmzn_field_group_destroy(&selection_group);
					}
					for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
					{
						if (mesh_group[iter])
						{
							cmzn_mesh_group_destroy(&(mesh_group[iter]));
						}
					}
					sub_region = pos->first;
					if (sub_region)
					{
						selection_group = cmzn_field_group_get_subregion_group(group, sub_region);
						if (!selection_group)
						{
							selection_group = cmzn_field_group_create_subregion_group(group, sub_region);
						}
					}
				}
				if (sub_region && selection_group)
				{
					cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(sub_region);
					int dimension = cmzn_element_get_dimension(pos->second);
					if (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
					{
						if (!mesh_group[dimension - 1])
						{
							cmzn_mesh_id temp_mesh =
								cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
							cmzn_field_element_group_id element_group =
								cmzn_field_group_get_element_group(selection_group, temp_mesh);
							if (!element_group)
							{
								element_group = cmzn_field_group_create_element_group(selection_group, temp_mesh);
							}
							mesh_group[dimension - 1] = cmzn_field_element_group_get_mesh(element_group);
							cmzn_field_element_group_destroy(&element_group);
							cmzn_mesh_destroy(&temp_mesh);
						}
						cmzn_mesh_group_add_element(mesh_group[dimension - 1], pos->second);
					}
					cmzn_fieldmodule_destroy(&field_module);
				}
			}
			if (selection_group)
			{
				cmzn_field_group_destroy(&selection_group);
			}
			for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
			{
				if (mesh_group[iter])
				{
					cmzn_mesh_group_destroy(&(mesh_group[iter]));
				}
			}
		}
	}
	else
	{
		return CMZN_ERROR_ARGUMENT;
	}
	return CMZN_OK;
}

int cmzn_scenepicker::addPickedNodesToGroup(cmzn_field_group_id group,
	enum cmzn_scenepicker_object_type type)
{
	if (group)
	{
		Region_node_map node_map = getPickedRegionSortedNodes(type);
		if (!node_map.empty())
		{
			cmzn_region *sub_region = NULL;
			cmzn_field_group_id selection_group = NULL;
			cmzn_nodeset_group_id nodeset_group = NULL;
			Region_node_map::iterator pos;
			for (pos = node_map.begin(); pos != node_map.end(); ++pos)
			{
				if (pos->first != sub_region)
				{
					if (sub_region && selection_group)
					{
						cmzn_field_group_destroy(&selection_group);
					}
					if (nodeset_group)
					{
						cmzn_nodeset_group_destroy(&nodeset_group);
					}
					sub_region = pos->first;
					if (sub_region)
					{
						selection_group = cmzn_field_group_get_subregion_group(group, sub_region);
						if (!selection_group)
						{
							selection_group = cmzn_field_group_create_subregion_group(group, sub_region);
						}
					}
					if (selection_group)
					{
						cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(sub_region);
						cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(field_module,
							(type == CMZN_SCENEPICKER_OBJECT_DATA) ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
						if (master_nodeset)
						{
							cmzn_field_node_group_id node_group = cmzn_field_group_get_node_group(
								selection_group, master_nodeset);
							if (!node_group)
							{
								node_group = cmzn_field_group_create_node_group(selection_group, master_nodeset);
							}
							nodeset_group = cmzn_field_node_group_get_nodeset(node_group);
							cmzn_field_node_group_destroy(&node_group);
							cmzn_nodeset_destroy(&master_nodeset);
						}
						cmzn_fieldmodule_destroy(&field_module);
					}
				}
				if (nodeset_group)
				{
					cmzn_nodeset_group_add_node(nodeset_group, pos->second);
				}
			}
			if (selection_group)
			{
				cmzn_field_group_destroy(&selection_group);
			}
			if (nodeset_group)
			{
				cmzn_nodeset_group_destroy(&nodeset_group);
			}
		}
	}
	else
	{
		return CMZN_ERROR_ARGUMENT;
	}
	return CMZN_OK;
}

int DESTROY(cmzn_scenepicker)(struct cmzn_scenepicker **scenepicker_address)
{
	if (scenepicker_address && (*scenepicker_address))
	{
		delete *scenepicker_address;
		*scenepicker_address = NULL;
		return 1;
	}
	else
	{
		return 0;
	}
}

cmzn_scenepicker_id cmzn_scenepicker_create(cmzn_scenefiltermodule_id filter_module)
{
	return new cmzn_scenepicker(filter_module);
}

DECLARE_OBJECT_FUNCTIONS(cmzn_scenepicker);

cmzn_scenepicker_id cmzn_scenepicker_access(
	cmzn_scenepicker_id scenepicker)
{
	if (scenepicker)
		return scenepicker->access();
	return 0;
}

int cmzn_scenepicker_destroy(cmzn_scenepicker_id *scenepicker_address)
{
	return DEACCESS(cmzn_scenepicker)(scenepicker_address);
}

cmzn_scene_id cmzn_scenepicker_get_scene(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getScene();
}

int cmzn_scenepicker_set_scene(cmzn_scenepicker_id scenepicker,
	cmzn_scene_id scene)
{
	return scenepicker->setScene(scene);
}

cmzn_scenefilter_id cmzn_scenepicker_get_scenefilter(
	cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getScenefilter();
}

int cmzn_scenepicker_set_scenefilter(cmzn_scenepicker_id scenepicker,
	cmzn_scenefilter_id filter)
{
	return scenepicker->setScenefilter(filter);
}

int cmzn_scenepicker_set_sceneviewer_rectangle(
	cmzn_scenepicker_id scenepicker, cmzn_sceneviewer_id sceneviewer_in,
	enum cmzn_scenecoordinatesystem coordinate_system_in, double x1,
		double y1, double x2, double y2)
{
	return scenepicker->setSceneviewerRectangle(sceneviewer_in,
		coordinate_system_in, x1, y1, x2, y2);
}

cmzn_element_id cmzn_scenepicker_get_nearest_element(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestElement();
}

cmzn_node_id cmzn_scenepicker_get_nearest_data(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestNode(CMZN_SCENEPICKER_OBJECT_DATA);
}

cmzn_node_id cmzn_scenepicker_get_nearest_node(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestNode(CMZN_SCENEPICKER_OBJECT_NODE);
}

cmzn_graphics_id cmzn_scenepicker_get_nearest_graphics(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestGraphics(CMZN_SCENEPICKER_OBJECT_ANY);
}

cmzn_graphics_id cmzn_scenepicker_get_nearest_data_graphics(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestGraphics(CMZN_SCENEPICKER_OBJECT_DATA);
}

cmzn_graphics_id cmzn_scenepicker_get_nearest_element_graphics(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestGraphics(CMZN_SCENEPICKER_OBJECT_ELEMENT);
}

cmzn_graphics_id cmzn_scenepicker_get_nearest_node_graphics(cmzn_scenepicker_id scenepicker)
{
	return scenepicker->getNearestGraphics(CMZN_SCENEPICKER_OBJECT_NODE);
}

int cmzn_scenepicker_add_picked_data_to_group(cmzn_scenepicker_id scenepicker,
	cmzn_field_group_id group)
{
	return scenepicker->addPickedNodesToGroup(group, CMZN_SCENEPICKER_OBJECT_DATA);
}

int cmzn_scenepicker_add_picked_elements_to_group(cmzn_scenepicker_id scenepicker,
	cmzn_field_group_id group)
{
	return scenepicker->addPickedElementsToGroup(group);
}

int cmzn_scenepicker_add_picked_nodes_to_group(cmzn_scenepicker_id scenepicker,
	cmzn_field_group_id group)
{
	return scenepicker->addPickedNodesToGroup(group, CMZN_SCENEPICKER_OBJECT_NODE);
}

int cmzn_scenepicker_set_interaction_volume(cmzn_scenepicker_id scenepicker,
	struct Interaction_volume *interaction_volume)
{
	return scenepicker->setInteractionVolume(interaction_volume);
}
