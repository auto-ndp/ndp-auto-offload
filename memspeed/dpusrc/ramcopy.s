	.text
	.file	"ramcopy.c"
	.file	1 "/usr/bin/../share/upmem/include/stdlib" "stdint.h"
	.file	2 "/phd/code/memspeed/dpusrc" "./../include/dpuramcopy.h"
	.file	3 "/phd/code/memspeed/dpusrc" "ramcopy.c"
	.section	.text.main,"ax",@progbits
	.globl	main                    // -- Begin function main
	.type	main,@function
main:                                   // @main
.Lfunc_begin0:
	.loc	3 15 0                  // ramcopy.c:15:0
	.cfi_sections .debug_frame
	.cfi_startproc
// %bb.0:
	.cfi_def_cfa_offset 0
	.loc	3 16 22 prologue_end    // ramcopy.c:16:22
	lw r1, zero, program_config
.Ltmp0:
	.loc	3 16 7 is_stmt 0        // ramcopy.c:16:7
	jeq r1, 1, .LBB0_8
// %bb.1:
	.loc	3 0 7                   // ramcopy.c:0:7
	move r0, 1
	.loc	3 16 7                  // ramcopy.c:16:7
	jneq r1, 2, .LBB0_10
// %bb.2:
.Ltmp1:
	.loc	3 17 57 is_stmt 1       // ramcopy.c:17:57
	move r1, program_config
.Ltmp2:
	//DEBUG_VALUE: _rep <- 0
	//DEBUG_VALUE: read_end <- undef
	.loc	3 18 51                 // ramcopy.c:18:51
	lw r0, r1, 8
.Ltmp3:
	.loc	3 18 5 is_stmt 0        // ramcopy.c:18:5
	jeq r0, 0, .LBB0_9
.Ltmp4:
// %bb.3:
	//DEBUG_VALUE: _rep <- 0
	.loc	3 0 0                   // ramcopy.c:0:0
	lw r1, r1, 4
.Ltmp5:
	move r2, id
.Ltmp6:
	.loc	3 18 5                  // ramcopy.c:18:5
	move r3, id4
.Ltmp7:
	.loc	3 0 0                   // ramcopy.c:0:0
	move r4, wram_buffer_a
	lsl_add r4, r4, r1, 2
.Ltmp8:
	//DEBUG_VALUE: read_end <- $r4
	move r5, 0
.Ltmp9:
	move r8, wram_buffer_a
	jump .LBB0_4
.Ltmp10:
.LBB0_7:                                //   in Loop: Header=BB0_4 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	3 19 14 is_stmt 1       // ramcopy.c:19:14
	add r5, r5, 1
.Ltmp11:
	//DEBUG_VALUE: _rep <- $r5
	.loc	3 18 5                  // ramcopy.c:18:5
	jgeu r5, r0, .LBB0_9
.Ltmp12:
.LBB0_4:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_6 Depth 2
	//DEBUG_VALUE: read_end <- $r4
	//DEBUG_VALUE: _rep <- $r5
	.loc	3 22 7                  // ramcopy.c:22:7
	jges r2, r1, .LBB0_7
.Ltmp13:
// %bb.5:                               //   in Loop: Header=BB0_4 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	3 0 7 is_stmt 0         // ramcopy.c:0:7
	move r6, r3
.Ltmp14:
.LBB0_6:                                //   Parent Loop BB0_4 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: read_end <- $r4
	.loc	3 24 24 is_stmt 1       // ramcopy.c:24:24
	lw r7, r6, wram_buffer_a
.Ltmp15:
	//DEBUG_VALUE: read_begin <- undef
	//DEBUG_VALUE: write_begin <- undef
	.loc	3 24 22 is_stmt 0       // ramcopy.c:24:22
	sw r6, wram_buffer_b, r7
.Ltmp16:
	//DEBUG_VALUE: write_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	//DEBUG_VALUE: read_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	.loc	3 22 25 is_stmt 1       // ramcopy.c:22:25
	add r6, r6, 16
	add r7, r8, r6
.Ltmp17:
	.loc	3 22 7 is_stmt 0        // ramcopy.c:22:7
	jltu r7, r4, .LBB0_6
	jump .LBB0_7
.Ltmp18:
.LBB0_8:
	//DEBUG_VALUE: mram_read:nb_of_bytes <- 8
	.file	4 "/usr/bin/../share/upmem/include/syslib" "mram.h"
	.loc	4 35 24 is_stmt 1       // /usr/bin/../share/upmem/include/syslib/mram.h:35:24
	move r0, mram_buffer_a
.Ltmp19:
	//DEBUG_VALUE: mram_read:from <- $r0
	move r1, wram_buffer_a
.Ltmp20:
	//DEBUG_VALUE: mram_read:to <- $r1
	ldma r1, r0, 0
.Ltmp21:
	//DEBUG_VALUE: mram_read:nb_of_bytes <- 8
	.loc	4 35 24 is_stmt 0       // /usr/bin/../share/upmem/include/syslib/mram.h:35:24
	move r0, mram_buffer_b
.Ltmp22:
	//DEBUG_VALUE: mram_read:from <- $r0
	move r1, wram_buffer_b
.Ltmp23:
	//DEBUG_VALUE: mram_read:to <- $r1
	ldma r1, r0, 0
.Ltmp24:
.LBB0_9:
	.loc	4 0 24                  // /usr/bin/../share/upmem/include/syslib/mram.h:0:24
	move r0, 0
.LBB0_10:
	.loc	3 34 1 is_stmt 1        // ramcopy.c:34:1
	jump r23
.Ltmp25:
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
	.file	5 "/usr/bin/../share/upmem/include/syslib" "defs.h"
	.file	6 "/usr/bin/../share/upmem/include/syslib" "sysdef.h"
	.section	.stack_sizes,"o",@progbits,.text.main,unique,0
	.long	.Lfunc_begin0
	.byte	0
	.section	.text.main,"ax",@progbits
                                        // -- End function
	.type	program_config,@object  // @program_config
	.section	.dpu_host,"aw",@progbits
	.globl	program_config
	.p2align	3
program_config:
	.zero	16
	.size	program_config, 16

	.type	wram_buffer_a,@object   // @wram_buffer_a
	.globl	wram_buffer_a
	.p2align	3
wram_buffer_a:
	.zero	24576
	.size	wram_buffer_a, 24576

	.type	wram_buffer_b,@object   // @wram_buffer_b
	.globl	wram_buffer_b
	.p2align	3
wram_buffer_b:
	.zero	24576
	.size	wram_buffer_b, 24576

	.type	mram_buffer_a,@object   // @mram_buffer_a
	.section	.mram.noinit,"aw",@progbits
	.globl	mram_buffer_a
	.p2align	3
mram_buffer_a:
	.zero	33554432
	.size	mram_buffer_a, 33554432

	.type	mram_buffer_b,@object   // @mram_buffer_b
	.globl	mram_buffer_b
	.p2align	3
mram_buffer_b:
	.zero	33554432
	.size	mram_buffer_b, 33554432

	.section	.debug_str,"MS",@progbits,1
.Linfo_string0:
	.asciz	"clang version 10.0.0 (https://github.com/upmem/llvm-project.git aad86822198b21e428d23495764412e4880729e2)" // string offset=0
.Linfo_string1:
	.asciz	"ramcopy.c"             // string offset=106
.Linfo_string2:
	.asciz	"/phd/code/memspeed/dpusrc" // string offset=116
.Linfo_string3:
	.asciz	"program_config"        // string offset=142
.Linfo_string4:
	.asciz	"cfg_copy_mode"         // string offset=157
.Linfo_string5:
	.asciz	"unsigned int"          // string offset=171
.Linfo_string6:
	.asciz	"uint32_t"              // string offset=184
.Linfo_string7:
	.asciz	"cfg_copy_size"         // string offset=193
.Linfo_string8:
	.asciz	"cfg_copy_repetitions"  // string offset=207
.Linfo_string9:
	.asciz	"cfg_cache_size"        // string offset=228
.Linfo_string10:
	.asciz	"dpuramcopy_config_t"   // string offset=243
.Linfo_string11:
	.asciz	"wram_buffer_a"         // string offset=263
.Linfo_string12:
	.asciz	"__ARRAY_SIZE_TYPE__"   // string offset=277
.Linfo_string13:
	.asciz	"wram_buffer_b"         // string offset=297
.Linfo_string14:
	.asciz	"mram_buffer_a"         // string offset=311
.Linfo_string15:
	.asciz	"mram_buffer_b"         // string offset=325
.Linfo_string16:
	.asciz	"me"                    // string offset=339
.Linfo_string17:
	.asciz	"sysname_t"             // string offset=342
.Linfo_string18:
	.asciz	"mram_read"             // string offset=352
.Linfo_string19:
	.asciz	"from"                  // string offset=362
.Linfo_string20:
	.asciz	"to"                    // string offset=367
.Linfo_string21:
	.asciz	"nb_of_bytes"           // string offset=370
.Linfo_string22:
	.asciz	"main"                  // string offset=382
.Linfo_string23:
	.asciz	"int"                   // string offset=387
.Linfo_string24:
	.asciz	"_rep"                  // string offset=391
.Linfo_string25:
	.asciz	"read_end"              // string offset=396
.Linfo_string26:
	.asciz	"read_begin"            // string offset=405
.Linfo_string27:
	.asciz	"write_begin"           // string offset=416
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
	.long	.Ltmp2-.Lfunc_begin0
	.long	.Ltmp10-.Lfunc_begin0
	.short	2                       // Loc expr size
	.byte	48                      // DW_OP_lit0
	.byte	159                     // DW_OP_stack_value
	.long	.Ltmp10-.Lfunc_begin0
	.long	.Ltmp18-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	85                      // DW_OP_reg5
	.long	0
	.long	0
.Ldebug_loc1:
	.long	.Ltmp8-.Lfunc_begin0
	.long	.Ltmp18-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	84                      // DW_OP_reg4
	.long	0
	.long	0
.Ldebug_loc2:
	.long	.Ltmp18-.Lfunc_begin0
	.long	.Ltmp24-.Lfunc_begin0
	.short	2                       // Loc expr size
	.byte	56                      // DW_OP_lit8
	.byte	159                     // DW_OP_stack_value
	.long	0
	.long	0
.Ldebug_loc3:
	.long	.Ltmp19-.Lfunc_begin0
	.long	.Ltmp22-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	80                      // DW_OP_reg0
	.long	0
	.long	0
.Ldebug_loc4:
	.long	.Ltmp20-.Lfunc_begin0
	.long	.Ltmp23-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	81                      // DW_OP_reg1
	.long	0
	.long	0
.Ldebug_loc5:
	.long	.Ltmp21-.Lfunc_begin0
	.long	.Ltmp24-.Lfunc_begin0
	.short	2                       // Loc expr size
	.byte	56                      // DW_OP_lit8
	.byte	159                     // DW_OP_stack_value
	.long	0
	.long	0
.Ldebug_loc6:
	.long	.Ltmp22-.Lfunc_begin0
	.long	.Ltmp24-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	80                      // DW_OP_reg0
	.long	0
	.long	0
.Ldebug_loc7:
	.long	.Ltmp23-.Lfunc_begin0
	.long	.Ltmp24-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	81                      // DW_OP_reg1
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
	.ascii	"\210\001"              // DW_AT_alignment
	.byte	15                      // DW_FORM_udata
	.byte	2                       // DW_AT_location
	.byte	24                      // DW_FORM_exprloc
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	3                       // Abbreviation Code
	.byte	19                      // DW_TAG_structure_type
	.byte	1                       // DW_CHILDREN_yes
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	11                      // DW_AT_byte_size
	.byte	11                      // DW_FORM_data1
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	4                       // Abbreviation Code
	.byte	13                      // DW_TAG_member
	.byte	0                       // DW_CHILDREN_no
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	56                      // DW_AT_data_member_location
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	5                       // Abbreviation Code
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
	.byte	6                       // Abbreviation Code
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
	.byte	7                       // Abbreviation Code
	.byte	1                       // DW_TAG_array_type
	.byte	1                       // DW_CHILDREN_yes
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	8                       // Abbreviation Code
	.byte	33                      // DW_TAG_subrange_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	55                      // DW_AT_count
	.byte	5                       // DW_FORM_data2
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	9                       // Abbreviation Code
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
	.byte	10                      // Abbreviation Code
	.byte	33                      // DW_TAG_subrange_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	55                      // DW_AT_count
	.byte	6                       // DW_FORM_data4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	11                      // Abbreviation Code
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
	.byte	12                      // Abbreviation Code
	.byte	46                      // DW_TAG_subprogram
	.byte	1                       // DW_CHILDREN_yes
	.byte	3                       // DW_AT_name
	.byte	14                      // DW_FORM_strp
	.byte	58                      // DW_AT_decl_file
	.byte	11                      // DW_FORM_data1
	.byte	59                      // DW_AT_decl_line
	.byte	11                      // DW_FORM_data1
	.byte	39                      // DW_AT_prototyped
	.byte	25                      // DW_FORM_flag_present
	.byte	32                      // DW_AT_inline
	.byte	11                      // DW_FORM_data1
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	13                      // Abbreviation Code
	.byte	5                       // DW_TAG_formal_parameter
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
	.byte	14                      // Abbreviation Code
	.byte	15                      // DW_TAG_pointer_type
	.byte	0                       // DW_CHILDREN_no
	.byte	73                      // DW_AT_type
	.byte	19                      // DW_FORM_ref4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	15                      // Abbreviation Code
	.byte	38                      // DW_TAG_const_type
	.byte	0                       // DW_CHILDREN_no
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	16                      // Abbreviation Code
	.byte	15                      // DW_TAG_pointer_type
	.byte	0                       // DW_CHILDREN_no
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	17                      // Abbreviation Code
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
	.byte	18                      // Abbreviation Code
	.byte	11                      // DW_TAG_lexical_block
	.byte	1                       // DW_CHILDREN_yes
	.byte	17                      // DW_AT_low_pc
	.byte	1                       // DW_FORM_addr
	.byte	18                      // DW_AT_high_pc
	.byte	6                       // DW_FORM_data4
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	19                      // Abbreviation Code
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
	.byte	20                      // Abbreviation Code
	.byte	11                      // DW_TAG_lexical_block
	.byte	1                       // DW_CHILDREN_yes
	.byte	85                      // DW_AT_ranges
	.byte	23                      // DW_FORM_sec_offset
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	21                      // Abbreviation Code
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
	.byte	22                      // Abbreviation Code
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
	.byte	23                      // Abbreviation Code
	.byte	29                      // DW_TAG_inlined_subroutine
	.byte	1                       // DW_CHILDREN_yes
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
	.byte	24                      // Abbreviation Code
	.byte	5                       // DW_TAG_formal_parameter
	.byte	0                       // DW_CHILDREN_no
	.byte	2                       // DW_AT_location
	.byte	23                      // DW_FORM_sec_offset
	.byte	49                      // DW_AT_abstract_origin
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
	.byte	1                       // Abbrev [1] 0xb:0x200 DW_TAG_compile_unit
	.long	.Linfo_string0          // DW_AT_producer
	.short	12                      // DW_AT_language
	.long	.Linfo_string1          // DW_AT_name
	.long	.Lline_table_start0     // DW_AT_stmt_list
	.long	.Linfo_string2          // DW_AT_comp_dir
	.long	.Lfunc_begin0           // DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0 // DW_AT_high_pc
	.byte	2                       // Abbrev [2] 0x26:0x12 DW_TAG_variable
	.long	.Linfo_string3          // DW_AT_name
	.long	56                      // DW_AT_type
                                        // DW_AT_external
	.byte	3                       // DW_AT_decl_file
	.byte	7                       // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	program_config
	.byte	3                       // Abbrev [3] 0x38:0x39 DW_TAG_structure_type
	.long	.Linfo_string10         // DW_AT_name
	.byte	16                      // DW_AT_byte_size
	.byte	2                       // DW_AT_decl_file
	.byte	11                      // DW_AT_decl_line
	.byte	4                       // Abbrev [4] 0x40:0xc DW_TAG_member
	.long	.Linfo_string4          // DW_AT_name
	.long	113                     // DW_AT_type
	.byte	2                       // DW_AT_decl_file
	.byte	12                      // DW_AT_decl_line
	.byte	0                       // DW_AT_data_member_location
	.byte	4                       // Abbrev [4] 0x4c:0xc DW_TAG_member
	.long	.Linfo_string7          // DW_AT_name
	.long	113                     // DW_AT_type
	.byte	2                       // DW_AT_decl_file
	.byte	13                      // DW_AT_decl_line
	.byte	4                       // DW_AT_data_member_location
	.byte	4                       // Abbrev [4] 0x58:0xc DW_TAG_member
	.long	.Linfo_string8          // DW_AT_name
	.long	113                     // DW_AT_type
	.byte	2                       // DW_AT_decl_file
	.byte	14                      // DW_AT_decl_line
	.byte	8                       // DW_AT_data_member_location
	.byte	4                       // Abbrev [4] 0x64:0xc DW_TAG_member
	.long	.Linfo_string9          // DW_AT_name
	.long	113                     // DW_AT_type
	.byte	2                       // DW_AT_decl_file
	.byte	15                      // DW_AT_decl_line
	.byte	12                      // DW_AT_data_member_location
	.byte	0                       // End Of Children Mark
	.byte	5                       // Abbrev [5] 0x71:0xb DW_TAG_typedef
	.long	124                     // DW_AT_type
	.long	.Linfo_string6          // DW_AT_name
	.byte	1                       // DW_AT_decl_file
	.byte	48                      // DW_AT_decl_line
	.byte	6                       // Abbrev [6] 0x7c:0x7 DW_TAG_base_type
	.long	.Linfo_string5          // DW_AT_name
	.byte	7                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	2                       // Abbrev [2] 0x83:0x12 DW_TAG_variable
	.long	.Linfo_string11         // DW_AT_name
	.long	149                     // DW_AT_type
                                        // DW_AT_external
	.byte	3                       // DW_AT_decl_file
	.byte	9                       // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	wram_buffer_a
	.byte	7                       // Abbrev [7] 0x95:0xd DW_TAG_array_type
	.long	113                     // DW_AT_type
	.byte	8                       // Abbrev [8] 0x9a:0x7 DW_TAG_subrange_type
	.long	162                     // DW_AT_type
	.short	6144                    // DW_AT_count
	.byte	0                       // End Of Children Mark
	.byte	9                       // Abbrev [9] 0xa2:0x7 DW_TAG_base_type
	.long	.Linfo_string12         // DW_AT_name
	.byte	8                       // DW_AT_byte_size
	.byte	7                       // DW_AT_encoding
	.byte	2                       // Abbrev [2] 0xa9:0x12 DW_TAG_variable
	.long	.Linfo_string13         // DW_AT_name
	.long	149                     // DW_AT_type
                                        // DW_AT_external
	.byte	3                       // DW_AT_decl_file
	.byte	10                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	wram_buffer_b
	.byte	2                       // Abbrev [2] 0xbb:0x12 DW_TAG_variable
	.long	.Linfo_string14         // DW_AT_name
	.long	205                     // DW_AT_type
                                        // DW_AT_external
	.byte	3                       // DW_AT_decl_file
	.byte	12                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	mram_buffer_a
	.byte	7                       // Abbrev [7] 0xcd:0xf DW_TAG_array_type
	.long	113                     // DW_AT_type
	.byte	10                      // Abbrev [10] 0xd2:0x9 DW_TAG_subrange_type
	.long	162                     // DW_AT_type
	.long	8388608                 // DW_AT_count
	.byte	0                       // End Of Children Mark
	.byte	2                       // Abbrev [2] 0xdc:0x12 DW_TAG_variable
	.long	.Linfo_string15         // DW_AT_name
	.long	205                     // DW_AT_type
                                        // DW_AT_external
	.byte	3                       // DW_AT_decl_file
	.byte	13                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	mram_buffer_b
	.byte	11                      // Abbrev [11] 0xee:0xc DW_TAG_subprogram
	.long	.Linfo_string16         // DW_AT_name
	.byte	5                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	250                     // DW_AT_type
	.byte	1                       // DW_AT_inline
	.byte	5                       // Abbrev [5] 0xfa:0xb DW_TAG_typedef
	.long	124                     // DW_AT_type
	.long	.Linfo_string17         // DW_AT_name
	.byte	6                       // DW_AT_decl_file
	.byte	27                      // DW_AT_decl_line
	.byte	12                      // Abbrev [12] 0x105:0x2a DW_TAG_subprogram
	.long	.Linfo_string18         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
                                        // DW_AT_prototyped
	.byte	1                       // DW_AT_inline
	.byte	13                      // Abbrev [13] 0x10d:0xb DW_TAG_formal_parameter
	.long	.Linfo_string19         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	303                     // DW_AT_type
	.byte	13                      // Abbrev [13] 0x118:0xb DW_TAG_formal_parameter
	.long	.Linfo_string20         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	309                     // DW_AT_type
	.byte	13                      // Abbrev [13] 0x123:0xb DW_TAG_formal_parameter
	.long	.Linfo_string21         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	124                     // DW_AT_type
	.byte	0                       // End Of Children Mark
	.byte	14                      // Abbrev [14] 0x12f:0x5 DW_TAG_pointer_type
	.long	308                     // DW_AT_type
	.byte	15                      // Abbrev [15] 0x134:0x1 DW_TAG_const_type
	.byte	16                      // Abbrev [16] 0x135:0x1 DW_TAG_pointer_type
	.byte	17                      // Abbrev [17] 0x136:0xc8 DW_TAG_subprogram
	.long	.Lfunc_begin0           // DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0 // DW_AT_high_pc
	.byte	1                       // DW_AT_frame_base
	.byte	102
                                        // DW_AT_GNU_all_call_sites
	.long	.Linfo_string22         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	15                      // DW_AT_decl_line
	.long	510                     // DW_AT_type
                                        // DW_AT_external
	.byte	18                      // Abbrev [18] 0x14b:0x5a DW_TAG_lexical_block
	.long	.Ltmp1                  // DW_AT_low_pc
	.long	.Ltmp18-.Ltmp1          // DW_AT_high_pc
	.byte	19                      // Abbrev [19] 0x154:0xf DW_TAG_variable
	.long	.Ldebug_loc1            // DW_AT_location
	.long	.Linfo_string25         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	17                      // DW_AT_decl_line
	.long	517                     // DW_AT_type
	.byte	20                      // Abbrev [20] 0x163:0x41 DW_TAG_lexical_block
	.long	.Ldebug_ranges1         // DW_AT_ranges
	.byte	19                      // Abbrev [19] 0x168:0xf DW_TAG_variable
	.long	.Ldebug_loc0            // DW_AT_location
	.long	.Linfo_string24         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	18                      // DW_AT_decl_line
	.long	113                     // DW_AT_type
	.byte	20                      // Abbrev [20] 0x177:0x2c DW_TAG_lexical_block
	.long	.Ldebug_ranges0         // DW_AT_ranges
	.byte	21                      // Abbrev [21] 0x17c:0xb DW_TAG_variable
	.long	.Linfo_string26         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	20                      // DW_AT_decl_line
	.long	517                     // DW_AT_type
	.byte	21                      // Abbrev [21] 0x187:0xb DW_TAG_variable
	.long	.Linfo_string27         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	21                      // DW_AT_decl_line
	.long	517                     // DW_AT_type
	.byte	22                      // Abbrev [22] 0x192:0x10 DW_TAG_inlined_subroutine
	.long	238                     // DW_AT_abstract_origin
	.long	.Ltmp5                  // DW_AT_low_pc
	.long	.Ltmp6-.Ltmp5           // DW_AT_high_pc
	.byte	3                       // DW_AT_call_file
	.byte	20                      // DW_AT_call_line
	.byte	46                      // DW_AT_call_column
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	23                      // Abbrev [23] 0x1a5:0x2c DW_TAG_inlined_subroutine
	.long	261                     // DW_AT_abstract_origin
	.long	.Ltmp18                 // DW_AT_low_pc
	.long	.Ltmp21-.Ltmp18         // DW_AT_high_pc
	.byte	3                       // DW_AT_call_file
	.byte	28                      // DW_AT_call_line
	.byte	5                       // DW_AT_call_column
	.byte	24                      // Abbrev [24] 0x1b5:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc3            // DW_AT_location
	.long	269                     // DW_AT_abstract_origin
	.byte	24                      // Abbrev [24] 0x1be:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc4            // DW_AT_location
	.long	280                     // DW_AT_abstract_origin
	.byte	24                      // Abbrev [24] 0x1c7:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc2            // DW_AT_location
	.long	291                     // DW_AT_abstract_origin
	.byte	0                       // End Of Children Mark
	.byte	23                      // Abbrev [23] 0x1d1:0x2c DW_TAG_inlined_subroutine
	.long	261                     // DW_AT_abstract_origin
	.long	.Ltmp21                 // DW_AT_low_pc
	.long	.Ltmp24-.Ltmp21         // DW_AT_high_pc
	.byte	3                       // DW_AT_call_file
	.byte	29                      // DW_AT_call_line
	.byte	5                       // DW_AT_call_column
	.byte	24                      // Abbrev [24] 0x1e1:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc6            // DW_AT_location
	.long	269                     // DW_AT_abstract_origin
	.byte	24                      // Abbrev [24] 0x1ea:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc7            // DW_AT_location
	.long	280                     // DW_AT_abstract_origin
	.byte	24                      // Abbrev [24] 0x1f3:0x9 DW_TAG_formal_parameter
	.long	.Ldebug_loc5            // DW_AT_location
	.long	291                     // DW_AT_abstract_origin
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	6                       // Abbrev [6] 0x1fe:0x7 DW_TAG_base_type
	.long	.Linfo_string23         // DW_AT_name
	.byte	5                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	14                      // Abbrev [14] 0x205:0x5 DW_TAG_pointer_type
	.long	113                     // DW_AT_type
	.byte	0                       // End Of Children Mark
.Ldebug_info_end0:
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.Ltmp5-.Lfunc_begin0
	.long	.Ltmp6-.Lfunc_begin0
	.long	.Ltmp9-.Lfunc_begin0
	.long	.Ltmp10-.Lfunc_begin0
	.long	.Ltmp12-.Lfunc_begin0
	.long	.Ltmp18-.Lfunc_begin0
	.long	0
	.long	0
.Ldebug_ranges1:
	.long	.Ltmp2-.Lfunc_begin0
	.long	.Ltmp4-.Lfunc_begin0
	.long	.Ltmp5-.Lfunc_begin0
	.long	.Ltmp7-.Lfunc_begin0
	.long	.Ltmp9-.Lfunc_begin0
	.long	.Ltmp18-.Lfunc_begin0
	.long	0
	.long	0
	.addrsig
	.addrsig_sym program_config
	.addrsig_sym wram_buffer_a
	.addrsig_sym wram_buffer_b
	.addrsig_sym mram_buffer_a
	.addrsig_sym mram_buffer_b
	.section	.debug_line,"",@progbits
.Lline_table_start0:
