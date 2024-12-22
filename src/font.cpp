#include "pch.h"
#include "font.h"
#include "memory_eater.h"
#include <malloc.h>

void Offset_Subtable::read(void** data)
{
    scaler_type = eat_big_endian_u32(data);
    num_tables = eat_big_endian_u16(data);
    search_range = eat_big_endian_u16(data);
    entry_selector = eat_big_endian_u16(data);
    range_shift = eat_big_endian_u16(data);
}

void Table_Directory::read(void** data)
{
    tag = eat_big_endian_u32(data);
    checksum = eat_big_endian_u32(data);
    offset = eat_big_endian_u32(data);
    length = eat_big_endian_u32(data);
}

void Head::read(void** data)
{
    version = eat_big_endian_s32(data);
    font_revision = eat_big_endian_s32(data);
    checksum_adjustment = eat_big_endian_u32(data);
    magic_number = eat_big_endian_u32(data);
    flags = eat_big_endian_u16(data);
    units_per_em = eat_big_endian_u16(data);
    created = eat_big_endian_s64(data);
    modified = eat_big_endian_s64(data);
    x_min = eat_big_endian_s16(data);
    y_min = eat_big_endian_s16(data);
    x_max = eat_big_endian_s16(data);
    y_max = eat_big_endian_s16(data);
    mac_style = eat_big_endian_u16(data);
    lowest_rec_ppem = eat_big_endian_u16(data);
    font_direction_hint = eat_big_endian_s16(data);
    index_to_loc_format = eat_big_endian_s16(data);
    glyph_data_format = eat_big_endian_s16(data);
}

void Maxp::read(void** data)
{
    version = eat_big_endian_s32(data);
    num_glyphs = eat_big_endian_u16(data);
    max_points = eat_big_endian_u16(data);
    max_contours = eat_big_endian_u16(data);
    max_component_points = eat_big_endian_u16(data);
    max_component_contours = eat_big_endian_u16(data);
    max_zones = eat_big_endian_u16(data);
    max_twilight_points = eat_big_endian_u16(data);
    max_storage = eat_big_endian_u16(data);
    max_function_defs = eat_big_endian_u16(data);
    max_instruction_defs = eat_big_endian_u16(data);
    max_stack_elements = eat_big_endian_u16(data);
    max_size_of_instructions = eat_big_endian_u16(data);
    max_component_elements = eat_big_endian_u16(data);
    max_component_depth = eat_big_endian_u16(data);
}

void Hhea::read(void** data)
{
    version = eat_big_endian_s32(data);
    ascent = eat_big_endian_s16(data);
    descent = eat_big_endian_s16(data);
    line_gap = eat_big_endian_s16(data);
    advance_width_max = eat_big_endian_u16(data);
    min_left_side_bearing = eat_big_endian_s16(data);
    min_right_side_bearing = eat_big_endian_s16(data);
    x_max_extent = eat_big_endian_s16(data);
    caret_slope_rise = eat_big_endian_s16(data);
    caret_slope_run = eat_big_endian_s16(data);
    caret_offset = eat_big_endian_s16(data);
    eat(data, 4 * sizeof(s16)); // skip reserved
    metric_data_format = eat_big_endian_s16(data);
    num_of_long_hor_metrics = eat_big_endian_u16(data);
}

void Long_Hor_Metric::read(void** data)
{
    advance_width = eat_big_endian_u16(data);
    left_side_bearing = eat_big_endian_s16(data);
}

void Hmtx::read(Arena* arena, void** data, u16 num_glyphs, u16 num_of_long_hor_metrics)
{
    // @Note: maybe just memcpy blocks instead of eating each value.
    
    h_metrics = (Long_Hor_Metric*)arena->push(num_of_long_hor_metrics * sizeof(Long_Hor_Metric));
    for (u16 i = 0; i < num_of_long_hor_metrics; ++i)
    {
        h_metrics[i].read(data);
    }
    
    const u16 num_of_left_side_bearings = num_glyphs - num_of_long_hor_metrics;
    left_side_bearings = (s16*)arena->push(num_of_left_side_bearings * sizeof(s16));

    for (u16 i = 0; i < num_of_left_side_bearings; ++i)
    {
        left_side_bearings[i] = eat_big_endian_s16(data);
    }    
}

void Loca::read(Arena* arena, void** data, u16 num_glyphs, s16 index_to_loc_format)
{
    offsets = (u32*)arena->push((num_glyphs + 1) * sizeof(u32));
    
    for (u16 i = 0; i < num_glyphs + 1; ++i)
    {
        if (index_to_loc_format == 0)
        {
            offsets[i] = eat_big_endian_u16(data) * 2;
        }
        else
        {
            offsets[i] = eat_big_endian_u32(data);       
        }
    }
}

void Cmap_Encoding_Subtable::read(void** data)
{
    platform_id = eat_big_endian_u16(data);
    platform_specific_id = eat_big_endian_u16(data);
    offset = eat_big_endian_u32(data);
}

void Cmap::read(Arena* arena, void** data)
{
    version = eat_big_endian_u16(data);
    number_subtables = eat_big_endian_u16(data);

    subtables = (Cmap_Encoding_Subtable*)arena->push(number_subtables * sizeof(Cmap_Encoding_Subtable));
    for (u16 i = 0; i < number_subtables; ++i)
        subtables[i].read(data);
}

void Cmap::print() const
{
	msg_log("#)\tp_id\tps_id\toffset\ttype");

    for (u16 i = 0; i < number_subtables; ++i)
    {
        Cmap_Encoding_Subtable* cet = subtables + i;

        char* platform_str = (char*)alloca(32);
        switch (cet->platform_id)
        {
            case 0: platform_str = "Unicode"; break;
            case 1: platform_str = "Mac"; break;
            case 2: platform_str = "Not Supported"; break;
            case 3: platform_str = "Microsoft"; break;
        }
        
        msg_log("%u)\t%u\t%u\t%u\t%s", i + 1,
                cet->platform_id,
                cet->platform_specific_id,
                cet->offset,
                platform_str);
	}
}

void Format4::read(Arena* arena, void** data)
{
    void* start = *data;
    
    format = eat_big_endian_u16(data);
    ASSERT(format == 4);
    length = eat_big_endian_u16(data);
    language = eat_big_endian_u16(data);
    seg_count_x2 = eat_big_endian_u16(data);
    search_range = eat_big_endian_u16(data);
    entry_selector = eat_big_endian_u16(data);
    range_shift = eat_big_endian_u16(data);

    const u16 seg_count = seg_count_x2 / 2;
    end_code = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        end_code[i] = eat_big_endian_u16(data);
    }

    eat_u16(data); // skip reserved
    
    start_code = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        start_code[i] = eat_big_endian_u16(data);
    }

    id_delta = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        id_delta[i] = eat_big_endian_u16(data);
    }

    id_range_offset = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        id_range_offset[i] = eat_big_endian_u16(data);
    }
    
    const u32 remaining_bytes = length - (u32)((u8*)*data - (u8*)start);
    const u32 glyph_index_count = remaining_bytes / 2;
    glyph_index_array = (u16*)arena->push(remaining_bytes);
    
    for (u32 i = 0; i < glyph_index_count; ++i)
        glyph_index_array[i] = eat_big_endian_u16(data);
}

void Format4::print() const
{
    const u16 seg_count = seg_count_x2 / 2;

    msg_log("format: %u, length: %u, language: %u, seg_count: %u", format, length, language, seg_count);
    msg_log("search_range: %u, entry_celector: %u, range_shift: %u)", search_range, entry_selector, range_shift);
    msg_log("#)\tstart_code\tend_code\tid_delta\tid_range_offset");

	for (u16 i = 0; i < seg_count; ++i)
    {
	    msg_log("%u)\t%10u\t%8u\t%8u\t%15u", i + 1,
                start_code[i],
                end_code[i],
                id_delta[i],
                id_range_offset[i]);
    }
}

u16 Format4::glyph_index(u32 character) const
{
    const u16 seg_count = seg_count_x2 / 2;
    s32 index = -1;
    
	for (u16 i = 0; i < seg_count; i++)
    {
		if (end_code[i] > character)
        {
            index = i;
            break;
        };
	}
	
	if (index == -1)
        return 0;

	if (start_code[index] < character)
    {
		if (id_range_offset[index] != 0)
        {
			u16* ptr = id_range_offset + index + id_range_offset[index] / 2;
			ptr += character - start_code[index];
            
			if (*ptr == 0)
                return 0;
            
			return *ptr + id_delta[index];
		}
        else
        {
			return character + id_delta[index];
		}
	}

	return 0;
}

void Glyph_Header::read(void** data)
{
    number_of_countours = eat_big_endian_s16(data);
    x_min = eat_big_endian_s16(data);
    y_min = eat_big_endian_s16(data);
    x_max = eat_big_endian_s16(data);
    y_max = eat_big_endian_s16(data);
}

void Simple_Glyph::read(Arena* arena, void** data)
{
    header.read(data);

    end_pts_of_countours = (u16*)arena->push(header.number_of_countours * sizeof(u16));
    for (s16 i = 0; i < header.number_of_countours; ++i)
    {
        end_pts_of_countours[i] = eat_big_endian_u16(data);
    }

    instruction_length = eat_big_endian_u16(data);
    instructions = (u8*)arena->push(instruction_length);
    void* instruction_start = eat(data, instruction_length);
    memcpy(instructions, instruction_start, instruction_length);

    const u16 last_idx = end_pts_of_countours[header.number_of_countours - 1];
    flags = (Glyph_Outline_Flag*)arena->push(last_idx + 1);

    for (u16 i = 0; i <= last_idx; ++i)
    {
        flags[i].val = eat_u8(data);

        if (flags[i].repeat)
        {
            u8 repeat_count = eat_u8(data);
            while (repeat_count > 0)
            {
                i++;
                flags[i] = flags[i - 1];
                repeat_count--;
            }
        }
    }

    x_coordinates = (s16*)arena->push((last_idx + 1) * 2);
    s16 prev_coordinate = 0;
    s16 curr_coordinate = 0;

    for (u16 i = 0; i <= last_idx; ++i)
    {
        const u8 combined_flag = flags[i].x_short << 1 | flags[i].x_same;
        switch (combined_flag)
        {
            case 0: curr_coordinate = eat_big_endian_s16(data); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -eat_u8(data); break;
            case 3: curr_coordinate = eat_u8(data); break;
        }
        
        x_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = x_coordinates[i];
    }

    y_coordinates = (s16*)arena->push((last_idx + 1) * 2);
    prev_coordinate = 0;
    curr_coordinate = 0;

    for (u16 i = 0; i <= last_idx; ++i)
    {
        const u8 combined_flag = flags[i].y_short << 1 | flags[i].y_same;
        switch (combined_flag)
        {
            case 0: curr_coordinate = eat_big_endian_s16(data); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -eat_u8(data); break;
            case 3: curr_coordinate = eat_u8(data); break;
        }
        
        y_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = y_coordinates[i];
    }
}

void Simple_Glyph::print() const
{
    msg_log("#contours\t(x_min, y_min)\t(x_max, y_max)\tinstruction_length");
	msg_log("%9d\t(%d,%d)\t\t(%d,%d)\t%d", header.number_of_countours,
			header.x_min, header.y_min,
			header.x_max, header.y_max,
			instruction_length);

	msg_log("#)\t(  x  ,  y  )");
	const u16 last_idx = end_pts_of_countours[header.number_of_countours - 1];
	for(u16 i = 0; i <= last_idx; ++i)
    {
		msg_log("%d)\t(%5d,%5d)", i + 1, x_coordinates[i], y_coordinates[i]);
	}
}

void Component_Glyph_Part::read(void** data)
{
    flag.val = eat_big_endian_u16(data);
    glyph_index = eat_big_endian_u16(data);

    if (flag.arg_1_and_2_are_words)
    {
        argument1 = eat_big_endian_s16(data);
        argument2 = eat_big_endian_s16(data);
    }
    else
    {
        argument1 = eat_s8(data);
        argument2 = eat_s8(data);
    }

    e = 0.0f;
    f = 0.0f;
    
    if (flag.args_are_xy_values)
    {
        e = (f32)argument1;
        f = (f32)argument2;
    }
    else
    {
        // @Todo: handle points.
        msg_critical("Unhandled entry in (%s:%d)", __FILE__, __LINE__);
    }
            
    a = 1.0f;
    b = 0.0f;
    c = 0.0f;
    d = 1.0f;

    if (flag.we_have_a_scale)
    {
        a = eat_big_endian_f2dot14(data);
        d = a;
    }
    else if (flag.we_have_an_x_and_y_scale)
    {
        a = eat_big_endian_f2dot14(data);
        d = eat_big_endian_f2dot14(data);
    }
    else if (flag.we_have_a_two_by_two)
    {
        a = eat_big_endian_f2dot14(data);
        b = eat_big_endian_f2dot14(data);
        c = eat_big_endian_f2dot14(data);
        d = eat_big_endian_f2dot14(data);
    }
}

void Compound_Glyph::read(Arena* arena, void** data, const Font_Directory* fd)
{
    header.read(data);

    const u32 def_parts_length = 32 * sizeof(Component_Glyph_Part);
    //const u32 def_simple_glyphs_length = 32 * sizeof(Simple_Glyph);
    parts = (Component_Glyph_Part*)arena->push(def_parts_length);
    //simple_glyphs = (Simple_Glyph*)arena->push(def_simple_glyphs_length);

    u32 actual_parts_length = 0;
    //u32 actual_simple_glyphs_length = 0;
    u16 idx = 0;
    
    while (true)
    {
        Component_Glyph_Part* part = parts + idx;
        part->read(data);

        /*
        Simple_Glyph* simple = simple_glyphs + idx;
        *simple = fd->simple_glyph_from_index(arena, part->glyph_index);

        // Transform all simple glyph coordinates.
        // @Todo: move this to other place, here we want to simply read all needed data.
        const u16 last_idx = simple->end_pts_of_countours[simple->header.number_of_countours - 1];
        for(u16 i = 0; i <= last_idx; ++i)
        {
            f32 x = simple->x_coordinates[i];
            f32 y = simple->y_coordinates[i];

            x = part->a * x + part->c * y + part->e;
            y = part->b * x + part->d * y + part->f;
            
            simple->x_coordinates[i] = (s16)x;
            simple->y_coordinates[i] = (s16)y;
        }
        actual_simple_glyphs_length += sizeof(Simple_Glyph);
        */
        
        idx++;
        actual_parts_length += sizeof(Component_Glyph_Part);
        
        if (!part->flag.more_components)
            break;
    }

    // Pop extra memory for component glyph parts pushed earlier.
    // @Todo: there is a hole between parts and simple glyphs memory blocks after pop.
    
    //ASSERT(actual_simple_glyphs_length < def_simple_glyphs_length);
    //arena->pop(def_simple_glyphs_length - actual_simple_glyphs_length);
    
    ASSERT(actual_parts_length < def_parts_length);
    arena->pop(def_parts_length - actual_parts_length);
}

void Font_Directory::read(Arena* arena, void* data)
{
    void* data_start = data;
    
    offset_subtable = (Offset_Subtable*)arena->push(sizeof(Offset_Subtable));
    offset_subtable->read(&data);

    table_directories = (Table_Directory*)arena->push(offset_subtable->num_tables * sizeof(Table_Directory));
    
    // First read all table directories and non-dependable specific ones.
    for (u16 i = 0; i < offset_subtable->num_tables; ++i)
    {
        Table_Directory* t = table_directories + i;
        t->read(&data);
        
        switch (t->tag)
        {
            case HEAD_TAG:
            {
                u8* head_data = (u8*)data_start + t->offset;
                head = (Head*)arena->push(sizeof(Head));
                head->read((void**)&head_data);
                break;
            }

            case MAXP_TAG:
            {
                u8* maxp_data = (u8*)data_start + t->offset;
                maxp = (Maxp*)arena->push(sizeof(Maxp));
                maxp->read((void**)&maxp_data);
                break;
            }

            case HHEA_TAG:
            {
                u8* hhea_data = (u8*)data_start + t->offset;
                hhea = (Hhea*)arena->push(sizeof(Hhea));
                hhea->read((void**)&hhea_data);
                break;
            }
            
            case GLYF_TAG: glyf_start = (u8*)data_start + t->offset; break;
                
            case CMAP_TAG:
            {
                u8* cmap_data = (u8*)data_start + t->offset;
                cmap = (Cmap*)arena->push(sizeof(Cmap));
                cmap->read(arena, (void**)&cmap_data);

                // @Todo: assume 4th format by default for now, support others later.
                u8* format4_data = (u8*)data_start + t->offset + cmap->subtables[0].offset;
                format4 = (Format4*)arena->push(sizeof(Format4));
                format4->read(arena, (void**)&format4_data);
                
                break;
            }
        }
    }

    // Then read dependable specific table directories.
    for (u16 i = 0; i < offset_subtable->num_tables; ++i)
    {
        Table_Directory* t = table_directories + i;

        switch (t->tag)
        {
            case HMTX_TAG:
            {    
                u8* hmtx_data = (u8*)data_start + t->offset;
                hmtx = (Hmtx*)arena->push(sizeof(Hmtx));
                hmtx->read(arena, (void**)&hmtx_data, maxp->num_glyphs, hhea->num_of_long_hor_metrics);
                break;
            }

            case LOCA_TAG:
            {
                u8* loca_data = (u8*)data_start + t->offset;
                loca = (Loca*)arena->push(sizeof(Loca));
                loca->read(arena, (void**)&loca_data, maxp->num_glyphs, head->index_to_loc_format);
                break;
            }
        }
    }
}

void Font_Directory::print() const
{
    msg_log("#)\ttag\tlen\toffset");
	for(u16 i = 0; i < offset_subtable->num_tables; ++i)
    {
		Table_Directory* t = table_directories + i;
		msg_log("%u)\t%c%c%c%c\t%u\t%u", i + 1,
		        t->tag_str[3], t->tag_str[2],
				t->tag_str[1], t->tag_str[0],
				t->length, t->offset);
	}
}

Simple_Glyph Font_Directory::simple_glyph_from_char(Arena* arena, u32 character) const
{
    const u16 glyph_idx = format4->glyph_index(character);
    return simple_glyph_from_index(arena, glyph_idx);
}

Simple_Glyph Font_Directory::simple_glyph_from_index(Arena* arena, u16 glyph_index) const
{
    const u32 offset = loca->offsets[glyph_index];
    u8* glyph_data = glyf_start + offset;
    
    Simple_Glyph glyph;
    glyph.read(arena, (void**)&glyph_data);

    return glyph;
}

Compound_Glyph Font_Directory::compound_glyph_from_char(Arena* arena, u32 character) const
{
    const u16 glyph_idx = format4->glyph_index(character);
    const u32 offset = loca->offsets[glyph_idx];
    u8* glyph_data = glyf_start + offset;

    Compound_Glyph glyph;
    glyph.read(arena, (void**)&glyph_data, this);

    return glyph;
}

extern u8* read_entire_file(Arena* arena, const char* path, u32* size);

void Font_Face::read(Arena* arena, void* data)
{
    dir = (Font_Directory*)arena->push(sizeof(Font_Directory));
    dir->read(arena, data);

    glyphs = (Glyph*)arena->push(dir->maxp->num_glyphs * sizeof(Glyph));
    for (u32 c = 0; c < dir->maxp->num_glyphs; ++c)
    {
        const u16 glyph_index = dir->format4->glyph_index(c);
        u8* glyph_data = dir->glyf_start + dir->loca->offsets[glyph_index];
        const s16 num_contours = *(s16*)(glyph_data);

        if (num_contours > 0)
        {
            read_simple_glyph(arena, c);
        }
        else
        {
            read_compound_glyph(arena, c);
        }
    }
}

void Font_Face::read_simple_glyph(Arena* arena, u32 character)
{
    Glyph* glyph = glyphs + character;

    glyph->character = character;
    glyph->index = dir->format4->glyph_index(character);
    void* data = dir->glyf_start + dir->loca->offsets[glyph->index];    
        
    glyph->header.read(&data);
    glyph->h_metric = dir->hmtx->h_metrics[glyph - glyphs];

    const u32 contours_length = glyph->header.number_of_countours * sizeof(u16);
    glyph->end_pts_of_countours = (u16*)arena->push(contours_length);
    for (s16 i = 0; i < glyph->header.number_of_countours; ++i)
    {
        glyph->end_pts_of_countours[i] = eat_big_endian_u16(&data);
    }

    const u16 instruction_length = eat_big_endian_u16(&data); // skip instruction length
    eat(&data, instruction_length); // skip instructions

    const u16 last_idx = glyph->end_pts_of_countours[glyph->header.number_of_countours - 1];
    const u32 flags_length = last_idx + 1;
    Glyph_Outline_Flag* flags = (Glyph_Outline_Flag*)arena->push(flags_length);

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

    const u32 coordinates_length = (last_idx + 1) * 2;
    glyph->x_coordinates = (s16*)arena->push(coordinates_length);
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
        
        glyph->x_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = glyph->x_coordinates[i];
    }

    glyph->y_coordinates = (s16*)arena->push(coordinates_length);
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
        
        glyph->y_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = glyph->y_coordinates[i];
    }

    arena->pop(flags_length);
}

void Font_Face::read_compound_glyph(Arena* arena, u32 character)
{
    Glyph* glyph = glyphs + character;

    glyph->character = character;
    glyph->index = dir->format4->glyph_index(character);
    void* data = dir->glyf_start + dir->loca->offsets[glyph->index];    

    glyph->header.read(&data);
    glyph->h_metric = dir->hmtx->h_metrics[glyph - glyphs];

    const u32 def_parts_length = 32 * sizeof(Component_Glyph_Part);
    Component_Glyph_Part* parts = (Component_Glyph_Part*)arena->push(def_parts_length);

    u16 idx = 0;
    u32 actual_parts_length = 0;
    
    while (true)
    {
        Component_Glyph_Part* part = parts + idx;
        part->read(&data);

        /*
        Glyph* simple = simple_glyphs + idx;
        *simple = fd->simple_glyph_from_index(arena, part->glyph_index);

        // Transform all simple glyph coordinates.
        const u16 last_idx = simple->end_pts_of_countours[simple->header.number_of_countours - 1];
        for(u16 i = 0; i <= last_idx; ++i)
        {
            f32 x = simple->x_coordinates[i];
            f32 y = simple->y_coordinates[i];

            x = part->a * x + part->c * y + part->e;
            y = part->b * x + part->d * y + part->f;
            
            simple->x_coordinates[i] = (s16)x;
            simple->y_coordinates[i] = (s16)y;
        }
        */
        
        idx++;
        actual_parts_length += sizeof(Component_Glyph_Part);
        
        if (!part->flag.more_components)
            break;
    }

    // Pop extra memory pushed earlier.
    ASSERT(actual_parts_length < def_parts_length);
    arena->pop(def_parts_length - actual_parts_length);
}

Font_Face* create_font_face(Arena* arena, const char* font_path)
{
    u8* font_data = read_entire_file(arena, font_path, nullptr);
    Font_Face* face = (Font_Face*)arena->push(sizeof(Font_Face));
    face->read(arena, font_data);    
    return face;
}
