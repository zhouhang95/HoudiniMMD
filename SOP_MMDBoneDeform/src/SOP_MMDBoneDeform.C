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
 #include <GA/GA_SplittableRange.h>
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

static std::vector<glm::mat4> getBoneMatrix(const GU_Detail *detail) {
    std::vector<glm::mat4> matrixs;
    GA_ROHandleM3 attr_transform = GA_ROHandleM3(detail->findFloatTuple(GA_ATTRIB_POINT, "transform", 9));
    for (GA_Iterator it(detail->getPointRange()); !it.atEnd(); ++it) {
        UT_Vector3 pos = detail->getPos3(*it);
        UT_Matrix3 matrix = attr_transform.get(*it);
        glm::mat4 mat;
        mat[0] = {matrix[0][0], matrix[0][1], matrix[0][2], 0};
        mat[1] = {matrix[1][0], matrix[1][1], matrix[1][2], 0};
        mat[2] = {matrix[2][0], matrix[2][1], matrix[2][2], 0};
        mat[3] = {pos[0], pos[1], pos[2], 1};
        matrixs.push_back(mat);
    }
    return matrixs;
}
static std::vector<glm::mat4> getInvertedBoneMatrix(const GU_Detail *detail) {
    std::vector<glm::mat4> inv_matrixs;
    auto matrixs = getBoneMatrix(detail);
    for (auto i = 0; i < matrixs.size(); i++) {
        auto m = matrixs[i];
        auto inv_m = glm::inverse(m);
        inv_matrixs.push_back(inv_m);
    }
    return inv_matrixs;
}

static UT_Vector3 transform_pos(glm::mat4 &transform, UT_Vector3 pos) {
    auto p = transform * glm::vec4(pos[0], pos[1], pos[2], 1);
    return {p.x, p.y, p.z};
}
static UT_Vector3 transform_nrm(glm::mat4 &transform, UT_Vector3 pos) {
    auto p = glm::transpose(glm::inverse(transform)) * glm::vec4(pos[0], pos[1], pos[2], 0);
    return {p.x, p.y, p.z};
}

/// This is the function that does the actual work.
void
SOP_MMDBoneDeformVerb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    GU_Detail *detail = cookparms.gdh().gdpNC();
    const GU_Detail *input_0 = cookparms.inputGeo(0);
    detail->duplicate(*input_0);
    if (cookparms.hasInput(1) == false || cookparms.hasInput(2) == false) {
        return;
    }
    const GU_Detail *input_1 = cookparms.inputGeo(1);
    auto restPointTransformsInv = getInvertedBoneMatrix(input_1);
    const GU_Detail *input_2 = cookparms.inputGeo(2);
    auto deformPointTransforms = getBoneMatrix(input_2);
    std::vector<glm::mat4> matrixs;
    for (auto i = 0; i < restPointTransformsInv.size(); i++) {
        auto matrix = deformPointTransforms[i] * restPointTransformsInv[i];
        matrixs.push_back(matrix);
    }
    auto bones_index = GA_RWHandleV4(detail->findFloatTuple(GA_ATTRIB_POINT, "bones_index", 4));
    auto bones_weight = GA_RWHandleV4(detail->findFloatTuple(GA_ATTRIB_POINT, "bones_weight", 4));
    UTparallelFor(
        GA_SplittableRange(detail->getPointRange())
        , [detail, &bones_index, &bones_weight, &matrixs](const GA_SplittableRange &r) {
            for (GA_Iterator it(r); !it.atEnd(); ++it) {
                UT_Vector3 pos = detail->getPos3(*it);
                UT_Vector3 new_pos = {};
                float w = 0;
                auto bi = bones_index.get(*it);
                auto bw = bones_weight.get(*it);
                for (auto i = 0; i < 4; i++) {
                    int bii = int(bi[i]);
                    if (bii >= 0 && bw[i] > 0) {
                        new_pos += transform_pos(matrixs[bii], pos) * bw[i];
                        w += bw[i];
                    }
                }
                new_pos = new_pos / w;
                detail->setPos3(*it, new_pos);
            }
    });
}
