	.text
	.allow_optimizations
	.file	"indexed_andc_mask.ll"
	.globl	Neg0x3                          # -- Begin function Neg0x3
	.p2align	2
	.type	Neg0x3,@function
Neg0x3:                                 # @Neg0x3
	.cfi_startproc
# %bb.0:                                # %entry
	add $m11, $m11, -8
	.cfi_def_cfa_offset 8
	.cfi_offset $m10, -4
	st32 $m10, $m11, $m15, 1                # 4-byte Folded Spill
	andc $m2, $m2, 3
	ldb16 $a0, $m0, $m15, $m2
	mov	$m0, $m1
	call $m10, __st16f
	ld32 $m10, $m11, $m15, 1                # 4-byte Folded Reload
	add $m11, $m11, 8
	.cfi_def_cfa_offset 0
	br $m10
.Lfunc_end0:
	.size	Neg0x3, .Lfunc_end0-Neg0x3
	.cfi_endproc
                                        # -- End function
	.globl	Neg0x1000                       # -- Begin function Neg0x1000
	.p2align	2
	.type	Neg0x1000,@function
Neg0x1000:                              # @Neg0x1000
	.cfi_startproc
# %bb.0:                                # %entry
	add $m11, $m11, -8
	.cfi_def_cfa_offset 8
	.cfi_offset $m10, -4
	st32 $m10, $m11, $m15, 1                # 4-byte Folded Spill
	setzi $m3, 4096
	andc $m2, $m2, $m3
	ldb16 $a0, $m0, $m15, $m2
	mov	$m0, $m1
	call $m10, __st16f
	ld32 $m10, $m11, $m15, 1                # 4-byte Folded Reload
	add $m11, $m11, 8
	.cfi_def_cfa_offset 0
	br $m10
.Lfunc_end1:
	.size	Neg0x1000, .Lfunc_end1-Neg0x1000
	.cfi_endproc
                                        # -- End function
	.globl	Neg0x200000                     # -- Begin function Neg0x200000
	.p2align	2
	.type	Neg0x200000,@function
Neg0x200000:                            # @Neg0x200000
	.cfi_startproc
# %bb.0:                                # %entry
	add $m11, $m11, -8
	.cfi_def_cfa_offset 8
	.cfi_offset $m10, -4
	st32 $m10, $m11, $m15, 1                # 4-byte Folded Spill
	setzi $m3, 1048575
	or $m3, $m3, 2144337920
	and $m2, $m2, $m3
	ldb16 $a0, $m0, $m15, $m2
	mov	$m0, $m1
	call $m10, __st16f
	ld32 $m10, $m11, $m15, 1                # 4-byte Folded Reload
	add $m11, $m11, 8
	.cfi_def_cfa_offset 0
	br $m10
.Lfunc_end2:
	.size	Neg0x200000, .Lfunc_end2-Neg0x200000
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
