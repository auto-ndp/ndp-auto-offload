	.text
	.file	"wramcopy.c"
	.file	1 "/usr/bin/../share/upmem/include/stdlib" "stdint.h"
	.file	2 "/phd/code/memspeed/dpusrc" "wramcopy.c"
	.section	.text.main,"ax",@progbits
	.globl	main                    // -- Begin function main
	.type	main,@function
main:                                   // @main
.Lfunc_begin0:
	.loc	2 14 0                  // wramcopy.c:14:0
	.cfi_sections .debug_frame
	.cfi_startproc
// %bb.0:
	.cfi_def_cfa_offset 0
	//DEBUG_VALUE: main:read_end <- undef
	//DEBUG_VALUE: _rep <- 0
	.loc	2 16 34 prologue_end    // wramcopy.c:16:34
	lw r0, zero, run_repetitions
.Ltmp0:
	.loc	2 16 3 is_stmt 0        // wramcopy.c:16:3
	jeq r0, 0, .LBB0_6
.Ltmp1:
// %bb.1:
	//DEBUG_VALUE: _rep <- 0
	.loc	2 0 0                   // wramcopy.c:0:0
	lw r1, zero, copy_words_amount
.Ltmp2:
	move r2, id
.Ltmp3:
	.loc	2 16 3                  // wramcopy.c:16:3
	move r3, id4
.Ltmp4:
	.loc	2 0 0                   // wramcopy.c:0:0
	move r4, buffer_a
	lsl_add r4, r4, r1, 2
.Ltmp5:
	//DEBUG_VALUE: main:read_end <- $r4
	move r5, 0
.Ltmp6:
	move r8, buffer_a
	jump .LBB0_2
.Ltmp7:
.LBB0_5:                                //   in Loop: Header=BB0_2 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: main:read_end <- $r4
	.loc	2 17 13 is_stmt 1       // wramcopy.c:17:13
	add r5, r5, 1
.Ltmp8:
	//DEBUG_VALUE: _rep <- $r5
	.loc	2 16 3                  // wramcopy.c:16:3
	jgeu r5, r0, .LBB0_6
.Ltmp9:
.LBB0_2:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_4 Depth 2
	//DEBUG_VALUE: main:read_end <- $r4
	//DEBUG_VALUE: _rep <- $r5
	.loc	2 20 5                  // wramcopy.c:20:5
	jges r2, r1, .LBB0_5
.Ltmp10:
// %bb.3:                               //   in Loop: Header=BB0_2 Depth=1
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: main:read_end <- $r4
	.loc	2 0 5 is_stmt 0         // wramcopy.c:0:5
	move r6, r3
.Ltmp11:
.LBB0_4:                                //   Parent Loop BB0_2 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	//DEBUG_VALUE: _rep <- $r5
	//DEBUG_VALUE: main:read_end <- $r4
	.loc	2 22 22 is_stmt 1       // wramcopy.c:22:22
	lw r7, r6, buffer_a
.Ltmp12:
	//DEBUG_VALUE: read_begin <- undef
	//DEBUG_VALUE: write_begin <- undef
	.loc	2 22 20 is_stmt 0       // wramcopy.c:22:20
	sw r6, buffer_b, r7
.Ltmp13:
	//DEBUG_VALUE: write_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	//DEBUG_VALUE: read_begin <- [DW_OP_plus_uconst 16, DW_OP_stack_value] undef
	.loc	2 20 23 is_stmt 1       // wramcopy.c:20:23
	add r6, r6, 16
	add r7, r8, r6
.Ltmp14:
	.loc	2 20 5 is_stmt 0        // wramcopy.c:20:5
	jltu r7, r4, .LBB0_4
	jump .LBB0_5
.Ltmp15:
.LBB0_6:
	.loc	2 0 5                   // wramcopy.c:0:5
	move r0, 0
	.loc	2 25 3 is_stmt 1        // wramcopy.c:25:3
	jump r23
.Ltmp16:
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
	.type	buffer_a,@object        // @buffer_a
	.section	.dpu_host,"aw",@progbits
	.globl	buffer_a
	.p2align	3
buffer_a:
	.zero	24576
	.size	buffer_a, 24576

	.type	copy_words_amount,@object // @copy_words_amount
	.globl	copy_words_amount
	.p2align	3
copy_words_amount:
	.long	0                       // 0x0
	.size	copy_words_amount, 4

	.type	run_repetitions,@object // @run_repetitions
	.globl	run_repetitions
	.p2align	3
run_repetitions:
	.long	0                       // 0x0
	.size	run_repetitions, 4

	.type	buffer_b,@object        // @buffer_b
	.globl	buffer_b
	.p2align	3
buffer_b:
	.zero	24576
	.size	buffer_b, 24576

	.type	cycles,@object          // @cycles
	.globl	cycles
	.p2align	3
cycles:
	.quad	0                       // 0x0
	.size	cycles, 8

	.section	.debug_str,"MS",@progbits,1
.Linfo_string0:
	.asciz	"clang version 10.0.0 (https://github.com/upmem/llvm-project.git aad86822198b21e428d23495764412e4880729e2)" // string offset=0
.Linfo_string1:
	.asciz	"wramcopy.c"            // string offset=106
.Linfo_string2:
	.asciz	"/phd/code/memspeed/dpusrc" // string offset=117
.Linfo_string3:
	.asciz	"cycles"                // string offset=143
.Linfo_string4:
	.asciz	"long unsigned int"     // string offset=150
.Linfo_string5:
	.asciz	"uint64_t"              // string offset=168
.Linfo_string6:
	.asciz	"run_repetitions"       // string offset=177
.Linfo_string7:
	.asciz	"unsigned int"          // string offset=193
.Linfo_string8:
	.asciz	"uint32_t"              // string offset=206
.Linfo_string9:
	.asciz	"copy_words_amount"     // string offset=215
.Linfo_string10:
	.asciz	"buffer_a"              // string offset=233
.Linfo_string11:
	.asciz	"__ARRAY_SIZE_TYPE__"   // string offset=242
.Linfo_string12:
	.asciz	"buffer_b"              // string offset=262
.Linfo_string13:
	.asciz	"me"                    // string offset=271
.Linfo_string14:
	.asciz	"sysname_t"             // string offset=274
.Linfo_string15:
	.asciz	"main"                  // string offset=284
.Linfo_string16:
	.asciz	"int"                   // string offset=289
.Linfo_string17:
	.asciz	"read_end"              // string offset=293
.Linfo_string18:
	.asciz	"_rep"                  // string offset=302
.Linfo_string19:
	.asciz	"read_begin"            // string offset=307
.Linfo_string20:
	.asciz	"write_begin"           // string offset=318
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
	.long	.Ltmp5-.Lfunc_begin0
	.long	.Ltmp15-.Lfunc_begin0
	.short	1                       // Loc expr size
	.byte	84                      // DW_OP_reg4
	.long	0
	.long	0
.Ldebug_loc1:
	.long	.Lfunc_begin0-.Lfunc_begin0
	.long	.Ltmp7-.Lfunc_begin0
	.short	2                       // Loc expr size
	.byte	48                      // DW_OP_lit0
	.byte	159                     // DW_OP_stack_value
	.long	.Ltmp7-.Lfunc_begin0
	.long	.Ltmp15-.Lfunc_begin0
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
	.ascii	"\210\001"              // DW_AT_alignment
	.byte	15                      // DW_FORM_udata
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
	.byte	5                       // DW_FORM_data2
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
	.byte	9                       // Abbreviation Code
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
	.byte	10                      // Abbreviation Code
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
	.byte	11                      // Abbreviation Code
	.byte	11                      // DW_TAG_lexical_block
	.byte	1                       // DW_CHILDREN_yes
	.byte	85                      // DW_AT_ranges
	.byte	23                      // DW_FORM_sec_offset
	.byte	0                       // EOM(1)
	.byte	0                       // EOM(2)
	.byte	12                      // Abbreviation Code
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
	.byte	13                      // Abbreviation Code
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
	.byte	14                      // Abbreviation Code
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
	.byte	1                       // Abbrev [1] 0xb:0x137 DW_TAG_compile_unit
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
	.byte	2                       // DW_AT_decl_file
	.byte	7                       // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	cycles
	.byte	3                       // Abbrev [3] 0x38:0xb DW_TAG_typedef
	.long	67                      // DW_AT_type
	.long	.Linfo_string5          // DW_AT_name
	.byte	1                       // DW_AT_decl_file
	.byte	53                      // DW_AT_decl_line
	.byte	4                       // Abbrev [4] 0x43:0x7 DW_TAG_base_type
	.long	.Linfo_string4          // DW_AT_name
	.byte	7                       // DW_AT_encoding
	.byte	8                       // DW_AT_byte_size
	.byte	2                       // Abbrev [2] 0x4a:0x12 DW_TAG_variable
	.long	.Linfo_string6          // DW_AT_name
	.long	92                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	9                       // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	run_repetitions
	.byte	3                       // Abbrev [3] 0x5c:0xb DW_TAG_typedef
	.long	103                     // DW_AT_type
	.long	.Linfo_string8          // DW_AT_name
	.byte	1                       // DW_AT_decl_file
	.byte	48                      // DW_AT_decl_line
	.byte	4                       // Abbrev [4] 0x67:0x7 DW_TAG_base_type
	.long	.Linfo_string7          // DW_AT_name
	.byte	7                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	2                       // Abbrev [2] 0x6e:0x12 DW_TAG_variable
	.long	.Linfo_string9          // DW_AT_name
	.long	92                      // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	10                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	copy_words_amount
	.byte	2                       // Abbrev [2] 0x80:0x12 DW_TAG_variable
	.long	.Linfo_string10         // DW_AT_name
	.long	146                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	11                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	buffer_a
	.byte	5                       // Abbrev [5] 0x92:0xd DW_TAG_array_type
	.long	92                      // DW_AT_type
	.byte	6                       // Abbrev [6] 0x97:0x7 DW_TAG_subrange_type
	.long	159                     // DW_AT_type
	.short	6144                    // DW_AT_count
	.byte	0                       // End Of Children Mark
	.byte	7                       // Abbrev [7] 0x9f:0x7 DW_TAG_base_type
	.long	.Linfo_string11         // DW_AT_name
	.byte	8                       // DW_AT_byte_size
	.byte	7                       // DW_AT_encoding
	.byte	2                       // Abbrev [2] 0xa6:0x12 DW_TAG_variable
	.long	.Linfo_string12         // DW_AT_name
	.long	146                     // DW_AT_type
                                        // DW_AT_external
	.byte	2                       // DW_AT_decl_file
	.byte	12                      // DW_AT_decl_line
	.byte	8                       // DW_AT_alignment
	.byte	5                       // DW_AT_location
	.byte	3
	.long	buffer_b
	.byte	8                       // Abbrev [8] 0xb8:0xc DW_TAG_subprogram
	.long	.Linfo_string13         // DW_AT_name
	.byte	3                       // DW_AT_decl_file
	.byte	33                      // DW_AT_decl_line
	.long	196                     // DW_AT_type
	.byte	1                       // DW_AT_inline
	.byte	3                       // Abbrev [3] 0xc4:0xb DW_TAG_typedef
	.long	103                     // DW_AT_type
	.long	.Linfo_string14         // DW_AT_name
	.byte	4                       // DW_AT_decl_file
	.byte	27                      // DW_AT_decl_line
	.byte	9                       // Abbrev [9] 0xcf:0x66 DW_TAG_subprogram
	.long	.Lfunc_begin0           // DW_AT_low_pc
	.long	.Lfunc_end0-.Lfunc_begin0 // DW_AT_high_pc
	.byte	1                       // DW_AT_frame_base
	.byte	102
                                        // DW_AT_GNU_all_call_sites
	.long	.Linfo_string15         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	14                      // DW_AT_decl_line
	.long	309                     // DW_AT_type
                                        // DW_AT_external
	.byte	10                      // Abbrev [10] 0xe4:0xf DW_TAG_variable
	.long	.Ldebug_loc0            // DW_AT_location
	.long	.Linfo_string17         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	15                      // DW_AT_decl_line
	.long	316                     // DW_AT_type
	.byte	11                      // Abbrev [11] 0xf3:0x41 DW_TAG_lexical_block
	.long	.Ldebug_ranges1         // DW_AT_ranges
	.byte	10                      // Abbrev [10] 0xf8:0xf DW_TAG_variable
	.long	.Ldebug_loc1            // DW_AT_location
	.long	.Linfo_string18         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	16                      // DW_AT_decl_line
	.long	92                      // DW_AT_type
	.byte	11                      // Abbrev [11] 0x107:0x2c DW_TAG_lexical_block
	.long	.Ldebug_ranges0         // DW_AT_ranges
	.byte	12                      // Abbrev [12] 0x10c:0xb DW_TAG_variable
	.long	.Linfo_string19         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	18                      // DW_AT_decl_line
	.long	316                     // DW_AT_type
	.byte	12                      // Abbrev [12] 0x117:0xb DW_TAG_variable
	.long	.Linfo_string20         // DW_AT_name
	.byte	2                       // DW_AT_decl_file
	.byte	19                      // DW_AT_decl_line
	.long	316                     // DW_AT_type
	.byte	13                      // Abbrev [13] 0x122:0x10 DW_TAG_inlined_subroutine
	.long	184                     // DW_AT_abstract_origin
	.long	.Ltmp2                  // DW_AT_low_pc
	.long	.Ltmp3-.Ltmp2           // DW_AT_high_pc
	.byte	2                       // DW_AT_call_file
	.byte	18                      // DW_AT_call_line
	.byte	39                      // DW_AT_call_column
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	0                       // End Of Children Mark
	.byte	4                       // Abbrev [4] 0x135:0x7 DW_TAG_base_type
	.long	.Linfo_string16         // DW_AT_name
	.byte	5                       // DW_AT_encoding
	.byte	4                       // DW_AT_byte_size
	.byte	14                      // Abbrev [14] 0x13c:0x5 DW_TAG_pointer_type
	.long	92                      // DW_AT_type
	.byte	0                       // End Of Children Mark
.Ldebug_info_end0:
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.Ltmp2-.Lfunc_begin0
	.long	.Ltmp3-.Lfunc_begin0
	.long	.Ltmp6-.Lfunc_begin0
	.long	.Ltmp7-.Lfunc_begin0
	.long	.Ltmp9-.Lfunc_begin0
	.long	.Ltmp15-.Lfunc_begin0
	.long	0
	.long	0
.Ldebug_ranges1:
	.long	.Lfunc_begin0-.Lfunc_begin0
	.long	.Ltmp1-.Lfunc_begin0
	.long	.Ltmp2-.Lfunc_begin0
	.long	.Ltmp4-.Lfunc_begin0
	.long	.Ltmp6-.Lfunc_begin0
	.long	.Ltmp15-.Lfunc_begin0
	.long	0
	.long	0
	.addrsig
	.addrsig_sym buffer_a
	.addrsig_sym copy_words_amount
	.addrsig_sym run_repetitions
	.addrsig_sym buffer_b
	.addrsig_sym cycles
	.section	.debug_line,"",@progbits
.Lline_table_start0:
