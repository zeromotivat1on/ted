#include "pch.h"
#include "font.h"
#include "arena.h"
#include "memory_eater.h"
#include <stdio.h>
#include <malloc.h>

static inline void eat_offset_subtable(void** data, Offset_Subtable* table)
{
    table->scaler_type = eat_big_endian_u32(data);
    table->num_tables = eat_big_endian_u16(data);
    table->search_range = eat_big_endian_u16(data);
    table->entry_selector = eat_big_endian_u16(data);
    table->range_shift = eat_big_endian_u16(data);
}

static inline void eat_table_directory(void** data, Table_Directory* table)
{
    table->tag = eat_big_endian_u32(data);
    table->checksum = eat_big_endian_u32(data);
    table->offset = eat_big_endian_u32(data);
    table->length = eat_big_endian_u32(data);
}

static inline void eat_head(void** data, Head* head)
{
    head->version = eat_big_endian_s32(data);
    head->font_revision = eat_big_endian_s32(data);
    head->checksum_adjustment = eat_big_endian_u32(data);
    head->magic_number = eat_big_endian_u32(data);
    head->flags = eat_big_endian_u16(data);
    head->units_per_em = eat_big_endian_u16(data);
    head->created = eat_big_endian_s64(data);
    head->modified = eat_big_endian_s64(data);
    head->x_min = eat_big_endian_s16(data);
    head->y_min = eat_big_endian_s16(data);
    head->x_max = eat_big_endian_s16(data);
    head->y_max = eat_big_endian_s16(data);
    head->mac_style = eat_big_endian_u16(data);
    head->lowest_rec_ppem = eat_big_endian_u16(data);
    head->font_direction_hint = eat_big_endian_s16(data);
    head->index_to_loc_format = eat_big_endian_s16(data);
    head->glyph_data_format = eat_big_endian_s16(data);
}

static inline void eat_maxp(void** data, Maxp* maxp)
{
    maxp->version = eat_big_endian_s32(data);
    maxp->num_glyphs = eat_big_endian_u16(data);
    maxp->max_points = eat_big_endian_u16(data);
    maxp->max_contours = eat_big_endian_u16(data);
    maxp->max_component_points = eat_big_endian_u16(data);
    maxp->max_component_contours = eat_big_endian_u16(data);
    maxp->max_zones = eat_big_endian_u16(data);
    maxp->max_twilight_points = eat_big_endian_u16(data);
    maxp->max_storage = eat_big_endian_u16(data);
    maxp->max_function_defs = eat_big_endian_u16(data);
    maxp->max_instruction_defs = eat_big_endian_u16(data);
    maxp->max_stack_elements = eat_big_endian_u16(data);
    maxp->max_size_of_instructions = eat_big_endian_u16(data);
    maxp->max_component_elements = eat_big_endian_u16(data);
    maxp->max_component_depth = eat_big_endian_u16(data);
}

static inline void eat_hhea(void** data, Hhea* hhea)
{
    hhea->version = eat_big_endian_s32(data);
    hhea->ascent = eat_big_endian_s16(data);
    hhea->descent = eat_big_endian_s16(data);
    hhea->line_gap = eat_big_endian_s16(data);
    hhea->advance_width_max = eat_big_endian_u16(data);
    hhea->min_left_side_bearing = eat_big_endian_s16(data);
    hhea->min_right_side_bearing = eat_big_endian_s16(data);
    hhea->x_max_extent = eat_big_endian_s16(data);
    hhea->caret_slope_rise = eat_big_endian_s16(data);
    hhea->caret_slope_run = eat_big_endian_s16(data);
    hhea->caret_offset = eat_big_endian_s16(data);
    eat(data, 4 * sizeof(s16)); // skip reserved
    hhea->metric_data_format = eat_big_endian_s16(data);
    hhea->num_of_long_hor_metrics = eat_big_endian_u16(data);
}

static inline void eat_hmtx(Arena* arena, void** data, u16 num_glyphs, u16 num_of_long_hor_metrics, Hmtx* hmtx)
{   
    hmtx->h_metrics = push_array(arena, num_of_long_hor_metrics, Long_Hor_Metric);
    for (u16 i = 0; i < num_of_long_hor_metrics; ++i)
    {
        hmtx->h_metrics[i].advance_width = eat_big_endian_u16(data);
        hmtx->h_metrics[i].left_side_bearing = eat_big_endian_s16(data);
    }
    
    const u16 num_of_left_side_bearings = num_glyphs - num_of_long_hor_metrics;
    hmtx->left_side_bearings = push_array(arena, num_of_left_side_bearings, s16);
    for (u16 i = 0; i < num_of_left_side_bearings; ++i)
    {
        hmtx->left_side_bearings[i] = eat_big_endian_s16(data);
    }    
}

static inline void eat_loca(Arena* arena, void** data, u16 num_glyphs, s16 index_to_loc_format, Loca* loca)
{
    loca->offsets = push_array(arena, num_glyphs + 1, u32);
    
    for (u16 i = 0; i < num_glyphs + 1; ++i)
    {
        if (index_to_loc_format == 0)
        {
            loca->offsets[i] = eat_big_endian_u16(data) * 2;
        }
        else
        {
            loca->offsets[i] = eat_big_endian_u32(data);       
        }
    }
}

static inline void eat_cmap(Arena* arena, void** data, Cmap* cmap)
{
    cmap->version = eat_big_endian_u16(data);
    cmap->number_subtables = eat_big_endian_u16(data);

    cmap->subtables = push_array(arena, cmap->number_subtables, Cmap_Encoding_Subtable);
    for (u16 i = 0; i < cmap->number_subtables; ++i)
    {
        cmap->subtables[i].platform_id = eat_big_endian_u16(data);
        cmap->subtables[i].platform_specific_id = eat_big_endian_u16(data);
        cmap->subtables[i].offset = eat_big_endian_u32(data);
    }
}

static inline void eat_format4(Arena* arena, void** data, Format4* f4)
{
    const u8* start = (u8*)*data;
    
    f4->format = eat_big_endian_u16(data);
    assert(f4->format == 4);
    f4->length = eat_big_endian_u16(data);
    f4->language = eat_big_endian_u16(data);
    f4->seg_count_x2 = eat_big_endian_u16(data);
    f4->search_range = eat_big_endian_u16(data);
    f4->entry_selector = eat_big_endian_u16(data);
    f4->range_shift = eat_big_endian_u16(data);

    const u16 seg_count = f4->seg_count_x2 / 2;
    f4->end_code = push_array(arena, seg_count, u16);
    for (u16 i = 0; i < seg_count; ++i)
        f4->end_code[i] = eat_big_endian_u16(data);

    eat_u16(data); // skip reserved
    
    f4->start_code = push_array(arena, seg_count, u16);
    for (u16 i = 0; i < seg_count; ++i)
        f4->start_code[i] = eat_big_endian_u16(data);

    f4->id_delta = push_array(arena, seg_count, u16);
    for (u16 i = 0; i < seg_count; ++i)
        f4->id_delta[i] = eat_big_endian_u16(data);

    f4->id_range_offset = push_array(arena, seg_count, u16);
    for (u16 i = 0; i < seg_count; ++i)
        f4->id_range_offset[i] = eat_big_endian_u16(data);
    
    const u32 remaining_bytes = f4->length - (u32)((u8*)*data - start);
    const u32 glyph_index_count = remaining_bytes / 2;
    f4->glyph_index_array = (u16*)push(arena, remaining_bytes);
    
    for (u32 i = 0; i < glyph_index_count; ++i)
        f4->glyph_index_array[i] = eat_big_endian_u16(data);
}

static u16 get_glyph_index(Format4* f4, u32 character)
{
    assert(f4->format == 4);
    
    const u16 seg_count = f4->seg_count_x2 / 2;
    s32 code_idx = -1;
	for (u16 i = 0; i < seg_count; i++)
    {
		if (f4->end_code[i] >= character)
        {
            code_idx = i;
            break;
        };
	}
	
	if (code_idx == -1)
        return 0;

	if (f4->start_code[code_idx] < character)
    {
		if (f4->id_range_offset[code_idx] == 0)
        {
            return (character + f4->id_delta[code_idx]) % 65536;
        }
        else
        {
            const u16 glyph_idx = *(f4->id_range_offset + code_idx + f4->id_range_offset[code_idx] / 2 + (character - f4->start_code[code_idx]));
            
			if (glyph_idx == 0)
                return 0;
            
			return (glyph_idx + f4->id_delta[code_idx]) % 65536;
		}
	}
    
	return 0;
}

static inline void eat_format12(Arena* arena, void** data, Format12* f12)
{
    f12->format = eat_big_endian_u16(data);
    assert(f12->format == 12);
    eat_u16(data); // skip reserved
    f12->length = eat_big_endian_u32(data);
    f12->language = eat_big_endian_u32(data);
    f12->n_groups = eat_big_endian_u32(data);
    f12->groups = push_array(arena, f12->n_groups, Format12_Group);
    
    for (u32 i = 0; i < f12->n_groups; ++i)
    {
        auto* group = f12->groups + i;
        group->start_char_code = eat_big_endian_u32(data);
        group->end_char_code = eat_big_endian_u32(data);
        group->start_glyph_code = eat_big_endian_u32(data);
    }
}

static u16 get_glyph_index(Format12* f12, u32 character)
{
    assert(f12->format == 12);
    return 0;
}

static inline void eat_font_directory(Arena* arena, void* data, Font_Directory* dir)
{
    u8* data_start = (u8*)data;
    
    eat_offset_subtable(&data, &dir->offset_subtable);

    dir->table_directories = push_array(arena, dir->offset_subtable.num_tables, Table_Directory);
    
    // First read all tables and non-dependable ones.
    for (u16 i = 0; i < dir->offset_subtable.num_tables; ++i)
    {
        Table_Directory* t = dir->table_directories + i;
        eat_table_directory(&data, t);
        
        switch (t->tag)
        {
            case HEAD_TAG:
            {
                void* head_data = data_start + t->offset;
                eat_head(&head_data, &dir->head);
                break;
            }

            case MAXP_TAG:
            {
                void* maxp_data = data_start + t->offset;
                eat_maxp(&maxp_data, &dir->maxp);
                break;
            }

            case HHEA_TAG:
            {
                void* hhea_data = data_start + t->offset;
                eat_hhea(&hhea_data, &dir->hhea);
                break;
            }
            
            case GLYF_TAG:
            {
                dir->glyf.data = data_start + t->offset;
                break;
            }
            
            case CMAP_TAG:
            {
                void* cmap_data = data_start + t->offset;
                eat_cmap(arena, &cmap_data, &dir->cmap);
    
                // @Todo: assume 4th format by default for now, support others later.
                void* format_data = data_start + t->offset + dir->cmap.subtables[0].offset;

#if LITTLE_ENDIAN
                dir->format_type = swap_endianness_16(format_data);
#else
                dir->format_type = *(u16*)format_data;
#endif
                
                if (dir->format_type == 4)
                {
                    dir->f4 = push_struct(arena, Format4);
                    eat_format4(arena, &format_data, dir->f4);
                }
                else if (dir->format_type == 12)
                {
                    dir->f12 = push_struct(arena, Format12);
                    eat_format12(arena, &format_data, dir->f12);
                }
                
                break;
            }
        }
    }

    // Then read dependable tables.
    for (u16 i = 0; i < dir->offset_subtable.num_tables; ++i)
    {
        Table_Directory* t = dir->table_directories + i;

        switch (t->tag)
        {
            case HMTX_TAG:
            {    
                void* hmtx_data = data_start + t->offset;
                eat_hmtx(arena, &hmtx_data, dir->maxp.num_glyphs, dir->hhea.num_of_long_hor_metrics, &dir->hmtx);
                break;
            }

            case LOCA_TAG:
            {
                void* loca_data = data_start + t->offset;
                eat_loca(arena, &loca_data, dir->maxp.num_glyphs, dir->head.index_to_loc_format, &dir->loca);
                break;
            }
        }
    }
}

static void eat_component_glyph(void** data, Component_Glyph* component)
{    
    component->flag.val = eat_big_endian_u16(data);
    component->glyph_index = eat_big_endian_u16(data);

    if (component->flag.arg_1_and_2_are_words)
    {
        component->argument1 = eat_big_endian_s16(data);
        component->argument2 = eat_big_endian_s16(data);
    }
    else
    {
        component->argument1 = eat_s8(data);
        component->argument2 = eat_s8(data);
    }

    component->e = 0.0f;
    component->f = 0.0f;
    
    if (component->flag.args_are_xy_values)
    {
        component->e = (f32)component->argument1;
        component->f = (f32)component->argument2;
    }
    else
    {
        // @Todo: handle points.
        assert(false && "Unhandled entry");
    }
    
    component->a = 1.0f;
    component->b = 0.0f;
    component->c = 0.0f;
    component->d = 1.0f;

    if (component->flag.we_have_a_scale)
    {
        component->a = eat_big_endian_f2dot14(data);
        component->d = component->a;
    }
    else if (component->flag.we_have_an_x_and_y_scale)
    {
        component->a = eat_big_endian_f2dot14(data);
        component->d = eat_big_endian_f2dot14(data);
    }
    else if (component->flag.we_have_a_two_by_two)
    {
        component->a = eat_big_endian_f2dot14(data);
        component->b = eat_big_endian_f2dot14(data);
        component->c = eat_big_endian_f2dot14(data);
        component->d = eat_big_endian_f2dot14(data);
    }
}

static void eat_glyph_arrays(Font_Face* face, void* data, u16 countours_num, u16* countours, s16* x_coordinates, s16* y_coordinates)
{
    for (s16 i = 0; i < countours_num; ++i)
    {
        countours[i] = eat_big_endian_u16(&data);
    }

    const u16 instruction_length = eat_big_endian_u16(&data); // skip instruction length
    eat(&data, instruction_length); // skip instructions

    const u16 last_idx = countours[countours_num - 1];
    auto* flags = (Glyph_Outline_Flag*)alloca(last_idx + 1);

    for (u16 i = 0; i <= last_idx; ++i)
    {
        flags[i].val = eat_u8(&data);

        if (flags[i].repeat)
        {
            u8 repeat_count = eat_u8(&data);
            while (repeat_count > 0)
            {
                i++;
                flags[i] = flags[i - 1];
                repeat_count--;
            }
        }
    }

    s16 prev_coordinate = 0;
    s16 curr_coordinate = 0;

    for (u16 i = 0; i <= last_idx; ++i)
    {
        const u8 combined_flag = flags[i].x_short << 1 | flags[i].x_same;
        switch (combined_flag)
        {
            case 0: curr_coordinate = eat_big_endian_s16(&data); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -eat_u8(&data); break;
            case 3: curr_coordinate = eat_u8(&data); break;
        }
        
        x_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = x_coordinates[i];
    }

    prev_coordinate = 0;
    curr_coordinate = 0;

    for (u16 i = 0; i <= last_idx; ++i)
    {
        const u8 combined_flag = flags[i].y_short << 1 | flags[i].y_same;
        switch (combined_flag)
        {
            case 0: curr_coordinate = eat_big_endian_s16(&data); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -eat_u8(&data); break;
            case 3: curr_coordinate = eat_u8(&data); break;
        }
        
        y_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = y_coordinates[i];
    }
}

static inline void* get_glyph_data(Font_Face* face, u16 glyph_index)
{
    return face->dir->glyf.data + face->dir->loca.offsets[glyph_index];
}

extern u8* read_entire_file(Arena* arena, const char* path, s32* size_read);

void init_font_face(Arena* arena, const char* font_path, Font_Face* face)
{ 
    u8* font_data = read_entire_file(arena, font_path, nullptr);
    face->dir = push_struct(arena, Font_Directory);
    eat_font_directory(arena, font_data, face->dir);

    face->glyph = push_struct(arena, Glyph_Slot);

    const Maxp* maxp = &face->dir->maxp;
    const u16 max_contours = max(maxp->max_contours, maxp->max_component_contours);
    face->glyph->end_pts_of_countours = push_array(arena, max_contours, u16);

    const u16 max_points = max(maxp->max_points, maxp->max_component_points);
    face->glyph->x_coordinates = push_array(arena, max_points * 2, s16);
    face->glyph->y_coordinates = push_array(arena, max_points * 2, s16);
}

u16 get_glyph_index(Font_Face* face, u32 character)
{
    if (face->dir->format_type == 4)
        return get_glyph_index(face->dir->f4, character);

    if (face->dir->format_type == 12)
        return get_glyph_index(face->dir->f12, character);

    printf("Unsupported format type (%u)\n", face->dir->format_type);
    return 0;
}

bool load_glyph(Font_Face* face, u16 glyph_index)
{
    if (glyph_index == 0)
    {
        printf("Invalid glyph index (%u)\n", glyph_index);
        return false;
    }
    
    void* data = get_glyph_data(face, glyph_index);
    auto* glyph = face->glyph;
    
    glyph->number_of_countours = eat_big_endian_s16(&data);
    glyph->x_min = eat_big_endian_s16(&data);
    glyph->y_min = eat_big_endian_s16(&data);
    glyph->x_max = eat_big_endian_s16(&data);
    glyph->y_max = eat_big_endian_s16(&data);

    if (glyph->number_of_countours >= 0)
    {
        eat_glyph_arrays(face, data, glyph->number_of_countours, glyph->end_pts_of_countours, glyph->x_coordinates, glyph->y_coordinates);
    }
    else // @Todo: probably won't work with component depth > 1 ?
    {
        s16 total_component_countours = 0;
        s16 total_component_points = 0;
        u16* component_contours = nullptr;
        s16* component_x_coordinates = nullptr;
        s16* component_y_coordinates = nullptr;
        
        while (true)
        {           
            Component_Glyph component;
            eat_component_glyph(&data, &component);

            void* component_data = get_glyph_data(face, component.glyph_index);
            const s16 component_number_of_countours = eat_big_endian_s16(&component_data);
            eat(&component_data, 4 * sizeof(u16)); // skip min/max points

            component_contours = glyph->end_pts_of_countours + total_component_countours;
            component_x_coordinates = glyph->x_coordinates + total_component_points;
            component_y_coordinates = glyph->y_coordinates + total_component_points;
            
            eat_glyph_arrays(face, component_data, component_number_of_countours, component_contours, component_x_coordinates, component_y_coordinates);
            
            // Offset countour indices.
            for (s16 i = 0; i < component_number_of_countours; ++i)
            {
                component_contours[i] += total_component_points;
            }
            
            const u16 component_last_idx = component_contours[component_number_of_countours - 1];
            total_component_countours += component_number_of_countours;
            total_component_points += component_last_idx + 1;
            
            // Transform coordinates.
            for(u16 i = 0; i <= component_last_idx; ++i)
            {
                f32 x = (f32)component_x_coordinates[i];
                f32 y = (f32)component_y_coordinates[i];

                x = component.a * x + component.c * y + component.e;
                y = component.b * x + component.d * y + component.f;

                component_x_coordinates[i] = (s16)x;
                component_y_coordinates[i] = (s16)y;
            }
                    
            if (!component.flag.more_components)
                break;
        }

        glyph->number_of_countours = total_component_countours;
    }
    
    return true;
}

bool load_char(Font_Face* face, u32 character)
{
    const u16 idx = get_glyph_index(face, character);
    return load_glyph(face, idx);
}

void print_font_directory(Font_Directory* dir)
{
    printf("#)\ttag\tlen\toffset\n");
	for(u16 i = 0; i < dir->offset_subtable.num_tables; ++i)
    {
		Table_Directory* t = dir->table_directories + i;
		printf("%u)\t%c%c%c%c\t%u\t%u\n", i + 1,
		        t->tag_str[3], t->tag_str[2],
				t->tag_str[1], t->tag_str[0],
				t->length, t->offset);
	}
}

void print_cmap(Cmap* cmap)
{
	printf("#)\tp_id\tps_id\toffset\ttype\n");

    for (u16 i = 0; i < cmap->number_subtables; ++i)
    {
        Cmap_Encoding_Subtable* cet = cmap->subtables + i;

        char* platform_str = (char*)alloca(32);
        switch (cet->platform_id)
        {
            case 0: platform_str = "Unicode"; break;
            case 1: platform_str = "Mac"; break;
            case 2: platform_str = "Not Supported"; break;
            case 3: platform_str = "Microsoft"; break;
        }
        
        printf("%u)\t%u\t%u\t%u\t%s\n", i + 1,
                cet->platform_id,
                cet->platform_specific_id,
                cet->offset,
                platform_str);
	}
}

void print_format4(Format4* f4)
{
    const u16 seg_count = f4->seg_count_x2 / 2;

    printf("format: %u, length: %u, language: %u, seg_count: %u\n", f4->format, f4->length, f4->language, seg_count);
    printf("search_range: %u, entry_celector: %u, range_shift: %u)\n", f4->search_range, f4->entry_selector, f4->range_shift);
    printf("#)\tstart_code\tend_code\tid_delta\tid_range_offset\n");

	for (u16 i = 0; i < seg_count; ++i)
    {
	    printf("%u)\t%10u\t%8u\t%8u\t%15u\n", i + 1,
                f4->start_code[i],
                f4->end_code[i],
                f4->id_delta[i],
                f4->id_range_offset[i]);
    }
}

void print_glyph_data(Glyph_Slot* glyph)
{
    printf("#contours\t(x_min, y_min)\t(x_max, y_max)\n");
	printf("%9d\t(%d,%d)\t\t(%d,%d)\n", glyph->number_of_countours,
			glyph->x_min, glyph->y_min,
			glyph->x_max, glyph->y_max);

	printf("#)\t(  x  ,  y  )\n");
	const u16 last_idx = glyph->end_pts_of_countours[glyph->number_of_countours - 1];
	for(u16 i = 0; i <= last_idx; ++i)
    {
		printf("%d)\t(%5d,%5d)\n", i + 1, glyph->x_coordinates[i], glyph->y_coordinates[i]);
	}
}
