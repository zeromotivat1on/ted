#include "pch.h"
#include "font.h"
#include "file_reader.h"
#include <malloc.h>

// True type font format is stored in big endian byte order,
// so here are some file reader wrapper specifically for font.

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

void Cmap_Encoding_Subtable::read(File_Reader* fr)
{
    platform_id = font_eat_u16(fr);
    platform_specific_id = font_eat_u16(fr);
    offset = font_eat_u32(fr);
}

void Cmap::read(Arena* arena, File_Reader* fr)
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

void Format4::read(Arena* arena, File_Reader* fr)
{    
    format = font_eat_u16(fr);
    length = font_eat_u16(fr);
    language = font_eat_u16(fr);
    seg_count_x2 = font_eat_u16(fr);
    search_range = font_eat_u16(fr);
    entry_selector = font_eat_u16(fr);
    range_shift = font_eat_u16(fr);

    const u16 seg_count = seg_count_x2 / 2;
    
    end_code = (u16*)arena->push(length - 8 * sizeof(u16));
    start_code = end_code + seg_count;
    id_delta = start_code + seg_count;
    id_range_offset = id_delta + seg_count;
    glyph_index_array = id_range_offset + seg_count;

    u8* end_code_start = fr->current;
    u8* start_code_start = end_code_start + seg_count_x2 * 1 + sizeof(reserved_pad);
    u8* id_delta_start = end_code_start + seg_count_x2 * 2 + sizeof(reserved_pad);
    u8* id_range_start = end_code_start + seg_count_x2 * 3 + sizeof(reserved_pad);

    for (u16 i = 0; i < seg_count; ++i)
    {
        end_code[i] = swap_endianness_16(end_code_start + i * 2);
        start_code[i] = swap_endianness_16(start_code_start + i * 2);
        id_delta[i] = swap_endianness_16(id_delta_start + i * 2);
        id_range_offset[i] = swap_endianness_16(id_range_start + i * 2);
    }

    fr->current += seg_count_x2 * 4 + 2;

    const u32 remaining_bytes = fr->size - (u32)(fr->current - fr->buffer);
    const u32 glyph_index_count = remaining_bytes / 2;

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

void Glyph_Outline::read(Arena* arena, File_Reader* fr)
{    
    number_of_countours = font_eat_s16(fr);
    x_min = font_eat_s16(fr);
    y_min = font_eat_s16(fr);
    x_max = font_eat_s16(fr);
    y_max = font_eat_s16(fr);

    end_pts_of_countours = (u16*)arena->push(number_of_countours * sizeof(u16));
    for (s16 i = 0; i < number_of_countours; ++i)
    {
        end_pts_of_countours[i] = font_eat_u16(fr);
    }

    instruction_length = font_eat_u16(fr);
    instructions = (u8*)arena->push(instruction_length);
    memcpy(instructions, fr->current, instruction_length);
    fr->current += instruction_length;

    const u16 last_idx = end_pts_of_countours[number_of_countours - 1];
    flags = (Glyph_Flag*)arena->push(last_idx + 1);

    for (u16 i = 0; i <= last_idx; ++i)
    {
        flags[i].flag = fr->eat_u8();

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

void Glyph_Outline::print() const
{
    msg_log("#contours\t(x_min, y_min)\t(x_max, y_max)\tinstruction_length");
	msg_log("%9d\t(%d,%d)\t\t(%d,%d)\t%d", number_of_countours,
			x_min, y_min,
			x_max, y_max,
			instruction_length);

	msg_log("#)\t(  x  ,  y  )");
	const u16 last_idx = end_pts_of_countours[number_of_countours - 1];
	for(u16 i = 0; i <= last_idx; ++i)
    {
		msg_log("%d)\t(%5d,%5d)", i, x_coordinates[i], y_coordinates[i]);
	}
}

void Font_Directory::read(Arena* arena, File_Reader* fr)
{
    offset_subtable.read(fr);

    const u16 num_tables = offset_subtable.num_tables;
    table_directories = (Table_Directory*)arena->push(num_tables * sizeof(Table_Directory));
    
    for (u16 i = 0; i < num_tables; ++i)
    {
        Table_Directory* t = table_directories + i;
        t->read(fr);

        switch (t->tag)
        {
            case GLYF_TAG: glyf_start = fr->buffer + t->offset; break;
            case LOCA_TAG: loca_start = fr->buffer + t->offset; break;
            case HEAD_TAG: head_start = fr->buffer + t->offset; break;
            case CMAP_TAG:
            {
                File_Reader fr_temp = *fr;

                fr_temp.current = fr_temp.buffer + t->offset;
                cmap = (Cmap*)arena->push(sizeof(Cmap));
                cmap->read(arena, &fr_temp);

                // TODO: Assume 4th format by default for now, support others later.
                fr_temp.current = fr_temp.buffer + t->offset + cmap->subtables[0].offset;
                format4 = (Format4*)arena->push(sizeof(Format4));
                format4->read(arena, &fr_temp);
                
                break;
            }
        }
    }
}

void Font_Directory::print() const
{
    msg_log("#)\ttag\tlen\toffset");
	for(u16 i = 0; i < offset_subtable.num_tables; ++i)
    {
		Table_Directory* t = table_directories + i;
		msg_log("%u)\t%c%c%c%c\t%u\t%u", i + 1,
		        t->tag_str[3], t->tag_str[2],
				t->tag_str[1], t->tag_str[0],
				t->length, t->offset);
	}
}

s16 Font_Directory::loca_type() const
{
    return *(s16*)(head_start + 50);
}

u16 Font_Directory::glyph_index(u16 code_point) const
{
    const u16 seg_count = format4->seg_count_x2 / 2;
    s32 index = -1;
    
	for (u16 i = 0; i < seg_count; i++)
    {
		if (format4->end_code[i] > code_point)
        {
            index = i;
            break;
        };
	}
	
	if (index == -1)
        return 0;

	if (format4->start_code[index] < code_point)
    {
		if (format4->id_range_offset[index] != 0)
        {
			u16* ptr = format4->id_range_offset + index + format4->id_range_offset[index] / 2;
			ptr += code_point - format4->start_code[index];
            
			if (*ptr == 0)
                return 0;
            
			return *ptr + format4->id_delta[index];
		}
        else
        {
			return code_point + format4->id_delta[index];
		}
	}

	return 0;
}

u32 Font_Directory::glyph_offset(u16 glyph_index) const
{
    if (loca_type() == 0)
    {
        // short offsets
        return swap_endianness_16((u16*)loca_start + glyph_index) * 2;
    }
    else
    {
        // long offsets
        return swap_endianness_32((u32*)loca_start + glyph_index);
    }
}

Glyph_Outline Font_Directory::glyph_outline(Arena* arena, File_Reader* fr, u16 glyph_index) const
{
    const u32 offset = glyph_offset(glyph_index);
    fr->current = glyf_start + offset;

    Glyph_Outline glyph;
    glyph.read(arena, fr);

    return glyph;
}
