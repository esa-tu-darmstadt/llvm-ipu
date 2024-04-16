	.text
	.allow_optimizations
	.file	"allow_asm_optimizations.ll"
	.globl	func                            # -- Begin function func
	.p2align	2
	.type	func,@function
func:                                   # @func
	.cfi_startproc
# %bb.0:
	br $m10
.Lfunc_end0:
	.size	func, .Lfunc_end0-func
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
