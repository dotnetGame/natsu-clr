// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/***************************************************************************/
/*                                OpInfo.cpp                               */
/***************************************************************************/

#include <vm/opinfo.h>
#include <cstring>


OpInfo::OpInfoData OpInfo::table[] = {

#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) \
    { s, (OPCODE_FORMAT) (args + type), FLOW_ ## ctrl, pop, push, c },

    // Kind of a workaround, get the prefixes (IInternal) to return InlineOpcode instead of InlineNone
#define IInternal   (InlineOpcode - InlineNone)
#define IMacro      0
#define IPrimitive  0
#define IAnnotation 0
#define IObjModel   0
#define IPrefix     0

#define Pop0    0
#define Pop1    1
#define PopI    1
#define PopI4   1
#define PopR4   1
#define PopI8   1
#define PopR8   1
#define PopRef  1
#define VarPop  -1

#define Push0    0
#define Push1    1
#define PushI    1
#define PushI4   1
#define PushR4   1
#define PushI8   1
#define PushR8   1
#define PushRef  1
#define VarPush  -1

#include "opcode.def"
#undef OPDEF
};

inline uint16_t GET_UNALIGNED_16(const void *pObject)
{
	uint16_t temp;
	memcpy(&temp, pObject, sizeof(temp));
	return temp;
}
inline uint32_t GET_UNALIGNED_32(const void *pObject)
{
	uint32_t temp;
	memcpy(&temp, pObject, sizeof(temp));
	return temp;
}
inline uint64_t GET_UNALIGNED_64(const void *pObject)
{
	uint64_t temp;
	memcpy(&temp, pObject, sizeof(temp));
	return temp;
}
#define VAL16(x) x
#define VAL32(x) x
#define VAL64(x) x

#define GET_UNALIGNED_VAL16(_pObject) VAL16(GET_UNALIGNED_16(_pObject))
#define GET_UNALIGNED_VAL32(_pObject) VAL32(GET_UNALIGNED_32(_pObject))
#define GET_UNALIGNED_VAL64(_pObject) VAL64(GET_UNALIGNED_64(_pObject))

/***************************************************************************/
/* parse instruction at 'instrPtr', into its opcode (OpInfo), and its
   (inline)args, 'args'  'instrPtr' is updated */

/***************************************************************************/
const uint8_t* OpInfo::fetch(const uint8_t* instrPtr, OpArgsVal* args) {

    data = &table[*instrPtr++];
AGAIN:
    assert(data - table == data->opcode);
    switch(data->format) {
        case InlineNone:
            break;
        case InlineOpcode:
            assert(*instrPtr + 256 < (int) (sizeof(table) / sizeof(OpInfoData)));
            data = &table[256 + *instrPtr++];
            goto AGAIN;

        case ShortInlineVar:
            args->i = *instrPtr; instrPtr +=1;
            break;
        case InlineVar:
            args->i = GET_UNALIGNED_VAL16(instrPtr); instrPtr +=2;
            break;
        case ShortInlineI:
        case ShortInlineBrTarget:
            args->i = *instrPtr; instrPtr +=1;
            break;
        case ShortInlineR: {
            uint32_t f = GET_UNALIGNED_VAL32(instrPtr); instrPtr +=4;
            args->r = *((float*) (&f));
            }
            break;
        case InlineRVA:
        case InlineI:
        case InlineMethod:
        case InlineField:
        case InlineType:
        case InlineString:
        case InlineSig:
        case InlineTok:
        case InlineBrTarget:
            args->i = GET_UNALIGNED_VAL32(instrPtr); instrPtr +=4;
            break;
        case InlineI8:
            args->i8 = GET_UNALIGNED_VAL64(instrPtr); instrPtr +=8;
            break;
        case InlineR: {
            uint64_t d = GET_UNALIGNED_VAL64(instrPtr); instrPtr +=8;
            instrPtr += 8;
            args->r = *((double*) (&d));
            } break;
        case InlineSwitch:
            args->switch_.count   = GET_UNALIGNED_VAL32(instrPtr); instrPtr +=4;
            args->switch_.targets = (int*) instrPtr; instrPtr += (4 * args->switch_.count);
            break;
        case InlinePhi:
            args->phi.count = GET_UNALIGNED_VAL32(instrPtr); instrPtr +=1;
            args->phi.vars  = (unsigned short*) instrPtr; instrPtr += (2 * args->phi.count);
            break;
        default:
            assert(!"BadType");
            break;
        }
    return(instrPtr);
}

