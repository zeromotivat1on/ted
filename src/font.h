#pragma once

inline constexpr u32 font_tag(char a, char b, char c, char d)
{
    return a << 24 | b << 16 | c << 8 | d << 0;
}

inline constexpr u32 HEAD_TAG = font_tag('h', 'e', 'a', 'd');
inline constexpr u32 MAXP_TAG = font_tag('m', 'a', 'x', 'p');
inline constexpr u32 HHEA_TAG = font_tag('h', 'h', 'e', 'a');
inline constexpr u32 HMTX_TAG = font_tag('h', 'm', 't', 'x');
inline constexpr u32 LOCA_TAG = font_tag('l', 'o', 'c', 'a');
inline constexpr u32 CMAP_TAG = font_tag('c', 'm', 'a', 'p');
inline constexpr u32 GLYF_TAG = font_tag('g', 'l', 'y', 'f');

struct File_Reader;

struct Offset_Subtable
{
    u32 scaler_type;
    u16 num_tables;
    u16 search_range;
    u16 entry_selector;
    u16 range_shift;
};

struct Table_Directory
{
    union
    {
        u32     tag;
        char    tag_str[4];
    };

    u32 checksum;
    u32 offset;
    u32 length;
};

struct Head
{
    s32 version; // fixed
    s32 font_revision; // fixed
    u32 checksum_adjustment;
    u32 magic_number;
    u16 flags;
    u16 units_per_em;
    s64 created;  // seconds since 12:00 midnight, January 1, 1904
    s64 modified; // seconds since 12:00 midnight, January 1, 1904
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
    u16 mac_style;
    u16 lowest_rec_ppem; // smallest readable size in pixels
    s16 font_direction_hint;
    s16 index_to_loc_format;
    s16 glyph_data_format;
};

struct Maxp
{
    s32 version; // fixed
    u16 num_glyphs;
    u16 max_points;   // for simple glyph
    u16 max_contours; // for simple glyph
    u16 max_component_points;   // for compound glyph
    u16 max_component_contours; // for compound glyph
    u16 max_zones; // should be 2
    u16 max_twilight_points;
    u16 max_storage;
    u16 max_function_defs;
    u16 max_instruction_defs;
    u16 max_stack_elements;
    u16 max_size_of_instructions;
    u16 max_component_elements;
    u16 max_component_depth;
};

struct Hhea
{
    s32 version; // fixed
    s16 ascent;
    s16 descent;
    s16 line_gap;
    u16 advance_width_max;
    s16 min_left_side_bearing;
    s16 min_right_side_bearing;
    s16 x_max_extent;
    s16 caret_slope_rise;
    s16 caret_slope_run;
    s16 caret_offset;
    // 4 x s16 - reserved
    s16 metric_data_format;
    u16 num_of_long_hor_metrics;
};

struct Long_Hor_Metric
{
    u16 advance_width;
    s16 left_side_bearing;
};

struct Hmtx
{
    Long_Hor_Metric* h_metrics;
    s16* left_side_bearings;
};

struct Loca
{
    u32* offsets;
};

struct Glyf
{
    u8* data;
};

struct Cmap_Encoding_Subtable
{
    u16 platform_id;
    u16 platform_specific_id;
    u32 offset;
};

struct Cmap
{
    u16 version;
    u16 number_subtables;
    Cmap_Encoding_Subtable* subtables;
};

struct Format4
{
	u16 format;
 	u16 length;
 	u16 language;
 	u16 seg_count_x2;
 	u16 search_range;
 	u16 entry_selector;
 	u16 range_shift;
	u16* end_code;
    // u16 - reserved
	u16* start_code;
	u16* id_delta;
	u16* id_range_offset;
	u16* glyph_index_array;
};

struct Format12_Group
{
    u32 start_char_code;
    u32 end_char_code;
    u32 start_glyph_code;
};
    
struct Format12
{
    u16 format;
    // u16 - reserved
    u32 length;
    u32 language;
    u32 n_groups;
    Format12_Group* groups;
};

using Glyph_Outline_Flag = union
{
    u8 val;

    struct
    {
        u8 on_curve : 1;
        u8 x_short : 1;
        u8 y_short : 1;
        u8 repeat : 1;
        u8 x_same : 1;
        u8 y_same : 1;
        u8 reserved_1 : 1;
        u8 reserved_2 : 1;
    };
};

struct Font_Directory
{
    Offset_Subtable offset_subtable;
    Table_Directory* table_directories;
    Head head;
    Maxp maxp;
    Hhea hhea;
    Hmtx hmtx;
    Loca loca;
    Cmap cmap;
    Glyf glyf;
    u16 format_type;

    union
    {
        Format4*  f4;
        Format12* f12;
    };
};

using Component_Glyph_Flag = union
{
    u16 val;

    struct
    {
        u16 arg_1_and_2_are_words : 1;
        u16 args_are_xy_values : 1;
        u16 round_xy_to_grid : 1;
        u16 we_have_a_scale : 1;
        u16 obsolete : 1;
        u16 more_components : 1;
        u16 we_have_an_x_and_y_scale : 1;
        u16 we_have_a_two_by_two : 1;
        u16 we_have_instructions : 1;
        u16 use_my_metrics : 1;
        u16 overlap_compound : 1;
    };
};

// @Note: maybe not the best way to store parsed data for (e, f)
// as they are derived from (argument1, argument2) when args_are_xy_values is set.
struct Component_Glyph
{
    Component_Glyph_Flag flag;
    u16 glyph_index;
    s16 argument1;
    s16 argument2;
    f32 a; // mat2x3[0, 0] scale/rotation x
    f32 b; // mat2x3[0, 1] shear x
    f32 c; // mat2x3[1, 0] shear y
    f32 d; // mat2x3[1, 1] scale/rotation y
    f32 e; // mat2x3[0, 2] translation x
    f32 f; // mat2x3[1, 2] translation y
};

struct Glyph_Slot
{
    s16 number_of_countours;
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
    u16* end_pts_of_countours;
    s16* x_coordinates;
    s16* y_coordinates;
};

inline s16 get_glyph_width(const Glyph_Slot* glyph)
{
    return glyph->x_max - glyph->x_min;
}

inline s16 get_glyph_height(const Glyph_Slot* glyph)
{
    return glyph->y_max - glyph->y_min;
}

inline u16 get_point_count(const Glyph_Slot* glyph)
{
    return glyph->end_pts_of_countours[glyph->number_of_countours - 1] + 1;
}

struct Font_Face
{
    Font_Directory* dir;
    Glyph_Slot* glyph;
};

struct Arena;

void init_font_face(Arena* arena, const char* font_path, Font_Face* face);
u16 get_glyph_index(Font_Face* face, u32 character);
bool load_glyph(Font_Face* face, u16 glyph_index);
bool load_char(Font_Face* face, u32 character);

void print_font_directory(Font_Directory* dir);
void print_cmap(Cmap* cmap);
void print_format4(Format4* f4);
void print_glyph_data(Glyph_Slot* glyph);
