/*
  Native File Dialog

  Internal, common across platforms

  http://www.frogtoss.com/labs

  This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
 */


#ifndef _NFD_COMMON_H
#define _NFD_COMMON_H

#define NFD_MAX_STRLEN 256
#define _NFD_UNUSED(x) ((void)x) 

void *NFDi_Malloc( size_t bytes );
void  NFDi_Free( void *ptr );
void  NFDi_SetError( const char *msg );
void  NFDi_SafeStrncpy( char *dst, const char *src, size_t maxCopy );

#endif
