	.text
	.file	"ramcopy.c"
	.file	1 "/usr/bin/../share/upmem/include/stdlib" "stdint.h"
	.file	2 "/phd/code/memspeed/dpusrc" "ramcopy.c"
	.section	.text.main,"ax",@progbits
	.globl	main                    // -- Begin function main
	.type	main,@function
main:                                   // @main
.Lfunc_begin0:
	.loc	2 18 0                  // ramcopy.c:18:0
	.cfi_sections .debug_frame
	.cfi_startproc
// %bb.0:
	.cfi_def_cfa_offset 0
	.loc	2 19 7 prologue_end     // ramcopy.c:19:7
	lw r1, zero, cfg_copy_mode
.Ltmp0:
	.loc	2 19 7 is_stmt 0        // ramcopy.c:19:7
	jeq r1, 1, .LBB0_8
// %bb.1:
	.loc	2 0 7                   // ramcopy.c:0:7
	move r0, 1
	.loc	2 19 7                  // ramcopy.c:19:7
	jneq r1, 2, .LBB0_9
// %bb.2:
.Ltmp1:
	//DEBUG_VALUE: read_end <- undef
	//DEBUG_VALUE: _rep <- 0
	.loc	2 21 36 is_stmt 1       // ramcopy.c:21:36
	lw r0, zero, cfg_copy_repetitions
.Ltmp2:
	.loc	2 21 5 is_stmt 0        // ramcopy.c:21:5
	jeq r0, 0, .LBB0_8
.Ltmp3:
// %bb.3:
	//DEBUG_VALUE: _rep <- 0
	.loc	2 0 0                   // ramcopy.c:0:0
	lw r1, zero, cfg_copy_size
.Ltmp4:
	move r2, id
.Ltmp5:
	.loc	2 21 5                  // ramcopy.c:21:5
	move r3, id4
.Ltmp6:
	.loc	2 0 0                   // ramcopy.c:0:0
	move r4, wram_buffer_a
	lsl_add r4, r4, r1, 2
.Ltmp7:
	//DEBUG_VALUE: read_end <- $r4
	move r5, 0
.Ltmp8:
	move r8, wram_buffer_a
	jump .LBB0_4
.Ltmp9:
.LBB0_7:                                //   in Loop: Header=BB0_4 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	2 21 62                 // ramcopy.c:21:62
	add r5, r5, 1
.Ltmp10:
	//DEBUG_VALUE: _rep <- $r5
	.loc	2 21 5                  // ramcopy.c:21:5
	jgeu r5, r0, .LBB0_8
.Ltmp11:
.LBB0_4:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_6 Depth 2
	//DEBUG_VALUE: read_end <- $r4
	//DEBUG_VALUE: _rep <- $r5
	.loc	2 24 7 is_stmt 1        // ramcopy.c:24:7
	jges r2, r1, .LBB0_7
.Ltmp12:
// %bb.5:                               //   in Loop: Header=BB0_4 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	2 0 7 is_stmt 0         // ramcopy.c:0:7
	move r6, r3
.Ltmp13:
.LBB0_6:                                //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	2 25 24 is_stmt 1       // ramcopy.c:25:24
	lw r7, r6, wram_buffer_a
.Ltmp14:
	//DEBUG_VALUE: read_begin <- undef
	//DEBUG_VALUE: write_begin <- undef
	.loc	2 25 22 is_stmt 0       // ramcopy.c:25:22
	sw r6, wram_buffer_b, r7
.Ltmp15:
	//DEBUG_VALUE: write_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	//DEBUG_VALUE: read_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	.loc	2 24 24 is_stmt 1       // ramcopy.c:24:24
	add r6, r6, 16
	add r7, r8, r6
.Ltmp16:
	.loc	2 24 7 is_stmt 0        // ramcopy.c:24:7
	jltu r7, r4, .LBB0_6
	jump .LBB0_7
.Ltmp17:
.LBB0_8:
	.loc	2 0 7                   // ramcopy.c:0:7
	move r0, 0
.LBB0_9:
	.loc	2 34 1 is_stmt 1        // ramcopy.c:34:1
	jump r23
.Ltmp18:
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
	.file	3 "/usr/bin/../share/upmem/include/syslib" "defs.h"
	.file	4 "/usr/bin/../share/upmem/include/syslib" "sysdef.h"
	.section	.stack_sizes,"o",@progbits,.text.main,unique,0
	.long	.Lfunc_begin0
	.byte	0
	.section	.text.main,"ax",@progbits
                                        // -- End function
	.type	cfg_copy_mode,@object   // @cfg_copy_mode
	.section	.data.cfg_copy_mode,"aw",@progbits
	.globl	cfg_copy_mode
	.p2align	2
cfg_copy_mode:
	.long	1                       // 0x1
	.size	cfg_copy_mode, 4

	.type	cfg_copy_size,@object   // @cfg_copy_size
	.section	.data.cfg_copy_size,"aw",@progbits
	.globl	cfg_copy_size
	.p2align	2
cfg_copy_size:
	.long	1                       // 0x1
	.size	cfg_copy_size, 4

	.type	cfg_copy_repetitions,@object // @cfg_copy_repetitions
	.section	.data.cfg_copy_repetitions,"aw",@progbits
	.globl	cfg_copy_repetitions
	.p2align	2
cfg_copy_repetitions:
	.long	1                       // 0x1
	.size	cfg_copy_repetitions, 4

	.type	cfg_cache_size,@object  // @cfg_cache_size
	.section	.data.cfg_cache_size,"aw",@progbits
	.globl	cfg_cache_size
	.p2align	2
cfg_cache_size:
	.long	1                       // 0x1
	.size	cfg_cache_size, 4

	.type	wram_buffer_a,@object   // @wram_buffer_a
	.comm	wram_buffer_a,512,4
	.type	wram_buffer_b,@object   // @wram_buffer_b
	.comm	wram_buffer_b,512,4
	.type	mram_buffer_a,@object   // @mram_buffer_a
	.section	.mram.noinit,"aw",@progbits
	.globl	mram_buffer_a
	.p2align	3
mram_buffer_a:
	.zero	4096
	.size	mram_buffer_a, 4096

	.type	mram_buffer_b,@object   // @mram_buffer_b
	.globl	mram_buffer_b
	.p2align	3
mram_buffer_b:
	.zero	4096
	.size	mram_buffer_b, 4096

	.section	.debug_str,"MS",@progbits,1
.Linfo_string0:
	.asciz	"clang version 10.0.0 (https://github.com/upmem/llvm-project.git aad86822198b21e428d23495764412e4880729e2)" // string offset=0
.Linfo_string1:
	.asciz	"ramcopy.c"             // string offset=106
.Linfo_string2:
	.asciz	"/phd/code/memspeed/dpusrc" // string offset=116
.Linfo_string3:
	.asciz	"cfg_copy_mode"         // string offset=142
.Linfo_string4:
	.asciz	"unsigned int"          // string offset=156
.Linfo_string5:
	.asciz	"uint32_t"              // string offset=169
.Linfo_string6:
	.asciz	"cfg_copy_size"         // string offset=178
.Linfo_string7:
	.asciz	"cfg_copy_repetitions"  // string offset=192
.Linfo_string8:
	.asciz	"cfg_cache_size"        // string offset=213
.Linfo_string9:
	.asciz	"wram_buffer_a"         // string offset=228
.Linfo_string10:
	.asciz	"__ARRAY_SIZE_TYPE__"   // string offset=242
.Linfo_string11:
	.asciz	"wram_buffer_b"         // string offset=262
.Linfo_string12:
	.asciz	"mram_buffer_a"         // string offset=276
.Linfo_string13:
	.asciz	"mram_buffer_b"         // string offset=290
.Linfo_string14:
	.asciz	"me"                    // string offset=304
.Linfo_string15:
	.asciz	"sysname_t"             // string offset=307
.Linfo_string16:
	.asciz	"main"                  // string offset=317
.Linfo_string17:
	.asciz	"int"                   // string offset=322
.Linfo_string18:
	.asciz	"read_end"              // string offset=326
.Linfo_string19:
	.asciz	"_rep"                  // string offset=335
.Linfo_string20:
	.asciz	"read_begin"            // string offset=340
.Linfo_string21:
	.asciz	"write_begin"           // string offset=351
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
	.long	.Ltmp7-.Lfunc_begin0
	.long	.Ltmp17-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	84                      // DW_OP_reg4
	.long	0
	.long	0
.Ldebug_loc1:
	.long	.Ltmp1-.Lfunc_begin0
	.long	.Ltmp9-.Lfunc_begin0
	.short	2                       // Loc expr size
	.byte	48                      // DW_OP_lit0
	.byte	159                     // DW_OP_stack_value
	.long	.Ltmp9-.Lfunc_begin0
	.long	.Ltmp17-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	85                      // DW_OP_reg5
	.long	0
	.long	0
	.section	.debug_abbrev,"",@progbits
	.byte	1                       // Abbreviation Code
	.byte	17                      // DW_TAG_compile_unit
	.byte	1                       // DW_CHILDREN_yes
	.byte	37                      // DW_AT_producer
	.byte	14                      // DW_FORM_strp
	.byte	19                      // DW_AT_language
	.byte	5                       // DW_FORM_data2
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	16                      // DW_AT_stmt_list
	.byte	23                      // DW_FORM_sec_offset
	.byte	27                      // DW_AT_comp_dir
	.byte	14                      // DW_FORM_strp
	.byte	17                      // DW_AT_low_pc
	.byte	1                       // DW_FORM_addr
	.byte	18                      // DW_AT_high_pc
	.byte	6                       // DW_FORM_data4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	2                       // Abbreviation Code
	.byte	52                      // DW_TAG_variable
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	63                      // DW_AT_external
	.byte	25                      // DW_FORM_flag_present
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	2                       // DW_AT_location
	.byte	24                      // DW_FORM_exprloc
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	3                       // Abbreviation Code
	.byte	22                      // DW_TAG_typedef
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	4                       // Abbreviation Code
	.byte	36                      // DW_TAG_base_type
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	62                      // DW_AT_encoding
	.byte	11                      // DW_FORM_data1
	.byte	11                      // DW_AT_byte_size
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	5                       // Abbreviation Code
	.byte	1                       // DW_TAG_array_type
	.byte	1                       // DW_CHILDREN_yes
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	6                       // Abbreviation Code
	.byte	33                      // DW_TAG_subrange_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	55                      // DW_AT_count
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	7                       // Abbreviation Code
	.byte	36                      // DW_TAG_base_type
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	11                      // DW_AT_byte_size
	.byte	11                      // DW_FORM_data1
	.byte	62                      // DW_AT_encoding
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	8                       // Abbreviation Code
	.byte	52                      // DW_TAG_variable
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	63                      // DW_AT_external
	.byte	25                      // DW_FORM_flag_present
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.ascii	"\210\001"              // DW_AT_alignment
	.byte	15                      // DW_FORM_udata
	.byte	2                       // DW_AT_location
	.byte	24                      // DW_FORM_exprloc
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	9                       // Abbreviation Code
	.byte	33                      // DW_TAG_subrange_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	55                      // DW_AT_count
	.byte	5                       // DW_FORM_data2
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	10                      // Abbreviation Code
	.byte	46                      // DW_TAG_subprogram
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	32                      // DW_AT_inline
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	11                      // Abbreviation Code
	.byte	46                      // DW_TAG_subprogram
	.byte	1                       // DW_CHILDREN_yes
	.byte	17                      // DW_AT_low_pc
	.byte	1                       // DW_FORM_addr
	.byte	18                      // DW_AT_high_pc
	.byte	6                       // DW_FORM_data4
	.byte	64                      // DW_AT_frame_base
	.byte	24                      // DW_FORM_exprloc
	.ascii	"\227B"                 // DW_AT_GNU_all_call_sites
	.byte	25                      // DW_FORM_flag_present
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	63                      // DW_AT_external
	.byte	25                      // DW_FORM_flag_present
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	12                      // Abbreviation Code
	.byte	11                      // DW_TAG_lexical_block
	.byte	1                       // DW_CHILDREN_yes
	.byte	17                      // DW_AT_low_pc
	.byte	1                       // DW_FORM_addr
	.byte	18                      // DW_AT_high_pc
	.byte	6                       // DW_FORM_data4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	13                      // Abbreviation Code
	.byte	52                      // DW_TAG_variable
	.byte	0                       // DW_CHILDREN_no
	.byte	2                       // DW_AT_location
	.byte	23                      // DW_FORM_sec_offset
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	14                      // Abbreviation Code
	.byte	11                      // DW_TAG_lexical_block
	.byte	1                       // DW_CHILDREN_yes
	.byte	85                      // DW_AT_ranges
	.byte	23                      // DW_FORM_sec_offset
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	15                      // Abbreviation Code
	.byte	52                      // DW_TAG_variable
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	16                      // Abbreviation Code
	.byte	29                      // DW_TAG_inlined_subroutine
	.byte	0                       // DW_CHILDREN_no
	.byte	49                      // DW_AT_abstract_origin
	.byte	19                      // DW_FORM_ref4
	.byte	17                      // DW_AT_low_pc
	.byte	1                       // DW_FORM_addr
	.byte	18                      // DW_AT_high_pc
	.byte	6                       // DW_FORM_data4
	.byte	88                      // DW_AT_call_file
	.byte	11                      // DW_FORM_data1
	.byte	89                      // DW_AT_call_line
	.byte	11                      // DW_FORM_data1
	.byte	87                      // DW_AT_call_column
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	17                      // Abbreviation Code
	.byte	15                      // DW_TAG_pointer_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	0                       // EOM(3)
	.section	.debug_info,"",@progbits
.Lcu_begin0:
	.long	.Ldebug_info_end0-.Ldebug_info_start0 // Length of Unit
.Ldebug_info_start0:
	.short	4                       // DWARF version number
	.long	.debug_abbrev           // Offset Into Abbrev. Section
	.byte	4                       // Address Size (in bytes)
	.byte	1                       // Abbrev [1] 0xb:0x16b DW_TAG_compile_unit
	.long	.Linfo_string0          // DW_AT_producer
	.short	12                      // DW_AT_language
	.long	.Linfo_string1          // DW_AT_name
	.long	.Lline_table_start0     // DW_AT_stmt_list
	.long	.Linfo_string2          // DW_AT_comp_dir
	.long	.Lfunc_begin0           // DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0 // DW_AT_high_pc
	.byte	2                       // Abbrev [2] 0x26:0x11 DW_TAG_variable
	.long	.Linfo_string3          // DW_AT_name
	.long	55                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	7                       // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	cfg_copy_mode
	.byte	3                       // Abbrev [3] 0x37:0xb DW_TAG_typedef
	.long	66                      // DW_AT_type
	.long	.Linfo_string5          // DW_AT_name
	.byte	1                       // DW_AT_decl_file
	.byte	48                      // DW_AT_decl_line
	.byte	4                       // Abbrev [4] 0x42:0x7 DW_TAG_base_type
	.long	.Linfo_string4          // DW_AT_name
	.byte	7                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	2                       // Abbrev [2] 0x49:0x11 DW_TAG_variable
	.long	.Linfo_string6          // DW_AT_name
	.long	55                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	8                       // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	cfg_copy_size
	.byte	2                       // Abbrev [2] 0x5a:0x11 DW_TAG_variable
	.long	.Linfo_string7          // DW_AT_name
	.long	55                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	9                       // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	cfg_copy_repetitions
	.byte	2                       // Abbrev [2] 0x6b:0x11 DW_TAG_variable
	.long	.Linfo_string8          // DW_AT_name
	.long	55                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	10                      // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	cfg_cache_size
	.byte	2                       // Abbrev [2] 0x7c:0x11 DW_TAG_variable
	.long	.Linfo_string9          // DW_AT_name
	.long	141                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	12                      // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	wram_buffer_a
	.byte	5                       // Abbrev [5] 0x8d:0xc DW_TAG_array_type
	.long	55                      // DW_AT_type
	.byte	6                       // Abbrev [6] 0x92:0x6 DW_TAG_subrange_type
	.long	153                     // DW_AT_type
	.byte	128                     // DW_AT_count
	.byte	0                       // End Of Children Mark
	.byte	7                       // Abbrev [7] 0x99:0x7 DW_TAG_base_type
	.long	.Linfo_string10         // DW_AT_name
	.byte	8                       // DW_AT_byte_size
	.byte	7                       // DW_AT_encoding
	.byte	2                       // Abbrev [2] 0xa0:0x11 DW_TAG_variable
	.long	.Linfo_string11         // DW_AT_name
	.long	141                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	13                      // DW_AT_decl_line
	.byte	5                       // DW_AT_location
	.byte	3
	.long	wram_buffer_b
	.byte	8                       // Abbrev [8] 0xb1:0x12 DW_TAG_variable
	.long	.Linfo_string12         // DW_AT_name
	.long	195                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	15                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	mram_buffer_a
	.byte	5                       // Abbrev [5] 0xc3:0xd DW_TAG_array_type
	.long	55                      // DW_AT_type
	.byte	9                       // Abbrev [9] 0xc8:0x7 DW_TAG_subrange_type
	.long	153                     // DW_AT_type
	.short	1024                    // DW_AT_count
	.byte	0                       // End Of Children Mark
	.byte	8                       // Abbrev [8] 0xd0:0x12 DW_TAG_variable
	.long	.Linfo_string13         // DW_AT_name
	.long	195                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	16                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	mram_buffer_b
	.byte	10                      // Abbrev [10] 0xe2:0xc DW_TAG_subprogram
	.long	.Linfo_string14         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	238                     // DW_AT_type
	.byte	1                       // DW_AT_inline
	.byte	3                       // Abbrev [3] 0xee:0xb DW_TAG_typedef
	.long	66                      // DW_AT_type
	.long	.Linfo_string15         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	27                      // DW_AT_decl_line
	.byte	11                      // Abbrev [11] 0xf9:0x70 DW_TAG_subprogram
	.long	.Lfunc_begin0           // DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0 // DW_AT_high_pc
	.byte	1                       // DW_AT_frame_base
	.byte	102
                                        // DW_AT_GNU_all_call_sites
	.long	.Linfo_string16         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	18                      // DW_AT_decl_line
	.long	361                     // DW_AT_type
                                        // DW_AT_external
	.byte	12                      // Abbrev [12] 0x10e:0x5a DW_TAG_lexical_block
	.long	.Ltmp1                  // DW_AT_low_pc
	.long	.Ltmp17-.Ltmp1          // DW_AT_high_pc
	.byte	13                      // Abbrev [13] 0x117:0xf DW_TAG_variable
	.long	.Ldebug_loc0            // DW_AT_location
	.long	.Linfo_string18         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	20                      // DW_AT_decl_line
	.long	368                     // DW_AT_type
	.byte	14                      // Abbrev [14] 0x126:0x41 DW_TAG_lexical_block
	.long	.Ldebug_ranges1         // DW_AT_ranges
	.byte	13                      // Abbrev [13] 0x12b:0xf DW_TAG_variable
	.long	.Ldebug_loc1            // DW_AT_location
	.long	.Linfo_string19         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	21                      // DW_AT_decl_line
	.long	55                      // DW_AT_type
	.byte	14                      // Abbrev [14] 0x13a:0x2c DW_TAG_lexical_block
	.long	.Ldebug_ranges0         // DW_AT_ranges
	.byte	15                      // Abbrev [15] 0x13f:0xb DW_TAG_variable
	.long	.Linfo_string20         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	22                      // DW_AT_decl_line
	.long	368                     // DW_AT_type
	.byte	15                      // Abbrev [15] 0x14a:0xb DW_TAG_variable
	.long	.Linfo_string21         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	23                      // DW_AT_decl_line
	.long	368                     // DW_AT_type
	.byte	16                      // Abbrev [16] 0x155:0x10 DW_TAG_inlined_subroutine
	.long	226                     // DW_AT_abstract_origin
	.long	.Ltmp4                  // DW_AT_low_pc
	.long	.Ltmp5-.Ltmp4           // DW_AT_high_pc
	.byte	2                       // DW_AT_call_file
	.byte	22                      // DW_AT_call_line
	.byte	46                      // DW_AT_call_column
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	4                       // Abbrev [4] 0x169:0x7 DW_TAG_base_type
	.long	.Linfo_string17         // DW_AT_name
	.byte	5                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	17                      // Abbrev [17] 0x170:0x5 DW_TAG_pointer_type
	.long	55                      // DW_AT_type
	.byte	0                       // End Of Children Mark
.Ldebug_info_end0:
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.Ltmp4-.Lfunc_begin0
	.long	.Ltmp5-.Lfunc_begin0
	.long	.Ltmp8-.Lfunc_begin0
	.long	.Ltmp9-.Lfunc_begin0
	.long	.Ltmp11-.Lfunc_begin0
	.long	.Ltmp17-.Lfunc_begin0
	.long	0
	.long	0
.Ldebug_ranges1:
	.long	.Ltmp1-.Lfunc_begin0
	.long	.Ltmp3-.Lfunc_begin0
	.long	.Ltmp4-.Lfunc_begin0
	.long	.Ltmp6-.Lfunc_begin0
	.long	.Ltmp8-.Lfunc_begin0
	.long	.Ltmp17-.Lfunc_begin0
	.long	0
	.long	0
	.addrsig
	.addrsig_sym wram_buffer_a
	.addrsig_sym mram_buffer_a
	.addrsig_sym mram_buffer_b
	.section	.debug_line,"",@progbits
.Lline_table_start0:
