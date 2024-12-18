#include "pch.h"
#include "font.h"
#include "file_reader.h"
#include <malloc.h>

// True type font format is stored in big endian byte order,
// so here are some file reader wrappers specifically for font.

static s16 font_eat_s16(File_Reader* fr)
{
    s16 val = fr->eat_s16();

    if (platform_little_endian())
    {
        val = swap_endianness_16(&val);
    }
    
    return val;
}

static u16 font_eat_u16(File_Reader* fr)
{
    u16 val = fr->eat_u16();

    if (platform_little_endian())
    {
        val = swap_endianness_16(&val);
    }
    
    return val;
}

static s32 font_eat_s32(File_Reader* fr)
{
    s32 val = fr->eat_s32();

    if (platform_little_endian())
    {
        val = swap_endianness_32(&val);
    }
    
    return val;
}

static u32 font_eat_u32(File_Reader* fr)
{
    u32 val = fr->eat_u32();

    if (platform_little_endian())
    {
        val = swap_endianness_32(&val);
    }
    
    return val;
}

static u64 font_eat_u64(File_Reader* fr)
{
    u64 val = fr->eat_u64();

    if (platform_little_endian())
    {
        val = swap_endianness_64(&val);
    }
    
    return val;
}

static s64 font_eat_s64(File_Reader* fr)
{
    s64 val = fr->eat_s64();

    if (platform_little_endian())
    {
        val = swap_endianness_64(&val);
    }
    
    return val;
}

static f32 font_eat_f2dot14(File_Reader* fr)
{
    s16 val = fr->eat_s16();
    return val / (f32)(1 << 14);
}

void Offset_Subtable::read(File_Reader* fr)
{
    scaler_type = font_eat_u32(fr);
    num_tables = font_eat_u16(fr);
    search_range = font_eat_u16(fr);
    entry_selector = font_eat_u16(fr);
    range_shift = font_eat_u16(fr);
}

void Table_Directory::read(File_Reader* fr)
{
    tag = font_eat_u32(fr);
    checksum = font_eat_u32(fr);
    offset = font_eat_u32(fr);
    length = font_eat_u32(fr);
}

void Head::read(File_Reader* fr)
{
    version = font_eat_s32(fr);
    font_revision = font_eat_s32(fr);
    checksum_adjustment = font_eat_u32(fr);
    magic_number = font_eat_u32(fr);
    flags = font_eat_u16(fr);
    units_per_em = font_eat_u16(fr);
    created = font_eat_s64(fr);
    modified = font_eat_s64(fr);
    x_min = font_eat_s16(fr);
    y_min = font_eat_s16(fr);
    x_max = font_eat_s16(fr);
    y_max = font_eat_s16(fr);
    mac_style = font_eat_u16(fr);
    lowest_rec_ppem = font_eat_u16(fr);
    font_direction_hint = font_eat_s16(fr);
    index_to_loc_format = font_eat_s16(fr);
    glyph_data_format = font_eat_s16(fr);
}

void Maxp::read(File_Reader* fr)
{
    version = font_eat_s32(fr);
    num_glyphs = font_eat_u16(fr);
    max_points = font_eat_u16(fr);
    max_contours = font_eat_u16(fr);
    max_component_points = font_eat_u16(fr);
    max_component_contours = font_eat_u16(fr);
    max_zones = font_eat_u16(fr);
    max_twilight_points = font_eat_u16(fr);
    max_storage = font_eat_u16(fr);
    max_function_defs = font_eat_u16(fr);
    max_instruction_defs = font_eat_u16(fr);
    max_stack_elements = font_eat_u16(fr);
    max_size_of_instructions = font_eat_u16(fr);
    max_component_elements = font_eat_u16(fr);
    max_component_depth = font_eat_u16(fr);
}

void Hhea::read(File_Reader* fr)
{
    version = font_eat_s32(fr);
    ascent = font_eat_s16(fr);
    descent = font_eat_s16(fr);
    line_gap = font_eat_s16(fr);
    advance_width_max = font_eat_u16(fr);
    min_left_side_bearing = font_eat_s16(fr);
    min_right_side_bearing = font_eat_s16(fr);
    x_max_extent = font_eat_s16(fr);
    caret_slope_rise = font_eat_s16(fr);
    caret_slope_run = font_eat_s16(fr);
    caret_offset = font_eat_s16(fr);
    fr->eat(4 * sizeof(s16)); // skip reserved
    metric_data_format = font_eat_s16(fr);
    num_of_long_hor_metrics = font_eat_u16(fr);
}

void Long_Hor_Metric::read(File_Reader* fr)
{
    advance_width = font_eat_u16(fr);
    left_side_bearing = font_eat_s16(fr);
}

void Hmtx::read(File_Reader* fr, Arena* arena, u16 num_glyphs, u16 num_of_long_hor_metrics)
{
    // @Note: maybe just memcpy blocks instead of eating each value.
    
    h_metrics = (Long_Hor_Metric*)arena->push(num_of_long_hor_metrics * sizeof(Long_Hor_Metric));
    for (u16 i = 0; i < num_of_long_hor_metrics; ++i)
    {
        h_metrics[i].read(fr);
    }
    
    const u16 num_of_left_side_bearings = num_glyphs - num_of_long_hor_metrics;
    left_side_bearings = (s16*)arena->push(num_of_left_side_bearings * sizeof(s16));

    for (u16 i = 0; i < num_of_left_side_bearings; ++i)
    {
        left_side_bearings[i] = font_eat_s16(fr);
    }    
}

void Loca::read(File_Reader* fr, Arena* arena, u16 num_glyphs, s16 index_to_loc_format)
{
    offsets = (u32*)arena->push((num_glyphs + 1) * sizeof(u32));
    
    for (u16 i = 0; i < num_glyphs + 1; ++i)
    {
        if (index_to_loc_format == 0)
        {
            offsets[i] = font_eat_u16(fr) * 2;
        }
        else
        {
            offsets[i] = font_eat_u32(fr);       
        }
    }
}

void Cmap_Encoding_Subtable::read(File_Reader* fr)
{
    platform_id = font_eat_u16(fr);
    platform_specific_id = font_eat_u16(fr);
    offset = font_eat_u32(fr);
}

void Cmap::read(File_Reader* fr, Arena* arena)
{
    version = font_eat_u16(fr);
    number_subtables = font_eat_u16(fr);

    subtables = (Cmap_Encoding_Subtable*)arena->push(number_subtables * sizeof(Cmap_Encoding_Subtable));
    for (u16 i = 0; i < number_subtables; ++i)
        subtables[i].read(fr);
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

void Format4::read(File_Reader* fr, Arena* arena)
{
    u8* start = fr->current;
    
    format = font_eat_u16(fr);
    ASSERT(format == 4);
    length = font_eat_u16(fr);
    language = font_eat_u16(fr);
    seg_count_x2 = font_eat_u16(fr);
    search_range = font_eat_u16(fr);
    entry_selector = font_eat_u16(fr);
    range_shift = font_eat_u16(fr);

    const u16 seg_count = seg_count_x2 / 2;
    end_code = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        end_code[i] = font_eat_u16(fr);
    }

    fr->eat_u16(); // skip reserved
    
    start_code = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        start_code[i] = font_eat_u16(fr);
    }

    id_delta = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        id_delta[i] = font_eat_u16(fr);
    }

    id_range_offset = (u16*)arena->push(seg_count * sizeof(u16));
    for (u16 i = 0; i < seg_count; ++i)
    {
        id_range_offset[i] = font_eat_u16(fr);
    }
    
    const u32 remaining_bytes = length - (u32)(fr->current - start);
    const u32 glyph_index_count = remaining_bytes / 2;
    glyph_index_array = (u16*)arena->push(remaining_bytes);
    
    for (u32 i = 0; i < glyph_index_count; ++i)
        glyph_index_array[i] = font_eat_u16(fr);
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

u16 Format4::glyph_index(u16 code_point) const
{
    const u16 seg_count = seg_count_x2 / 2;
    s32 index = -1;
    
	for (u16 i = 0; i < seg_count; i++)
    {
		if (end_code[i] > code_point)
        {
            index = i;
            break;
        };
	}
	
	if (index == -1)
        return 0;

	if (start_code[index] < code_point)
    {
		if (id_range_offset[index] != 0)
        {
			u16* ptr = id_range_offset + index + id_range_offset[index] / 2;
			ptr += code_point - start_code[index];
            
			if (*ptr == 0)
                return 0;
            
			return *ptr + id_delta[index];
		}
        else
        {
			return code_point + id_delta[index];
		}
	}

	return 0;
}

void Glyph::read(File_Reader* fr)
{
    number_of_countours = font_eat_s16(fr);
    x_min = font_eat_s16(fr);
    y_min = font_eat_s16(fr);
    x_max = font_eat_s16(fr);
    y_max = font_eat_s16(fr);
}

void Simple_Glyph::read(File_Reader* fr, Arena* arena)
{
    glyph.read(fr);

    end_pts_of_countours = (u16*)arena->push(glyph.number_of_countours * sizeof(u16));
    for (s16 i = 0; i < glyph.number_of_countours; ++i)
    {
        end_pts_of_countours[i] = font_eat_u16(fr);
    }

    instruction_length = font_eat_u16(fr);
    instructions = (u8*)arena->push(instruction_length);
    memcpy(instructions, fr->current, instruction_length);
    fr->current += instruction_length;

    const u16 last_idx = end_pts_of_countours[glyph.number_of_countours - 1];
    flags = (Glyph_Outline_Flag*)arena->push(last_idx + 1);

    for (u16 i = 0; i <= last_idx; ++i)
    {
        flags[i].val = fr->eat_u8();

        if (flags[i].repeat)
        {
            u8 repeat_count = fr->eat_u8();
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
            case 0: curr_coordinate = font_eat_s16(fr); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -fr->eat_u8(); break;
            case 3: curr_coordinate = fr->eat_u8(); break;
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
            case 0: curr_coordinate = font_eat_s16(fr); break;
            case 1: curr_coordinate = 0; break;
            case 2: curr_coordinate = -fr->eat_u8(); break;
            case 3: curr_coordinate = fr->eat_u8(); break;
        }
        
        y_coordinates[i] = curr_coordinate + prev_coordinate;
        prev_coordinate = y_coordinates[i];
    }
}

void Simple_Glyph::print() const
{
    msg_log("#contours\t(x_min, y_min)\t(x_max, y_max)\tinstruction_length");
	msg_log("%9d\t(%d,%d)\t\t(%d,%d)\t%d", glyph.number_of_countours,
			glyph.x_min, glyph.y_min,
			glyph.x_max, glyph.y_max,
			instruction_length);

	msg_log("#)\t(  x  ,  y  )");
	const u16 last_idx = end_pts_of_countours[glyph.number_of_countours - 1];
	for(u16 i = 0; i <= last_idx; ++i)
    {
		msg_log("%d)\t(%5d,%5d)", i + 1, x_coordinates[i], y_coordinates[i]);
	}
}

void Component_Glyph_Part::read(File_Reader* fr)
{
    flag.val = font_eat_u16(fr);
    glyph_index = font_eat_u16(fr);

    if (flag.arg_1_and_2_are_words)
    {
        argument1 = font_eat_s16(fr);
        argument2 = font_eat_s16(fr);
    }
    else
    {
        argument1 = fr->eat_s8();
        argument2 = fr->eat_s8();
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
        a = font_eat_f2dot14(fr);
        d = a;
    }
    else if (flag.we_have_an_x_and_y_scale)
    {
        a = font_eat_f2dot14(fr);
        d = font_eat_f2dot14(fr);
    }
    else if (flag.we_have_a_two_by_two)
    {
        a = font_eat_f2dot14(fr);
        b = font_eat_f2dot14(fr);
        c = font_eat_f2dot14(fr);
        d = font_eat_f2dot14(fr);
    }
}

void Compound_Glyph::read(File_Reader* fr, Arena* arena, const Font_Directory* fd)
{
    glyph.read(fr);

    const u32 def_parts_length = 32 * sizeof(Component_Glyph_Part);
    const u32 def_simple_glyphs_length = 32 * sizeof(Simple_Glyph);
    parts = (Component_Glyph_Part*)arena->push(def_parts_length);
    simple_glyphs = (Simple_Glyph*)arena->push(def_simple_glyphs_length);

    u32 actual_parts_length = 0;
    u32 actual_simple_glyphs_length = 0;
    u16 idx = 0;
    
    while (true)
    {
        Component_Glyph_Part* part = parts + idx;
        part->read(fr);

        u8* current = fr->current;
        Simple_Glyph* simple = simple_glyphs + idx;
        *simple = fd->simple_glyph_from_index(fr, arena, part->glyph_index);
        fr->current = current;

        // Transform all simple glyph coordinates.
        // @Todo: move this to other place, here we want to simply read all needed data.
        const u16 last_idx = simple->end_pts_of_countours[simple->glyph.number_of_countours - 1];
        for(u16 i = 0; i <= last_idx; ++i)
        {
            f32 x = simple->x_coordinates[i];
            f32 y = simple->y_coordinates[i];

            x = part->a * x + part->c * y + part->e;
            y = part->b * x + part->d * y + part->f;
            
            simple->x_coordinates[i] = (s16)x;
            simple->y_coordinates[i] = (s16)y;
        }
        
        idx++;
        actual_parts_length += sizeof(Component_Glyph_Part);
        actual_simple_glyphs_length += sizeof(Simple_Glyph);

        if (!part->flag.more_components)
            break;
    }

    // Pop extra memory for component glyph parts pushed earlier.
    // @Todo: there is a hole between parts and simple glyphs memory blocks after pop.
    
    ASSERT(actual_simple_glyphs_length < def_simple_glyphs_length);
    arena->pop(def_simple_glyphs_length - actual_simple_glyphs_length);
    
    ASSERT(actual_parts_length < def_parts_length);
    arena->pop(def_parts_length - actual_parts_length);
}

void Font_Directory::read(File_Reader* fr, Arena* arena)
{
    offset_subtable = (Offset_Subtable*)arena->push(sizeof(Offset_Subtable));
    offset_subtable->read(fr);

    table_directories = (Table_Directory*)arena->push(offset_subtable->num_tables * sizeof(Table_Directory));
    
    // First read all table directories and non-dependable specific ones.
    for (u16 i = 0; i < offset_subtable->num_tables; ++i)
    {
        Table_Directory* t = table_directories + i;
        t->read(fr);

        // Save current position before possible table directory read.
        u8* current = fr->current;
        
        switch (t->tag)
        {
            case HEAD_TAG:
            {
                fr->current = fr->buffer + t->offset;
                head = (Head*)arena->push(sizeof(Head));
                head->read(fr);
                break;
            }

            case MAXP_TAG:
            {
                fr->current = fr->buffer + t->offset;
                maxp = (Maxp*)arena->push(sizeof(Maxp));
                maxp->read(fr);
                break;
            }

            case HHEA_TAG:
            {
                fr->current = fr->buffer + t->offset;
                hhea = (Hhea*)arena->push(sizeof(Hhea));
                hhea->read(fr);
                break;
            }
            
            case GLYF_TAG: glyf_start = fr->buffer + t->offset; break;
                
            case CMAP_TAG:
            {
                fr->current = fr->buffer + t->offset;
                cmap = (Cmap*)arena->push(sizeof(Cmap));
                cmap->read(fr, arena);

                // @Todo: assume 4th format by default for now, support others later.
                fr->current = fr->buffer + t->offset + cmap->subtables[0].offset;
                format4 = (Format4*)arena->push(sizeof(Format4));
                format4->read(fr, arena);
                
                break;
            }
        }

        // Restore initial current position.
        fr->current = current;
    }

    // Then read dependable specific table directories.
    for (u16 i = 0; i < offset_subtable->num_tables; ++i)
    {
        Table_Directory* t = table_directories + i;

        switch (t->tag)
        {
            case HMTX_TAG:
            {    
                fr->current = fr->buffer + t->offset;
                hmtx = (Hmtx*)arena->push(sizeof(Hmtx));
                hmtx->read(fr, arena, maxp->num_glyphs, hhea->num_of_long_hor_metrics);
                break;
            }

            case LOCA_TAG:
            {
                fr->current = fr->buffer + t->offset;
                loca = (Loca*)arena->push(sizeof(Loca));
                loca->read(fr, arena, maxp->num_glyphs, head->index_to_loc_format);
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

Simple_Glyph Font_Directory::simple_glyph_from_char(File_Reader* fr, Arena* arena, u16 code_point) const
{
    const u16 glyph_idx = format4->glyph_index(code_point);
    return simple_glyph_from_index(fr, arena, glyph_idx);
}

Simple_Glyph Font_Directory::simple_glyph_from_index(File_Reader* fr, Arena* arena, u16 glyph_index) const
{
    const u32 offset = loca->offsets[glyph_index];
    fr->current = glyf_start + offset;

    Simple_Glyph glyph;
    glyph.read(fr, arena);

    return glyph;
}

Compound_Glyph Font_Directory::compound_glyph_from_char(File_Reader* fr, Arena* arena, u16 code_point) const
{
    const u16 glyph_idx = format4->glyph_index(code_point);
    const u32 offset = loca->offsets[glyph_idx];
    fr->current = glyf_start + offset;

    Compound_Glyph glyph;
    glyph.read(fr, arena, this);

    return glyph;
}
