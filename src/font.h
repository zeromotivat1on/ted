#pragma once

inline constexpr u32 font_tag(char a, char b, char c, char d)
{
    return a << 24 | b << 16 | c << 8 | d << 0;
}

inline constexpr u32 GLYF_TAG = font_tag('g', 'l', 'y', 'f');
inline constexpr u32 LOCA_TAG = font_tag('l', 'o', 'c', 'a');
inline constexpr u32 HEAD_TAG = font_tag('h', 'e', 'a', 'd');
inline constexpr u32 CMAP_TAG = font_tag('c', 'm', 'a', 'p');

struct File_Reader;

struct Offset_Subtable
{
    u32 scaler_type;
    u16 num_tables;
    u16 search_range;
    u16 entry_selector;
    u16 range_shift;

    void read(File_Reader* fr);
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

    void read(File_Reader* fr);
};

struct Head
{
    s32 version; // fixed
    s32 font_revision; // fixed
    u32 checksum_adjustment;
    u32 magic_number;
    u16 flags;
    u16 units_per_em;
    s64 created; // seconds since 12:00 midnight, January 1, 1904.
    s64 modified; // seconds since 12:00 midnight, January 1, 1904.
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
    u16 mac_style;
    u16 lowest_rec_ppem; // smallest readable size in pixels
    s16 font_direction_hint;
    s16 index_to_loc_format;
    s16 glyph_data_format;

    void read(File_Reader* fr);
};

struct Cmap_Encoding_Subtable
{
    u16 platform_id;
    u16 platform_specific_id;
    u32 offset;

    void read(File_Reader* fr);
};

struct Cmap
{
    u16 version;
    u16 number_subtables;
    Cmap_Encoding_Subtable* subtables;

    void read(Arena* arena, File_Reader* fr);
    void print() const;
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
    u16 reserved_pad;
	u16* start_code;
	u16* id_delta;
	u16* id_range_offset;
	u16* glyph_index_array;

    void read(Arena* arena, File_Reader* fr);
    void print() const;
};

using Glyph_Flag = union
{
    u8 flag;

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

struct Glyph_Outline
{
    s16 number_of_countours;
    s16 x_min;
    s16 y_min;
    s16 x_max;
    s16 y_max;
    u16* end_pts_of_countours;
    u16 instruction_length;
    u8* instructions;
    Glyph_Flag* flags;
    s16* x_coordinates;
    s16* y_coordinates;

    void read(Arena* arena, File_Reader* fr);
    void print() const;
};

struct Font_Directory
{
    Offset_Subtable offset_subtable;
    Table_Directory* table_directories;
    Head* head;
    Cmap* cmap;
    Format4* format4;
    u8* glyf_start;
    u8* loca_start;
    u8* head_start;
    
    void read(Arena* arena, File_Reader* fr);
    void print() const;
    u16 glyph_index(u16 code_point) const;
    u32 glyph_offset(u16 glyph_index) const;
    Glyph_Outline glyph_outline(Arena* arena, File_Reader* fr, u16 code_point) const;
};
