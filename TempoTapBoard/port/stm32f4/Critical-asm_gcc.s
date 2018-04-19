.syntax unified
.section .text

.global Critical_DisallowInterrupts
.thumb_func
Critical_DisallowInterrupts:
   /* Sets Priority Mask register (preventing all exceptions with configurable priorities) */
   cpsid     i
   
   /* Branch to the link register (exception return, equiv. to in-line) */
   bx      lr

      
.global Critical_AllowInterrupts
.thumb_func
Critical_AllowInterrupts:
   /* Clears Priority Mask register (allowing all exceptions) */
   cpsie     i

   /* Branch to the link register (exception return, equiv. to in-line) */
   bx      lr
