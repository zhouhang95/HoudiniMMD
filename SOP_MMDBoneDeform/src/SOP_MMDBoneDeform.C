/*
 * Copyright (c) 2024
 *	Side Effects Software Inc.  All rights reserved.
 *
 * Redistribution and use of Houdini Development Kit samples in source and
 * binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. The name of Side Effects Software may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE `AS IS' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 * The MMDBoneDeform SOP
 */

#include "SOP_MMDBoneDeform.h"
#include "SOP_common.h"

// This is an automatically generated header file based on theDsFile, below,
// to provide SOP_MMDBoneDeformParms, an easy way to access parameter values from
// SOP_MMDBoneDeformVerb::cook with the correct type.
#include "SOP_MMDBoneDeform.proto.h"

#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <PRM/PRM_TemplateBuilder.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_StringHolder.h>
#include <SOP/SOP_Node.h>
#include <GU/GU_Detail.h>
#include <GA/GA_Attribute.h>
#include <GA/GA_AIFSharedStringTuple.h>
#include <UT/UT_StringArray.h>
#include <SYS/SYS_Math.h>
#include <SYS/SYS_Types.h>
#include <limits.h>
#include <iostream>
#include <typeinfo>
#include <algorithm> 
#include <stdint.h>
#include <fmt/core.h>

using namespace UT::Literal;
using namespace HDK_Sample;

//
// Help is stored in a "wiki" style text file.  This text file should be copied
// to $HOUDINI_PATH/help/nodes/sop/star.txt
//
// See the sample_install.sh file for an example.
//

/// This is the internal name of the SOP type.
/// It isn't allowed to be the same as any other SOP's type name.
const UT_StringHolder SOP_MMDBoneDeform::theSOPTypeName("mmd_bonedeform"_sh);

/// newSopOperator is the hook that Houdini grabs from this dll
/// and invokes to register the SOP.  In this case, we add ourselves
/// to the specified operator table.
void
newSopOperator(OP_OperatorTable *table)
{
    table->addOperator(new OP_Operator(
        SOP_MMDBoneDeform::theSOPTypeName,   // Internal name
        "MMDBoneDeform",                     // UI name
        SOP_MMDBoneDeform::myConstructor,    // How to build the SOP
        SOP_MMDBoneDeform::buildTemplates(), // My parameters
        1,                          // Min # of sources
        3,                          // Max # of sources
        nullptr,                    // Custom local variables (none)
        OP_FLAG_GENERATOR));        // Flag it as generator
}

/// This is a multi-line raw string specifying the parameter interface
/// for this SOP.
static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
}
)THEDSFILE";

PRM_Template*
SOP_MMDBoneDeform::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_MMDBoneDeform.C"_sh, theDsFile);
    return templ.templates();
}

class SOP_MMDBoneDeformVerb : public SOP_NodeVerb
{
public:
    SOP_MMDBoneDeformVerb() {}
    virtual ~SOP_MMDBoneDeformVerb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_MMDBoneDeformParms(); }
    virtual UT_StringHolder name() const { return SOP_MMDBoneDeform::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_GENERIC; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_MMDBoneDeformVerb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_MMDBoneDeformVerb> SOP_MMDBoneDeformVerb::theVerb;

const SOP_NodeVerb *
SOP_MMDBoneDeform::cookVerb() const 
{ 
    return SOP_MMDBoneDeformVerb::theVerb.get();
}

/// This is the function that does the actual work.
void
SOP_MMDBoneDeformVerb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
}
