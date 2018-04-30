/* MIT License (https://opensource.org/licenses/MIT)
 * Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)
 */
   MODULE critical
   SECTION .text : CODE

   PUBLIC Critical_DisallowInterrupts
   THUMB
Critical_DisallowInterrupts
   /* Sets Priority Mask register (preventing all exceptions with configurable priorities) */
   cpsid   i;
   
   /* Branch to the link register (exception return, equiv. to in-line) */
   bx     lr;

   PUBLIC Critical_AllowInterrupts
   THUMB
Critical_AllowInterrupts
   /* Clears Priority Mask register (allowing all exceptions) */
   cpsie   i;

   /* Branch to the link register (exception return, equiv. to in-line) */
   bx     lr;

   END
